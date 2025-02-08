#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "pb-math.c"
#include "pb-render.c"
#include "pb-input.c"

// Constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960

// TODO: adjust these forreal
#define FIELD_WIDTH 900
#define FIELD_HEIGHT 600

#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 100
#define PADDLE_SPEED 10

#define BALL_WIDTH 15
#define BALL_HEIGHT 15
#define BALL_SPEED 15

#define FPS 60.0f

typedef struct
{
    // Should probably live w/physics geometry, migrate when that's in place
    bool inputUpPressed;
    bool inputDownPressed;
    Rect FieldRect;
    Rect LeftPaddleRect;
    Rect RightPaddleRect;
    Rect PreviousBallRect;
    Rect BallRect;
    Vector2i BallDirection;
    InputContext InputContext;

    // Colors
    SDL_Color FieldBorderColor;
    SDL_Color RightPaddleColor;
    SDL_Color LeftPaddleColor;
    SDL_Color BallColor;
} GameState;

void GameState_init(GameState* pGameState) 
{
    memset(pGameState, 0, sizeof(*pGameState));

    const int HalfScreenWidth = SCREEN_WIDTH / 2;
    const int HalfScreenHeight = SCREEN_HEIGHT / 2;
    const int HalfFieldWidth = FIELD_WIDTH / 2;
    const int HalfFieldHeight = FIELD_HEIGHT / 2;
    const int HalfPaddleHeight = PADDLE_HEIGHT / 2;

    pGameState->FieldRect = (Rect) {
        .x = HalfScreenWidth - HalfFieldWidth,
        .y = HalfScreenHeight - HalfFieldHeight,
        .w = FIELD_WIDTH,
        .h = FIELD_HEIGHT
    };

    pGameState->LeftPaddleRect = (Rect) {
        .x = pGameState->FieldRect.x,
        .y = pGameState->FieldRect.y + 
             HalfFieldHeight - 
             HalfPaddleHeight,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT
    };

    pGameState->RightPaddleRect = (Rect) {
        .x = pGameState->FieldRect.x + FIELD_WIDTH - PADDLE_WIDTH,
        .y = pGameState->LeftPaddleRect.y,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT
    };

    pGameState->PreviousBallRect = (Rect) { .x = -1, .y = -1, .w = -1, .h = -1 };

    pGameState->BallRect = (Rect) {
        .x = pGameState->FieldRect.x + HalfFieldWidth,
        .y = pGameState->FieldRect.y + HalfFieldHeight,
        .w = BALL_WIDTH,
        .h = BALL_HEIGHT,
    };

    // TODO: real gameplay solution
    pGameState->BallDirection = (Vector2i) {
        .x = BALL_SPEED / 3.f,
        .y = BALL_SPEED / 2.f,
    };

    // Init colors
    pGameState->FieldBorderColor = (SDL_Color) { 255, 255, 255, SDL_ALPHA_OPAQUE };
    pGameState->RightPaddleColor = (SDL_Color) { 200, 50, 0, SDL_ALPHA_OPAQUE };
    pGameState->LeftPaddleColor = (SDL_Color) { 0, 100, 255, SDL_ALPHA_OPAQUE };
    pGameState->BallColor = (SDL_Color) { 255, 255, 255, SDL_ALPHA_OPAQUE };
}

typedef enum {
    NoCollision,
    LeftPaddleCollision,
    RightPaddleCollision,
    TopFieldCollision,
    BottomFieldCollision,
} BallCollision;

