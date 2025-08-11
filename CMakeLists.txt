cmake_minimum_required(VERSION 3.16)
project(installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)

# Setup SDL2 from source for static linking
set(SDL2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2-source")

# Check if SDL2 directories exist, if not create them
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# Download SDL2 source if not present
if(NOT EXISTS "${SDL2_SOURCE_DIR}")
    message(STATUS "Downloading SDL2 source...")
    file(DOWNLOAD
        "https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.zip"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2-source.zip"
        SHOW_PROGRESS
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2-source.zip"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
    )
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2-2.28.5" "${SDL2_SOURCE_DIR}")
endif()

# Configure SDL2 for static linking
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
# Disable SDL2's global "uninstall" custom target to avoid name collisions
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable uninstallation of SDL2" FORCE)

# Add SDL2 subdirectory
add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL2" EXCLUDE_FROM_ALL)



# Create executable with resource file
if(WIN32)
    add_executable(installer WIN32
        src/interface/installer/main.cpp
    src/interface/installer/InstallerWindow.cpp
        src/framework/nuklear_impl.cpp
        assets/resource.rc)
else()
    add_executable(installer
        src/interface/installer/main.cpp
    src/interface/installer/InstallerWindow.cpp
        src/framework/nuklear_impl.cpp)
endif()

# Include SDL2 headers
target_include_directories(installer PRIVATE "${SDL2_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")

# toml++ (header-only)
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(tomlplusplus)
target_include_directories(installer PRIVATE ${tomlplusplus_SOURCE_DIR}/include)


# LZMA (xz) via FetchContent (v5.8.1)
FetchContent_Declare(xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG v5.8.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(xz)

# Link to liblzma target from xz
if(TARGET liblzma)
    target_link_libraries(installer PRIVATE liblzma)
elseif(TARGET LibLZMA::LibLZMA)
    target_link_libraries(installer PRIVATE LibLZMA::LibLZMA)
elseif(TARGET lzma)
    target_link_libraries(installer PRIVATE lzma)
else()
    message(FATAL_ERROR "liblzma target not found from xz FetchContent")
endif()
target_compile_definitions(installer PRIVATE HAVE_LZMA=1 LZMA_API_STATIC=1)
target_include_directories(installer PRIVATE ${xz_SOURCE_DIR}/src/liblzma/api)
# Link static SDL2 libraries
target_link_libraries(installer PRIVATE SDL2-static SDL2main)

# Link additional Windows libraries for static SDL2 and Win32 dialogs
if(WIN32)
    target_link_libraries(installer PRIVATE 
        ole32 
        shell32 
        comdlg32
        winmm
        imm32
        version
        setupapi
        advapi32
        user32
        gdi32
        kernel32
        ws2_32
    dwmapi
    )
endif()

# Uninstaller target
if(WIN32)
    add_executable(uninstall WIN32
        src/interface/uninstall/main.cpp
        src/interface/uninstall/UninstallerWindow.cpp
        src/framework/nuklear_impl.cpp
        assets/resource.rc)
else()
    add_executable(uninstall
        src/interface/uninstall/main.cpp
        src/interface/uninstall/UninstallerWindow.cpp
        src/framework/nuklear_impl.cpp)
endif()

target_include_directories(uninstall PRIVATE "${SDL2_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(uninstall PRIVATE SDL2-static SDL2main)
if(WIN32)
    target_link_libraries(uninstall PRIVATE 
        ole32 
        shell32 
        comdlg32
        winmm
        imm32
        version
        setupapi
        advapi32
        user32
        gdi32
        kernel32
        ws2_32
        dwmapi
    )
endif()

## ------------------------------
## Python-based packaging (Windows)
## ------------------------------
if(WIN32)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(PACKAGE_APP_NAME "MikoIDE")
    set(PACKAGE_APP_VERSION "1.0.0")
    set(PACKAGER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/packaging/pack.py")
    set(SETUP_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Release/${PACKAGE_APP_NAME}-Setup-${PACKAGE_APP_VERSION}.exe")

    add_custom_command(
        OUTPUT "${SETUP_OUTPUT}"
        COMMAND ${Python3_EXECUTABLE} "${PACKAGER_SCRIPT}"
            --project-root "${CMAKE_CURRENT_SOURCE_DIR}"
            --build-dir "${CMAKE_CURRENT_BINARY_DIR}/Release"
            --sources-dir "${CMAKE_CURRENT_SOURCE_DIR}/sources"
            --bootstrap-exe "${CMAKE_CURRENT_BINARY_DIR}/Release/installer.exe"
            --app-name "${PACKAGE_APP_NAME}"
            --app-version "${PACKAGE_APP_VERSION}"
            --output "${SETUP_OUTPUT}"
        DEPENDS installer
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Packaging custom installer (LZMA + TOML)"
        VERBATIM
    )

    add_custom_target(package ALL DEPENDS "${SETUP_OUTPUT}")
endif()