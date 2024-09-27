#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef uint32_t u32;

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return -1;
    }

    const u32 WindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    SDL_Window* pWindow = SDL_CreateWindow(
        "deferred rendering",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        WindowFlags);
    if (!pWindow)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext GLContext = SDL_GL_CreateContext(pWindow);
    bool isRunning = true;
    while(isRunning)
    {
        SDL_Event evt;
        while(SDL_PollEvent(&evt))
        {
            if ((evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) ||
                (evt.type == SDL_QUIT))
            {
                isRunning = false;
                break;
            }
        }

        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(pWindow);
    }
    
    return 0;
}