void GameState_updateBall(GameState* pGameState)
{
    // Move the ball according to the current state
    Vector2i nextBallPos = {
        .x = pGameState->BallRect.x + pGameState->BallDirection.x,
        .y = pGameState->BallRect.y + pGameState->BallDirection.y
    };

    // Check for collisions
    BallCollision collision = NoCollision;

    Extents BallExtents = rect_to_extents(&pGameState->BallRect);
    const int FieldBottom = rect_extent_bottom(&pGameState->FieldRect);
    const int FieldTop = rect_extent_top(&pGameState->FieldRect);
    if (BallExtents.Top <= FieldTop && pGameState->BallDirection.y < 0)
    {
        collision = TopFieldCollision;
    }
    else if (BallExtents.Bottom >= FieldBottom && pGameState->BallDirection.y > 0)
    {
        collision = BottomFieldCollision;
    }
    else if (rect_test_intersects(&pGameState->BallRect, &pGameState->LeftPaddleRect) &&
             pGameState->BallDirection.x < 0)
    {
        collision = LeftPaddleCollision;
    }
    else if (rect_test_intersects(&pGameState->BallRect, &pGameState->RightPaddleRect) &&
             pGameState->BallDirection.x > 0)
    {
        collision = RightPaddleCollision;
    }

    // Collision response
    // TODO: account for tunnelling and account for distance traveled beyond the
    // point of collision
    if (collision == TopFieldCollision)
    {
        nextBallPos.y = pGameState->FieldRect.y;
        pGameState->BallDirection.y = -pGameState->BallDirection.y;
    }
    else if (collision == BottomFieldCollision)
    {
        nextBallPos.y = FieldBottom - BALL_HEIGHT;
        pGameState->BallDirection.y = -pGameState->BallDirection.y;
    }
    else if (collision == LeftPaddleCollision)
    {
        nextBallPos.x = rect_extent_right(&pGameState->LeftPaddleRect);
        pGameState->BallDirection.x = -pGameState->BallDirection.x;
    }
    else if (collision == RightPaddleCollision)
    {
        nextBallPos.x = rect_extent_left(&pGameState->RightPaddleRect) - BALL_WIDTH;
        pGameState->BallDirection.x = -pGameState->BallDirection.x;
    }

    // Apply position update
    pGameState->PreviousBallRect = pGameState->BallRect;
    rect_set_pos(&pGameState->BallRect, &nextBallPos);
}

void GameState_update(GameState* pGameState)
{
    if (pGameState->inputUpPressed)
    {
        pGameState->LeftPaddleRect.y -= PADDLE_SPEED;
        if (pGameState->LeftPaddleRect.y < pGameState->FieldRect.y)
        {
            pGameState->LeftPaddleRect.y = pGameState->FieldRect.y;
        }
    }

    if (pGameState->inputDownPressed)
    {
        pGameState->LeftPaddleRect.y += PADDLE_SPEED;
        const int FieldBottom = rect_extent_bottom(&pGameState->FieldRect);
        const int PaddleBottom = rect_extent_bottom(&pGameState->LeftPaddleRect);
        if (PaddleBottom > FieldBottom)
        {
            pGameState->LeftPaddleRect.y = FieldBottom - PADDLE_HEIGHT;
        }
    }

    GameState_updateBall(pGameState);
}

void Input_init(InputContext* pInput)
{
    InputInitializeContext(pInput);

    // Might be nice to have a macro?
    pInput->InputMap[INPUTEVENT_QUIT].Scancodes[0] = SDL_SCANCODE_Q;
    pInput->InputMap[INPUTEVENT_QUIT].Scancodes[1] = SDL_SCANCODE_ESCAPE;
    pInput->InputMap[INPUTEVENT_QUIT].NumScancodes = 2;

    pInput->InputMap[INPUTEVENT_UP].Scancodes[0] = SDL_SCANCODE_W;
    pInput->InputMap[INPUTEVENT_UP].Scancodes[1] = SDL_SCANCODE_UP;
    pInput->InputMap[INPUTEVENT_UP].NumScancodes = 2;

    pInput->InputMap[INPUTEVENT_DOWN].Scancodes[0] = SDL_SCANCODE_S;
    pInput->InputMap[INPUTEVENT_DOWN].Scancodes[1] = SDL_SCANCODE_DOWN;
    pInput->InputMap[INPUTEVENT_DOWN].NumScancodes = 2;
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

    InputContext* pInput = &(g_GameState.InputContext);
    InputUpdateContext(pInput);
    g_shouldQuit = InputHasEventPressed(pInput, INPUTEVENT_QUIT);
    g_GameState.inputUpPressed = InputHasEvent(pInput, INPUTEVENT_UP);
    g_GameState.inputDownPressed = InputHasEvent(pInput, INPUTEVENT_DOWN);

    GameState_update(&g_GameState);

    RenderField(g_pRender, &g_GameState.FieldBorderColor, &g_GameState.FieldRect);
    RenderColoredRect( 
        g_pRender, &g_GameState.LeftPaddleColor, &g_GameState.LeftPaddleRect);
    RenderColoredRect( 
        g_pRender, &g_GameState.RightPaddleColor, &g_GameState.RightPaddleRect);
    RenderColoredRect( 
        g_pRender, &g_GameState.BallColor, &g_GameState.BallRect);

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
            "paddlebaddle",
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
    Input_init(&g_GameState.InputContext);

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
