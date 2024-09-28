// Based on tutorial at https://learnopengl.com/Advanced-Lighting/Deferred-Shading

#include "common.h"

typedef u32 HSHADER;

#define INVALID_SHADER_HANDLE 0xFFFFFFFF
#define BUFFER_VERT_SHADER_PATH "vert.glsl"
#define BUFFER_FRAG_SHADER_PATH "frag.glsl"

#define LIGHT_VERT_SHADER_PATH "lightvert.glsl"
#define LIGHT_FRAG_SHADER_PATH "lightfrag.glsl"

typedef struct {
    u32 GBuffer;
    u32 PositionBuffer;
    u32 NormalBuffer;
    u32 AlbedoSpecBuffer;

    HSHADER BufferProgram;
    HSHADER LightProgram;
} RenderContext_t;

// DR = Deferred Renderer

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
            BUFFER_VERT_SHADER_PATH,
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

bool DR_UseShader(HSHADER shaderHandle)
{
    if (shaderHandle == INVALID_SHADER_HANDLE)
    {
        return false;
    }

    glUseProgram(shaderHandle);
    return true;
}

bool DR_IsContextValid(RenderContext_t* pContext)
{
    if (!pContext)
    {
        return false;
    }

    // None of the buffer names should be zero
    return 
        pContext->GBuffer &&
        pContext->PositionBuffer &&
        pContext->NormalBuffer &&
        pContext->AlbedoSpecBuffer &&
        pContext->BufferProgram != INVALID_SHADER_HANDLE &&
        pContext->LightProgram != INVALID_SHADER_HANDLE;
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
    pContext->BufferProgram = INVALID_SHADER_HANDLE;
    pContext->LightProgram = INVALID_SHADER_HANDLE;

    // Create and bind the g buffer
    glGenFramebuffers(1, &pContext->GBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, pContext->GBuffer);

    // Position color buffer
    glGenTextures(1, &pContext->PositionBuffer);
    glBindTexture(GL_TEXTURE_2D, pContext->PositionBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pContext->PositionBuffer, 0);

    // Normal color buffer
    glGenTextures(1, &pContext->NormalBuffer);
    glBindTexture(GL_TEXTURE_2D, pContext->NormalBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pContext->PositionBuffer, 0);

    // Color/specular component buffer (albedo)
    // Note the use of 8-bit precision
    glGenTextures(1, &pContext->AlbedoSpecBuffer);
    glBindTexture(GL_TEXTURE_2D, pContext->AlbedoSpecBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, pContext->PositionBuffer, 0);

    // Tell OpenGL which buffers we'll be using to draw for the bound frame buffer
    u32 attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Get and compile shaders
    char vertText[2048] = {};
    char fragText[2048] = {};
    if (!_DR_ReadText(pAssetRoot, BUFFER_VERT_SHADER_PATH, vertText, sizeof(vertText)))
    {
        return false;
    }

    if (!_DR_ReadText(pAssetRoot, BUFFER_FRAG_SHADER_PATH, fragText, sizeof(fragText)))
    {
        return false;
    }

    pContext->BufferProgram = DR_CreateShader(vertText, fragText);
    assert(pContext->BufferProgram != INVALID_SHADER_HANDLE);

    memset(vertText, 0, sizeof(vertText));
    memset(fragText, 0, sizeof(fragText));
    if (!_DR_ReadText(pAssetRoot, LIGHT_VERT_SHADER_PATH, vertText, sizeof(vertText)))
    {
        return false;
    }

    if (!_DR_ReadText(pAssetRoot, LIGHT_FRAG_SHADER_PATH, fragText, sizeof(fragText)))
    {
        return false;
    }

    pContext->LightProgram = DR_CreateShader(vertText, fragText);
    assert(pContext->LightProgram != INVALID_SHADER_HANDLE);

    return true;
}

void DR_BeginFrame(RenderContext_t* pContext)
{
    if (!DR_IsContextValid(pContext))
    {
        fprintf(stderr, "DR_BeginFrame() failed: invalid context\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, pContext->GBuffer);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: bind shaders
}

void DR_EndFrame(RenderContext_t* pContext)
{
    if (!DR_IsContextValid(pContext))
    {
        fprintf(stderr, "DR_EndFrame() failed: invalid context\n");
        return;
    }

    // Perform the lighting pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pContext->PositionBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pContext->NormalBuffer);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pContext->AlbedoSpecBuffer);

    // TODO: bind lighting pass shader
    // TODO: bind all g buffer textures
    // TODO: set all lighting uniform vars
    // TODO: render the screen quad
}
