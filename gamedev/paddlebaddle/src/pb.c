#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "pb-render.c"

// Constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960

// TODO: adjust these forreal
#define FIELD_WIDTH 900
#define FIELD_HEIGHT 600

#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 100

#define FPS 60.0f

typedef struct
{
    // Should probably live w/physics geometry, migrate when that's in place
    int FieldUpperLeftX;
    int FieldUpperLeftY;
    int LeftPaddlePosX;
    int LeftPaddlePosY;
    int RightPaddlePosX;
    int RightPaddlePosY;
} GameState;

void GameState_init(GameState* gameState) 
{
    const int HalfScreenWidth = SCREEN_WIDTH / 2;
    const int HalfScreenHeight = SCREEN_HEIGHT / 2;
    const int HalfFieldWidth = FIELD_WIDTH / 2;
    const int HalfFieldHeight = FIELD_HEIGHT / 2;
    const int HalfPaddleHeight = PADDLE_HEIGHT / 2;

    gameState->FieldUpperLeftX = HalfScreenWidth - HalfFieldWidth;
    gameState->FieldUpperLeftY = HalfScreenHeight - HalfFieldHeight;
    gameState->LeftPaddlePosX = gameState->FieldUpperLeftX;
    gameState->LeftPaddlePosY = gameState->FieldUpperLeftY + 
                                HalfFieldHeight - 
                                HalfPaddleHeight;
    gameState->RightPaddlePosX = gameState->FieldUpperLeftX + 
                                 FIELD_WIDTH - 
                                 PADDLE_WIDTH;
    gameState->RightPaddlePosY = gameState->LeftPaddlePosY;
}

// Globals
static GameState g_GameState;
static SDL_Window* g_pWindow = NULL;
static SDL_Renderer* g_pRender = NULL;
static bool g_shouldQuit = false;

static void mainloop()
{
    const SDL_Color ClearColor = { 0, 0, 0 };
    SDL_SetRenderDrawColor(
        g_pRender,
        ClearColor.r,
        ClearColor.g,
        ClearColor.b,
        SDL_ALPHA_OPAQUE);
    SDL_RenderClear(g_pRender);

    // Main loop
    Uint64 startTime = SDL_GetPerformanceCounter();

    // Pump events, gotta do this before polling input
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            g_shouldQuit = true;
        }
    }

    RenderField(
        g_pRender,
        g_GameState.FieldUpperLeftX,
        g_GameState.FieldUpperLeftY,
        FIELD_WIDTH,
        FIELD_HEIGHT);
    RenderLeftPaddle(
        g_pRender,
        g_GameState.LeftPaddlePosX,
        g_GameState.LeftPaddlePosY,
        PADDLE_WIDTH,
        PADDLE_HEIGHT);
    RenderRightPaddle(
        g_pRender,
        g_GameState.RightPaddlePosX,
        g_GameState.RightPaddlePosY,
        PADDLE_WIDTH,
        PADDLE_HEIGHT);

    Uint64 endTime = SDL_GetPerformanceCounter();

    float elapsedMs = (endTime - startTime) / 
        (float)SDL_GetPerformanceFrequency() * 1000.0f; 
    float expectedMs = (1.0f / FPS) * 1000.0f;

    SDL_RenderPresent(g_pRender);
    SDL_Delay(expectedMs - elapsedMs);

}

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* g_pWindow = SDL_CreateWindow(
            "lil-tetris",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
    if (!g_pWindow)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return -1;
    }

    g_pRender = SDL_CreateRenderer(g_pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!g_pRender)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        return -1;
    }

    srand(time(NULL));

    GameState_init(&g_GameState);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (!g_shouldQuit)
    {
        mainloop();
    }

    SDL_DestroyRenderer(g_pRender);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();
#endif

    return 0;
} 
