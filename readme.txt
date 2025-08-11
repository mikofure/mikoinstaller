Miko Installer
=================

A modern, borderless Windows installer application built with SDL2 and Nuklear GUI framework.

Project Overview
----------------
This is a native Windows installer application for MikoIDE with a modern, borderless interface
that integrates seamlessly with Windows 11's native visual effects and animations.

Features
--------
- Borderless window design with custom title bar
- Windows 11 native effects integration (dark mode, rounded corners, backdrop)
- Drag-and-drop window movement
- Custom window controls (minimize, maximize, close)
- Installation path selection with folder browser dialog
- Options for PATH environment variable modification
- File extension association configuration
- Embedded resources (fonts and images) for standalone distribution
- Static linking for single-executable deployment

Technical Architecture
----------------------

Core Technologies:
- SDL2 (Simple DirectMedia Layer) for cross-platform windowing and rendering
- Nuklear immediate-mode GUI framework for UI components
- Windows DWM API for native visual effects
- CMake build system with automatic dependency management

Key Components:

1. InstallerWindow Class (src/main.cpp)
   - Main application window management
   - SDL2 initialization and window creation
   - Windows-specific integration (DWM effects, window subclassing)
   - Event handling and user interaction

2. Embedded Resources:
   - src/fonts/InterVariable.h: Inter Variable font as binary data
   - src/images/banner.h: Background image as binary data
   - assets/resource.rc: Windows resource file for icon embedding

3. GUI Framework Integration:
   - framework/nuklear.h: Immediate-mode GUI library
   - framework/nuklear_sdl_renderer.h: SDL2 renderer backend

Build System
------------
The project uses CMake with the following configuration:
- C++17 standard
- Static linking of SDL2 (downloaded automatically from GitHub)
- Windows-specific libraries for system integration
- Resource compilation for embedded assets

Key Build Features:
- Automatic SDL2 source download and compilation
- Static linking for standalone executable
- WIN32 subsystem for GUI application (no console window)
- Embedded resources compilation

Installer Functionality
-----------------------
The installer provides the following user options:

1. Installation Path Selection:
   - Default: %LOCALAPPDATA%\MikoIDE
   - Custom path selection via folder browser

2. System Integration Options:
   - Add MikoIDE to system PATH environment variable
   - Associate file extensions with MikoIDE

3. Installation Process:
   - Currently implements placeholder logic
   - Designed for easy extension with actual file deployment

Window Management
-----------------
The application implements custom window management:

- Borderless Design: Uses SDL_WINDOW_BORDERLESS flag
- Custom Title Bar: 32px height with integrated controls
- Window Controls: Custom minimize, maximize, and close buttons
- Drag Support: Click and drag anywhere in title bar area
- Native Effects: Windows 11 DWM integration for modern appearance

User Interface
--------------
The UI is built using Nuklear immediate-mode GUI:

- Background Image: Embedded banner displayed in upper portion
- Installation Panel: Lower section with options and controls
- Custom Styling: Dark theme matching Windows 11 aesthetics

Development Notes
-----------------

File Structure:
- src/main.cpp: Main application logic and window management
- src/fonts/: Embedded font resources
- src/images/: Embedded image resources
- src/framework/: GUI framework headers
- assets/: External resources (icons, resource definitions)
- CMakeLists.txt: Build configuration

Key Design Decisions:
- Static linking for single-file distribution
- Embedded resources to eliminate external dependencies
- Windows-specific optimizations for native look and feel
- Immediate-mode GUI for simplified state management

Build Instructions
------------------
1. Ensure CMake 3.16+ and Visual Studio are installed
2. Run: cmake -B build
3. Run: cmake --build build --config Release
4. Executable will be generated in build/Release/

Dependencies
------------
- SDL2 (automatically downloaded and built)
- Windows SDK (for DWM and system APIs)
- CMake 3.16 or higher
- Visual Studio 2019 or higher (for MSVC compiler)

Compatibility
-------------
- Windows 10 version 1903 or higher (for DWM effects)
- Windows 11 (full feature support)
- x64 architecture

License and Credits
-------------------
- Uses SDL2 library (zlib license)
- Uses Nuklear GUI framework (public domain)
- Uses Inter font family (SIL Open Font License)

This installer serves as a foundation for deploying MikoIDE with a professional,
modern appearance that integrates seamlessly with the Windows desktop environment.