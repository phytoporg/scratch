#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef uint32_t u32;
typedef int32_t  s32;

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#endif // __COMMON_H__
