#include "common.h"
#include "math.c"
#include "render.c"
#include "map.c"

// Globals
static bool g_isRunning = true;
SDL_Window* g_pWindow = NULL;
RenderContext_t g_RenderContext;
MapInfo_t g_mapInfo;

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

    const int TileIndex = 0;
    for (int y = 0; y < g_mapInfo.MapHeight; ++y)
    {
        for (int x = 0; x < g_mapInfo.MapWidth; ++x)
        {
            if(g_mapInfo.Grid[y][x] == 0)
            {
                // TODO: Shouldn't have to negate this
                DR_DrawTile(&g_RenderContext, (float)x, (float)-y, TileIndex);
            }
        }
    }

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext GLContext = SDL_GL_CreateContext(g_pWindow);
    char* pAssetRoot = argc > 1 ? argv[1] : ".";
    if (!DR_Initialize(&g_RenderContext, pAssetRoot))
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -1;
    }

    // Set up view, projection matrices
    Matrix44f projectionMatrix;
    Matrix44f viewMatrix;
    const float Left = 0.0f;
    const float Right = (float)SCREEN_WIDTH / 2.f;
    const float Bottom = 0.0f;
    const float Top = (float)SCREEN_HEIGHT / 2.f;
    const float Near = 0.1f;
    const float Far = 100.0f;
    Math_Matrix44f_Ortho(
        &projectionMatrix,
        Left, Right,
        Bottom, Top,
        Near, Far);	

    Vector3f cameraPosition = { 0.f, 0.f, 0.f };
    Vector3f cameraLook     = { 0.f, 0.f, 1.f };
    Vector3f cameraUp       = { 0.f, 1.f, 0.f };
    Math_Matrix44f_LookAt(&cameraPosition, &cameraLook, &cameraUp, &viewMatrix);

    DR_SetProjection(&g_RenderContext, &projectionMatrix);
    DR_SetView(&g_RenderContext, &viewMatrix);

    Vector2f tilemapOffset = {
        .x = Right / 2.f,
        .y = Top - TILE_HEIGHT_PX,
    };
    DR_SetTilemapOffset(&g_RenderContext, &tilemapOffset);

    if (!Map_LoadMap("./assets/tilemap.csv", &g_mapInfo))
    {
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
