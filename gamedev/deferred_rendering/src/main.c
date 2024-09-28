#include "common.h"
#include "render.c"

// Globals
static bool g_isRunning = true;
SDL_Window* g_pWindow = NULL;
RenderContext_t g_RenderContext;

void mainloop()
{
	SDL_Event evt;
	while(SDL_PollEvent(&evt))
	{
		if ((evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) ||
			(evt.type == SDL_QUIT))
		{
			g_isRunning = false;
			break;
		}
	}

    DR_BeginFrame(&g_RenderContext);
    DR_EndFrame(&g_RenderContext);

	SDL_GL_SwapWindow(g_pWindow);
}

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return -1;
    }

    const u32 WindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    g_pWindow = SDL_CreateWindow(
        "deferred rendering",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        WindowFlags);
    if (!g_pWindow)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext GLContext = SDL_GL_CreateContext(g_pWindow);
    char* pAssetRoot = argc > 1 ? argv[1] : ".";
    if (!DR_Initialize(&g_RenderContext, pAssetRoot))
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -1;
    }

    while(g_isRunning)
    {
		mainloop();
    }

	SDL_DestroyWindow(g_pWindow);
    SDL_Quit();
    
    return 0;
}
