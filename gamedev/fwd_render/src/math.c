#include "common.h"

#define PI 3.141592636

// Note: matrix convention is row-major, to be in line with glm
typedef struct
{
    float m[4][4];
} Matrix44f;

typedef struct
{
    union
    {
        struct
        {
            float x, y;
        };
        struct
        {
            float u, v;
        };
        float data[2];
    };
} Vector2f;

typedef struct
{
    union
    {
        struct 
        {
            float x, y, z;
        };
        struct 
        {
            float r, g, b;
        };
        float data[3];
    };
} Vector3f;

// General
float Math_ToRadians(float degrees)
{
    return degrees * (PI / 180.f);
}

// Vector2f
void Math_Vector2f_Zero(Vector2f* pV)
{
    pV->x = 0;
    pV->y= 0;
}

// Vector3f
void Math_Vector3f_Zero(Vector3f* pV)
{
    pV->x = 0;
    pV->y = 0;
    pV->z = 0;
}

void Math_Vector3f_Subtract(Vector3f* pA, Vector3f* pB, Vector3f* pResult)
{
    pResult->x = pA->x - pB->x;
    pResult->y = pA->y - pB->y;
    pResult->z = pA->z - pB->z;
}

void Math_Vector3f_Divide(Vector3f* pV, float scalar)
{
    pV->x /= scalar;
    pV->y /= scalar;
    pV->z /= scalar;
}

float Math_Vector3f_Dot(Vector3f* pA, Vector3f* pB)
{
    return
        pA->x * pB->x +
        pA->y * pB->y +
        pA->z * pB->z;
}

void Math_Vector3f_Cross(Vector3f* pA, Vector3f* pB, Vector3f* pCross)
{
    pCross->x = pA->y * pB->z - pA->z * pB->y;
    pCross->y = pA->z * pB->x - pA->x * pB->z;
    pCross->z = pA->x * pB->y - pA->y * pB->x;
}

float Math_Vector3f_Length(Vector3f* pV)
{
    const float LenSquared = Math_Vector3f_Dot(pV, pV);
    return sqrt(LenSquared);
}

void Math_Vector3f_Normalize(Vector3f* pV)
{
    const float Len = Math_Vector3f_Length(pV);
    Math_Vector3f_Divide(pV, Len);
}

// Matrix44f
void Math_Matrix44f_Zero(Matrix44f* pM)
{
    memset(pM, 0, sizeof(*pM));
}

void Math_Matrix44f_Identity(Matrix44f* pM)
{
    memset(pM, 0, sizeof(*pM));
    pM->m[0][0] = 1.f;
    pM->m[1][1] = 1.f;
    pM->m[2][2] = 1.f;
    pM->m[3][3] = 1.f;
}

void Math_Matrix44f_Translate(Matrix44f* pM, Vector3f* pV)
{
    pM->m[3][0] += pV->x;
    pM->m[3][1] += pV->y;
    pM->m[3][2] += pV->z;
}

void Math_Matrix44f_Scale(Matrix44f* pM, Vector3f* pV)
{
    pM->m[0][0] *= pV->x;
    pM->m[1][1] *= pV->y;
    pM->m[2][2] *= pV->z;
}

// Yoinked from cglm
void Math_Matrix44f_Perspective(Matrix44f* pM, float angle, float ratio, float near, float far)
{
    float f, fn;

    Math_Matrix44f_Zero(pM);

    f  = 1.0f / tanf(angle * 0.5f);
    fn = 1.0f / (near - far);

    pM->m[0][0] = f / ratio;
    pM->m[1][1] = f;
    pM->m[2][2] = (near + far) * fn;
    pM->m[2][3] = -1.f;
    pM->m[3][2] = 2.0f * near * far * fn;
}

// Yoinked from cglm
void Math_Matrix44f_LookAt(Vector3f* pEye, Vector3f* pCenter, Vector3f* pUp, Matrix44f* pLookAt)
{
    Vector3f f, u, s;

    Math_Vector3f_Subtract(pCenter, pEye, &f);
    Math_Vector3f_Normalize(&f);

	// glm_vec3_crossn(up, f, s);
	Math_Vector3f_Cross(pUp, &f, &s);
    Math_Vector3f_Normalize(&f);

    Math_Vector3f_Cross(&f, &s, &u);
	
    const float SEye = Math_Vector3f_Dot(&s, pEye);
    const float UEye = Math_Vector3f_Dot(&u, pEye);
    const float FEye = Math_Vector3f_Dot(&f, pEye);

    pLookAt->m[0][0] = s.x;
    pLookAt->m[0][1] = u.x;
    pLookAt->m[0][2] = f.x;
    pLookAt->m[1][0] = s.y;
    pLookAt->m[1][1] = u.y;
    pLookAt->m[1][2] = f.y;
    pLookAt->m[2][0] = s.z;
    pLookAt->m[2][1] = u.z;
    pLookAt->m[2][2] = f.z;
    pLookAt->m[3][0] = -SEye;
    pLookAt->m[3][1] = -UEye;
    pLookAt->m[3][2] = -FEye;
    pLookAt->m[0][3] = pLookAt->m[1][3] = pLookAt->m[2][3] = 0.0f;
    pLookAt->m[3][3] = 1.0f;
}
