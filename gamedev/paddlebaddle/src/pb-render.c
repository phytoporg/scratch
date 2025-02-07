#define FIELD_BORDER_PX 10

void _RenderColoredRect(
    SDL_Renderer* pRender,
    const SDL_Color* pColor,
    int x, int y, int w, int h)
{
    SDL_SetRenderDrawColor( pRender,
        pColor->r,
        pColor->g,
        pColor->b,
        pColor->a);

    const SDL_Rect rect = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
    SDL_RenderFillRect(pRender, &rect);
}

void RenderField(SDL_Renderer* pRender, int x, int y, int w, int h)
{
    const SDL_Color FieldBorderColor = { 255, 255, 255, SDL_ALPHA_OPAQUE };

    // Upper
    _RenderColoredRect(
        pRender, &FieldBorderColor, x, y - FIELD_BORDER_PX, w, FIELD_BORDER_PX);
    // Lower
    _RenderColoredRect(
        pRender, &FieldBorderColor, x, y + h, w, FIELD_BORDER_PX);
}

void RenderRightPaddle(SDL_Renderer* pRender, int x, int y, int w, int h)
{
    // Red-ish
    const SDL_Color RightPaddleColor = { 200, 50, 0, SDL_ALPHA_OPAQUE };

    _RenderColoredRect(
        pRender, &RightPaddleColor, x, y, w, h);
}

void RenderLeftPaddle(SDL_Renderer* pRender, int x, int y, int w, int h)
{
    // Blue-ish
    const SDL_Color LeftPaddleColor = { 0, 100, 255, SDL_ALPHA_OPAQUE };

    _RenderColoredRect(
        pRender, &LeftPaddleColor, x, y, w, h);
}
