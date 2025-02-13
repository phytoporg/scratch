// Macros
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Vectors
typedef struct 
{
    int x;
    int y;
} Vector2i;

typedef struct 
{
    float x;
    float y;
} Vector2f;

Vector2f Vector2f_mult_scalar(Vector2f* pVector, float s)
{
    return (Vector2f) {
        .x = pVector->x * s,
        .y = pVector->y * s
    };
}

float Vector2f_len(Vector2f* pVector)
{
    return sqrtf(
        pVector->x * pVector->x +
        pVector->y * pVector->y
    );
}

Vector2f Vector2f_normalize(Vector2f* pVector)
{
    const float len = Vector2f_len(pVector); 
    return Vector2f_mult_scalar(pVector, 1.f / len);
}

Vector2i Vector2f_to_Vector2i(Vector2f* pVector)
{
    return (Vector2i) {
        .x = (int)pVector->x,
        .y = (int)pVector->y
    };
}


// Rects
typedef struct 
{
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct
{
    int Left;
    int Right;
    int Top;
    int Bottom;
} Extents;

Extents rect_to_extents(Rect* pRect)
{
    const Extents rectExtents = {
        .Left = pRect->x,
        .Right = pRect->x + pRect->w,
        .Top = pRect->y,
        .Bottom = pRect->y + pRect->h
    };
    return rectExtents;
}

int rect_extent_top(Rect* pRect)
{
    return pRect->y;
}

int rect_extent_bottom(Rect* pRect)
{
    return pRect->y + pRect->h;
}

int rect_extent_left(Rect* pRect)
{
    return pRect->x;
}

int rect_extent_right(Rect* pRect)
{
    return pRect->x + pRect->w;
}

void rect_set_pos(Rect* pRect, Vector2i* pPos)
{
    pRect->x = pPos->x;
    pRect->y = pPos->y;
}

Vector2i rect_get_pos(Rect* pRect)
{
    return (Vector2i) { .x = pRect->x, .y = pRect->y };
}

bool rect_test_intersects(Rect* pA, Rect* pB)
{
    const Extents extentsA = rect_to_extents(pA);
    const Extents extentsB = rect_to_extents(pB);

    return
        extentsA.Right >= extentsB.Left &&
        extentsA.Left <= extentsB.Right &&
        extentsA.Top <= extentsB.Bottom &&
        extentsA.Bottom >= extentsB.Top;
}

typedef struct
{
    bool Intersects;
    Vector2i Separation;
} RectIntersectResult;

bool rect_test_intersect_result(Rect* pA, Rect* pB, RectIntersectResult* pResult)
{
    memset(pResult, 0, sizeof(*pResult));
    const Extents extentsA = rect_to_extents(pA);
    const Extents extentsB = rect_to_extents(pB);

    pResult->Intersects =
        extentsA.Right >= extentsB.Left &&
        extentsA.Left <= extentsB.Right &&
        extentsA.Top <= extentsB.Bottom &&
        extentsA.Bottom >= extentsB.Top;

    if (pResult->Intersects)
    {
        const int overlapX = extentsA.Right < extentsB.Right ? extentsA.Right - extentsB.Left : extentsB.Right - extentsA.Left;
        const int overlapY = extentsA.Top < extentsB.Top ? extentsA.Bottom - extentsB.Top : extentsB.Bottom - extentsA.Top;
        pResult->Separation = (Vector2i) { .x = overlapX, .y = overlapY };
    }

    return pResult->Intersects;
}
