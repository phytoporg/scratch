// Based on tutorial at https://learnopengl.com/Advanced-Lighting/Deferred-Shading

#include "common.h"

typedef u32 HSHADER;
typedef u32 HPOINTLIGHT;

#define INVALID_SHADER_HANDLE 0xFFFFFFFF
#define INVALID_POINTLIGHT_HANDLE 0xFFFFFFFF

#define BUFFER_VERT_SHADER_PATH "vert.glsl"
#define BUFFER_FRAG_SHADER_PATH "frag.glsl"

#define LIGHT_VERT_SHADER_PATH "lightvert.glsl"
#define LIGHT_FRAG_SHADER_PATH "lightfrag.glsl"

#define FWD_VERT_SHADER_PATH "fwdvert.glsl"
#define FWD_FRAG_SHADER_PATH "fwdfrag.glsl"

#define MAX_POINT_LIGHTS 32

typedef struct
{
    Vector3f Position;
    Vector3f Color;
} PointLight_t;

typedef struct {
    u32 GBuffer;
    u32 PositionBuffer;
    u32 NormalBuffer;
    u32 AlbedoSpecBuffer;

    HSHADER GeometryProgram;
    HSHADER LightProgram;
    HSHADER ForwardProgram;

    Matrix44f Projection;
    Matrix44f View;

    PointLight_t PointLights[MAX_POINT_LIGHTS];
    u32 PointLightCount;

    u32 ScreenQuadVAO;
    u32 ScreenQuadVBO;

    u32 CubeVAO;
    u32 CubeVBO;
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

HPOINTLIGHT DR_CreatePointLight(RenderContext_t* pContext, PointLight_t* pLight) 
{
    if (!pLight || !pContext || pContext->PointLightCount >= MAX_POINT_LIGHTS)
    {
        return INVALID_POINTLIGHT_HANDLE;
    }

    PointLight_t* pNext = &pContext->PointLights[pContext->PointLightCount];
    *pNext = *pLight;

    HPOINTLIGHT lightHandle = pContext->PointLightCount;
    pContext->PointLightCount++;

    return lightHandle;
}

PointLight_t* DR_GetPointLight(RenderContext_t* pContext, HPOINTLIGHT lightHandle)
{
    if (lightHandle == INVALID_POINTLIGHT_HANDLE || !pContext)
    {
        return NULL;
    }

    const u32 Index = lightHandle;
    if (Index >= MAX_POINT_LIGHTS || Index >= pContext->PointLightCount)
    {
        return NULL;
    }

    return &pContext->PointLights[Index];
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
    glUniformMatrix3fv(glGetUniformLocation(shaderHandle, pName), 1, GL_FALSE, &pVec->data[0]);
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
        pContext->GeometryProgram != INVALID_SHADER_HANDLE &&
        pContext->LightProgram != INVALID_SHADER_HANDLE &&
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
    pContext->GeometryProgram = INVALID_SHADER_HANDLE;
    pContext->LightProgram = INVALID_SHADER_HANDLE;
    pContext->ForwardProgram = INVALID_SHADER_HANDLE;

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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pContext->NormalBuffer, 0);

    // Color/specular component buffer (albedo)
    // Note the use of 8-bit precision
    glGenTextures(1, &pContext->AlbedoSpecBuffer);
    glBindTexture(GL_TEXTURE_2D, pContext->AlbedoSpecBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, pContext->AlbedoSpecBuffer, 0);

    // Tell OpenGL which buffers we'll be using to draw for the bound frame buffer
    u32 attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    u32 rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, pContext->GBuffer);

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

    pContext->GeometryProgram = DR_CreateShader(vertText, fragText);
    assert(pContext->GeometryProgram != INVALID_SHADER_HANDLE);

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
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &pContext->ScreenQuadVAO);
	glGenBuffers(1, &pContext->ScreenQuadVBO);
	glBindVertexArray(pContext->ScreenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pContext->ScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

    // Set up cube geometry
	static const float CubeVertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	glGenVertexArrays(1, &pContext->CubeVAO);
	glGenBuffers(1, &pContext->CubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, pContext->CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(pContext->CubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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

    glBindFramebuffer(GL_FRAMEBUFFER, pContext->GBuffer);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DR_UseShader(pContext->GeometryProgram);
    DR_SetShaderParameterMat4(
        pContext->GeometryProgram,
        "projection",
        &pContext->Projection);
    DR_SetShaderParameterMat4(
        pContext->GeometryProgram,
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

    // Perform the lighting pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DR_UseShader(pContext->LightProgram);

    DR_SetShaderParameteri(pContext->LightProgram, "gPosition", 0);
    DR_SetShaderParameteri(pContext->LightProgram, "gNormal", 1);
    DR_SetShaderParameteri(pContext->LightProgram, "gAlbedoSpec", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pContext->PositionBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pContext->NormalBuffer);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pContext->AlbedoSpecBuffer);

    for (u32 i = 0; i < pContext->PointLightCount; ++i)
    {
        PointLight_t* pLight = &pContext->PointLights[i];

        char positionParamName[256];
        sprintf(positionParamName, "lights[%d].Position", i);
        DR_SetShaderParameterVec3(
            pContext->LightProgram,
            positionParamName,
            &pLight->Position);

        char colorParamName[256];
        sprintf(colorParamName, "lights[%d].Color", i);
        DR_SetShaderParameterVec3(
            pContext->LightProgram,
            colorParamName,
            &pLight->Color);
    }

    Vector3f cameraPosition;
    Math_Vector3f_Zero(&cameraPosition);
    DR_SetShaderParameterVec3(pContext->LightProgram, "viewPos", &cameraPosition);

    // Render the screen quad
    glBindVertexArray(pContext->ScreenQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void DR_RenderCube(RenderContext_t* pContext)
{
    glBindVertexArray(pContext->CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
