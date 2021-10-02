#include "display.h"

#include <SDL2/SDL.h>

#include <stdio.h>

display_t display_create(uint32_t width, uint32_t height)
{
    display_t display = {
        .logical_width = width,
        .logical_height = height,
    };

    display.window = SDL_CreateWindow(
        "Test",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_RESIZABLE);

    if (display.window)
    {
        display.render = SDL_CreateRenderer(display.window, -1, SDL_RENDERER_ACCELERATED);

        if (display.render)
        {
            SDL_RenderSetLogicalSize(display.render, display.logical_width, display.logical_height);
        }
        else
        {
            fprintf(stderr, "Error creating the renderer: %s\n", SDL_GetError());
        }
    }
    else
    {
        fprintf(stderr, "Error creating the window: %s\n", SDL_GetError());
    }

    return display;
}

void display_destroy(display_t display)
{
    SDL_DestroyRenderer(display.render);
    SDL_DestroyWindow(display.window);
}

void display_set_title(display_t display, const char* title)
{
    // @Todo: does SDL internally make a copy of the string ?
    SDL_SetWindowTitle(display.window, title);
}
