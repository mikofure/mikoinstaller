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



# Create executable with resource file
if(WIN32)
    add_executable(installer WIN32 src/main.cpp assets/resource.rc)
else()
    add_executable(installer src/main.cpp)
endif()

# Include SDL2 headers
target_include_directories(installer PRIVATE "${SDL2_SOURCE_DIR}/include")

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
    )
endif()