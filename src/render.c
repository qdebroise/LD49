#include "render.h"
#include "camera.h"

#include <SDL2/SDL.h>

// @Todo: default to an in-memory created texture when loading failed ?

SDL_Texture* load_bmp_to_texture(struct SDL_Renderer* render, const char* file)
{
    SDL_Surface* surface = SDL_LoadBMP(file);
    if (!surface)
    {
        fprintf(stderr, "Couldn't load '%s': %s\n", file, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(render, surface);
    if (!texture)
    {
        fprintf(stderr, "Couldn't convert '%s' to texture: %s\n", file, SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect sdl_rect_from_pos_and_size(struct camera_o* camera, vec2_t pos, vec2_t size)
{
    vec2_t bl = {pos.x - size.x, pos.y - size.y};
    vec2_t tr = {pos.x + size.x, pos.y + size.y};

    vec2_t screen_bl = camera_world_to_screen(camera, bl);
    vec2_t screen_tr = camera_world_to_screen(camera, tr);

    float width = fabs(screen_tr.x - screen_bl.x);
    float height = fabs(screen_tr.y - screen_bl.y);

    // SDL_RenderFillRect needs top-left corner and not bottom-left hence the `- height`.
    return (SDL_Rect){screen_bl.x, screen_bl.y - height, width, height};
}

SDL_Rect sdl_rect_from_pos_and_size_with_scale(
    struct camera_o* camera,
    vec2_t pos,
    vec2_t size,
    float scale)
{
    mat3_t transform = {
        .x = {scale,     0, 0},
        .y = {    0, scale, 0},
        .z = {pos.x, pos.y, 1},
    };

    vec3_t bl = {-size.x, -size.y, 1};
    vec3_t tr = {+size.x, +size.y, 1};

    bl = mat3_mul_vec(transform, bl);
    tr = mat3_mul_vec(transform, tr);

    vec2_t screen_bl = camera_world_to_screen(camera, vec3_xy(bl));
    vec2_t screen_tr = camera_world_to_screen(camera, vec3_xy(tr));

    float width = fabs(screen_tr.x - screen_bl.x);
    float height = fabs(screen_tr.y - screen_bl.y);

    // SDL_RenderFillRect needs top-left corner and not bottom-left hence the `- height`.
    return (SDL_Rect){screen_bl.x, screen_bl.y - height, width, height};
}
