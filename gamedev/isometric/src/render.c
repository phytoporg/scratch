#include "common.h"

typedef u32 HSHADER;

#define INVALID_SHADER_HANDLE 0xFFFFFFFF

#define FWD_VERT_SHADER_PATH "fwdvert.glsl"
#define FWD_FRAG_SHADER_PATH "fwdfrag.glsl"

#define TILEMAP_TEXTURE_PATH "tilemap.png"

#define TILE_HEIGHT_PX 32
#define TILE_WIDTH_PX 32

typedef struct {
    HSHADER ForwardProgram;

    Matrix44f Projection;
    Matrix44f View;

    u32 QuadVAO;
    u32 QuadVBO;

    u32 TilemapTexture;
} RenderContext_t;

// Internal helpers
bool _DR_ReadText(char* pAssetRoot, const char* pFilePath, char* pText, u32 maxLen)
{
    char fullAssetPath[512] = {};
    const u32 TotalLength = strlen(fullAssetPath) + strlen(pFilePath) + 1;
    if (TotalLength > sizeof(fullAssetPath))
    {
        fprintf(stderr, "Full file path for %s is too long\n", pFilePath);
        return false;
    }

    sprintf(fullAssetPath, "%s/%s", pAssetRoot, pFilePath);
    FILE* pFile = fopen(fullAssetPath, "r");
    if (!pFile)
    {
        fprintf(
            stderr,
            "Failed to open buffer vert shader file %s (%s)\n",
            pFilePath,
            strerror(errno));
        return false;
    }

    fseek(pFile, 0, SEEK_END);
    const u32 FileSize = ftell(pFile);
    if (FileSize > maxLen)
    {
        fprintf(
            stderr,
            "Cannot read file %s: contents larger than max length\n",
            pFilePath);
        fclose(pFile);
        return false;
    }
    
    fseek(pFile, 0, SEEK_SET);

    if (!fread(pText, 1, maxLen, pFile))
    {
        fprintf(
            stderr,
            "Cannot read file %s: read failed (%s)\n",
            pFilePath,
            strerror(errno));
        fclose(pFile);
        return false;
    }

    fclose(pFile);
    return true;
}

bool _DR_TextureFromFile(u32* pName, char* pFilePath, char* pAssetRoot)
{
    char fullPath[512] = {};
    sprintf(fullPath, "%s/%s", pAssetRoot, pFilePath);
    SDL_Surface* pSurface = IMG_Load(fullPath);
    if (!pSurface)
    {
        return false;
    }

    glGenTextures(1, pName);
    glBindTexture(GL_TEXTURE_2D, *pName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    const int Depth = pSurface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, pSurface->w, pSurface->h, 0, Depth,
        GL_UNSIGNED_BYTE, pSurface->pixels);
    SDL_FreeSurface(pSurface);

    return true;
}

// Public functions
HSHADER DR_CreateShader(const char* pVertText, const char* pFragText) 
{
    u32 shaderProgram = glCreateProgram();
    u32 vertObject = glCreateShader(GL_VERTEX_SHADER);
    u32 fragObject = glCreateShader(GL_FRAGMENT_SHADER);

    // Set up vertex shader source
    const char* pVertSources[1] = { pVertText };
    const s32 VertSourceLengths[1] = { strlen(pVertText) };
    glShaderSource(vertObject, 1, pVertSources, VertSourceLengths);

    // Set up fragment shader source
    const char* pFragSources[1] = { pFragText };
    const s32 FragSourceLengths[1] = { strlen(pFragText) };
    glShaderSource(fragObject, 1, pFragSources, FragSourceLengths);

    // Compile both shaders, report errors at the end
    glCompileShader(vertObject);
    glCompileShader(fragObject);

    s32 vertCompileSuccess;
    glGetShaderiv(vertObject, GL_COMPILE_STATUS, &vertCompileSuccess);
    if (!vertCompileSuccess)
    {
        char infoLog[1024];
        glGetShaderInfoLog(vertObject, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Error compiling vertex shader %s: %s\n", pVertText, infoLog);
    }

    s32 fragCompileSuccess;
    glGetShaderiv(fragObject, GL_COMPILE_STATUS, &fragCompileSuccess);
    if (!fragCompileSuccess)
    {
        char infoLog[1024];
        glGetShaderInfoLog(fragObject, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Error compiling fragment shader %s: %s\n", pFragText, infoLog);
    }

    if (!vertCompileSuccess || !fragCompileSuccess)
    {
        goto fail;
    }

    glAttachShader(shaderProgram, vertObject);
    glAttachShader(shaderProgram, fragObject);
    glLinkProgram(shaderProgram);

    s32 linkSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess)
    {
        char infoLog[1024];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Error linking shader program: %s\n", infoLog);
        goto fail;
    }

    glValidateProgram(shaderProgram);
    s32 validateSuccess;
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &validateSuccess);
    if (!validateSuccess)
    {
        char infoLog[1024];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Error validating shader program: %s\n", infoLog);
        goto fail;
    }

    return shaderProgram;

fail:
    glDeleteShader(vertObject);
    glDeleteShader(fragObject);
    glDeleteProgram(shaderProgram);
    return INVALID_SHADER_HANDLE;
}

