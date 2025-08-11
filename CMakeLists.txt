cmake_minimum_required(VERSION 3.16)
project(installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Add SDL2 subdirectory
add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL2" EXCLUDE_FROM_ALL)



# Create executables - console version for testing
set(CONSOLE_INSTALLER_SOURCES
    src/console_main.cpp
    src/utils/system_utils.cpp
    src/installer/installation_manager.cpp
)

set(GUI_INSTALLER_SOURCES
    src/main.cpp
    src/nuklear_impl.cpp
    src/gui/installer_window.cpp
    src/gui/window_utils.cpp
    src/utils/system_utils.cpp
    src/installer/installation_manager.cpp
)

set(GUI_INSTALLER_ORIGINAL_SOURCES src/main.cpp)

# If a standalone original source file exists at repository root, prefer it
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.original.cpp")
    message(STATUS "Using main.original.cpp for installer_original target")
    set(GUI_INSTALLER_ORIGINAL_SOURCES main.original.cpp)
endif()

add_executable(installer_original WIN32 ${GUI_INSTALLER_ORIGINAL_SOURCES})
target_include_directories(installer_original PRIVATE 
    "${SDL2_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nuklear"
)
target_link_libraries(installer_original PRIVATE SDL2-static SDL2main)
if(WIN32)
    target_link_libraries(installer_original PRIVATE 
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
    )
endif()

# Console installer (no GUI dependencies)
add_executable(installer_console ${CONSOLE_INSTALLER_SOURCES})
target_include_directories(installer_console PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# GUI installer with SDL2 and resource file
if(WIN32)
    add_executable(installer WIN32 ${GUI_INSTALLER_SOURCES} assets/resource.rc)
else()
    add_executable(installer ${GUI_INSTALLER_SOURCES})
endif()

# Include directories for GUI version
target_include_directories(installer PRIVATE 
    "${SDL2_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nuklear"
)

# Link static SDL2 libraries (GUI version only)
target_link_libraries(installer PRIVATE SDL2-static SDL2main)

# Link additional Windows libraries
if(WIN32)
    # Console installer libraries
    target_link_libraries(installer_console PRIVATE 
        ole32 
        shell32 
        comdlg32
        advapi32
        user32
        gdi32
        kernel32
    )
    
    # GUI installer libraries (includes SDL2 dependencies)
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
    )
endif()