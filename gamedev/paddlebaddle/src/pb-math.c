
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

// Vectors
typedef struct 
{
    int x;
    int y;
} Vector2i;