// TODO:
// DR_DestroyPointLight

bool DR_UseShader(HSHADER shaderHandle)
{
    if (shaderHandle == INVALID_SHADER_HANDLE)
    {
        return false;
    }

    glUseProgram(shaderHandle);
    return true;
}

// State matrices
void DR_SetProjection(RenderContext_t* pContext, Matrix44f* pProj)
{
    pContext->Projection = *pProj;
}

void DR_SetView(RenderContext_t* pContext, Matrix44f* pView)
{
    pContext->View = *pView;
}

// Shader parameters
void DR_SetShaderParameteri(HSHADER shaderHandle, char* pName, u32 value)
{
    glUniform1i(glGetUniformLocation(shaderHandle, pName), value);
}

void DR_SetShaderParameterMat4(HSHADER shaderHandle, char* pName, Matrix44f* pMat)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderHandle, pName), 1, GL_FALSE, &pMat->m[0][0]);
}

void DR_SetShaderParameterVec3(HSHADER shaderHandle, char* pName, Vector3f* pVec)
{
    const GLint loc = glGetUniformLocation(shaderHandle, pName);
    glUniform3fv(glGetUniformLocation(shaderHandle, pName), 1, &pVec->data[0]);
}

bool DR_IsContextValid(RenderContext_t* pContext)
{
    if (!pContext)
    {
        return false;
    }

    // None of the buffer names should be zero
    return 
        pContext->ForwardProgram != INVALID_SHADER_HANDLE;
}

bool DR_Initialize(RenderContext_t* pContext, char* pAssetRoot)
{
    if (!pContext)
    {
        fprintf(stderr, "Cannot initialize render context: invalid pointer\n");
        return false;
    }

    // Zero out everything first, mark shaders as invalid
    memset(pContext, 0, sizeof(*pContext));
    pContext->ForwardProgram = INVALID_SHADER_HANDLE;

    // Get and compile shaders
    char vertText[2048] = {};
    char fragText[2048] = {};

    memset(vertText, 0, sizeof(vertText));
    memset(fragText, 0, sizeof(fragText));
    if (!_DR_ReadText(pAssetRoot, FWD_VERT_SHADER_PATH, vertText, sizeof(vertText)))
    {
        return false;
    }

    if (!_DR_ReadText(pAssetRoot, FWD_FRAG_SHADER_PATH, fragText, sizeof(fragText)))
    {
        return false;
    }

    pContext->ForwardProgram = DR_CreateShader(vertText, fragText);
    assert(pContext->ForwardProgram != INVALID_SHADER_HANDLE);

    // Set up screen quad geometry
	static const float QuadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, // TL
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // BL
		 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, // TR
		 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // BR
	};

	glGenVertexArrays(1, &pContext->QuadVAO);
	glGenBuffers(1, &pContext->QuadVBO);
	glBindVertexArray(pContext->QuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pContext->QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

    if (!_DR_TextureFromFile(&pContext->TilemapTexture, TILEMAP_TEXTURE_PATH, pAssetRoot))
    {
        fprintf(
            stderr, "Failed to load texture from file: %s\n", TILEMAP_TEXTURE_PATH);
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    return true;
}

void DR_BeginFrame(RenderContext_t* pContext)
{
    if (!DR_IsContextValid(pContext))
    {
        fprintf(stderr, "DR_BeginFrame() failed: invalid context\n");
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DR_UseShader(pContext->ForwardProgram);
    DR_SetShaderParameterMat4(
        pContext->ForwardProgram,
        "projection",
        &pContext->Projection);
    DR_SetShaderParameterMat4(
        pContext->ForwardProgram,
        "view",
        &pContext->View);
}

void DR_EndFrame(RenderContext_t* pContext)
{
    if (!DR_IsContextValid(pContext))
    {
        fprintf(stderr, "DR_EndFrame() failed: invalid context\n");
        return;
    }

    // TODO: postprocessing stuff
}

void DR_DrawTile(RenderContext_t* pContext, float x, float y, int tileIndex)
{
    Matrix44f modelMatrix;
    Math_Matrix44f_Identity(&modelMatrix);

    const float HalfTileWidth = TILE_WIDTH_PX * 0.5f;
    const float HalfTileHeight = TILE_HEIGHT_PX * 0.5f;
    Vector3f scale = { HalfTileWidth, HalfTileHeight, 1.0f };
    Math_Matrix44f_Scale(&modelMatrix, &scale);

    const float xOffset = HalfTileWidth;
    const float yOffset = HalfTileHeight;
    Vector3f translation = {
        x * TILE_WIDTH_PX + xOffset,
        y * TILE_HEIGHT_PX + yOffset,
        -0.1f };
    Math_Matrix44f_Translate(&modelMatrix, &translation);

    DR_SetShaderParameterMat4(
        pContext->ForwardProgram,
        "model",
        &modelMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pContext->TilemapTexture);

    glBindVertexArray(pContext->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
