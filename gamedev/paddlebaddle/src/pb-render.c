#define FIELD_BORDER_PX 10

void RenderColoredRect(
    SDL_Renderer* pRender,
    const SDL_Color* pColor,
    Rect* pRect)
{
    SDL_SetRenderDrawColor( pRender,
        pColor->r,
        pColor->g,
        pColor->b,
        pColor->a);

    const SDL_Rect rect = {
        .x = pRect->x,
        .y = pRect->y,
        .w = pRect->w,
        .h = pRect->h
    };
    SDL_RenderFillRect(pRender, &rect);
}

// Using knowledge about gameplay state but whatever
void RenderField(SDL_Renderer* pRender, SDL_Color* pColor, Rect* pRect)
{
    // Upper
    Rect topRect = {
        .x = pRect->x,
        .y = pRect->y - FIELD_BORDER_PX,
        .w = pRect->w,
        .h = FIELD_BORDER_PX,
    };
    RenderColoredRect(pRender, pColor, &topRect);

    // Lower
    Rect bottomRect = {
        .x = pRect->x,
        .y = pRect->y + pRect->h,
        .w = pRect->w,
        .h = FIELD_BORDER_PX,
    };
    RenderColoredRect(pRender, pColor, &bottomRect);
}

