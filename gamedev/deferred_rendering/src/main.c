#include "common.h"
#include "math.c"
#include "render.c"

// Globals
static bool g_isRunning = true;
SDL_Window* g_pWindow = NULL;
RenderContext_t g_RenderContext;
Matrix44f g_ProjectionMatrix;
Matrix44f g_ViewMatrix;

Vector3f g_CubePosition = { 0.0f, 0.0f, -4.0f };

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
    DR_SetShaderParameterMat4(
        g_RenderContext.GeometryProgram,
        "projection",
        &g_ProjectionMatrix);
    DR_SetShaderParameterMat4(
        g_RenderContext.GeometryProgram,
        "view",
        &g_ViewMatrix);

    // Draw the cube
    Matrix44f modelMatrix;
    Math_Matrix44f_Identity(&modelMatrix);
    Math_Matrix44f_Translate(&modelMatrix, &g_CubePosition);
    DR_SetShaderParameterMat4(g_RenderContext.GeometryProgram, "model", &modelMatrix);
    DR_RenderCube(&g_RenderContext);

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

    // Set up view, projection matrices
    const float Angle = 45.0f;
    const float Ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    const float Near = 0.1f;
    const float Far = 100.0f;
    Math_Matrix44f_Perspective(&g_ProjectionMatrix, Angle, Ratio, Near, Far);

    Vector3f cameraPosition = { 0.f, 0.f,  0.f };
    Vector3f cameraLook     = { 0.f, 0.f, -1.f };
    Vector3f cameraUp       = { 0.f, 1.f,  0.f };
    Math_Matrix44f_LookAt(&cameraPosition, &cameraLook, &cameraUp, &g_ViewMatrix);

    // Lighting -- one light behind the camera
    PointLight_t pointLight = {
        .Position = { 0.f, 0.f, -2.0f },
        .Color = { 1.0f, 1.0f, 1.0f }
    };
    DR_CreatePointLight(&g_RenderContext, &pointLight);

    while(g_isRunning)
    {
		mainloop();
    }

	SDL_DestroyWindow(g_pWindow);
    SDL_Quit();
    
    return 0;
}
