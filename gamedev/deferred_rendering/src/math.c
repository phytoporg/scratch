#include "common.h"

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
    pM->m[0][2] += pV->x;
    pM->m[1][2] += pV->y;
    pM->m[2][2] += pV->z;
}

void Math_Matrix44f_Scale(Matrix44f* pM, Vector3f* pV)
{
    pM->m[0][0] *= pV->x;
    pM->m[1][1] *= pV->y;
    pM->m[2][2] *= pV->z;
}

// Yoinked from https://stackoverflow.com/questions/8115352/glmperspective-explanation
void Math_Matrix44f_Perspective(Matrix44f* pM, float angle, float ratio, float near, float far)
{
    Math_Matrix44f_Zero(pM);

    float tanHalfAngle = tan(angle / 2.f);
    pM->m[0][0] = 1.f / (ratio * tanHalfAngle);
    pM->m[1][1] = 1.f / (tanHalfAngle);
    pM->m[2][2] = -(far + near) / (far - near);
    pM->m[3][2] = -1.f;
    pM->m[2][3] = -(2.f * far * near) / (far - near);
}

// Yoinked from https://stackoverflow.com/questions/21830340/understanding-glmlookat
void Math_Matrix44f_LookAt(Vector3f* pEye, Vector3f* pCenter, Vector3f* pUp, Matrix44f* pLookAt)
{
    Vector3f X, Y, Z;

    Math_Vector3f_Subtract(pEye, pCenter, &Z);
    Y = *pUp;
    Math_Vector3f_Cross(&Y, &Z, &X);

    Math_Vector3f_Normalize(&X);
    Math_Vector3f_Normalize(&Y);
    Math_Vector3f_Normalize(&Z);

    const float XDotEye = Math_Vector3f_Dot(&X, pEye);
    const float YDotEye = Math_Vector3f_Dot(&Y, pEye);
    const float ZDotEye = Math_Vector3f_Dot(&Z, pEye);

    pLookAt->m[0][0] = X.x;
    pLookAt->m[1][0] = X.y;
    pLookAt->m[2][0] = X.z;
    pLookAt->m[3][0] = -XDotEye;
    pLookAt->m[0][1] = Y.x;
    pLookAt->m[1][1] = Y.y;
    pLookAt->m[2][1] = Y.z;
    pLookAt->m[3][1] = -YDotEye;
    pLookAt->m[0][2] = Z.x;
    pLookAt->m[1][2] = Z.y;
    pLookAt->m[2][2] = Z.z;
    pLookAt->m[3][2] = -ZDotEye;
    pLookAt->m[0][3] = 0;
    pLookAt->m[1][3] = 0;
    pLookAt->m[2][3] = 0;
    pLookAt->m[3][3] = 1.0f;
}
