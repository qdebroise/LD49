#include "render.h"

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
