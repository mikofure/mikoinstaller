// Nuklear implementation file
// This file contains the implementation of Nuklear
// Include this file only once in the entire project

#include <SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE

// Override NK_API to ensure functions are exported
#define NK_API extern "C"

#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"
