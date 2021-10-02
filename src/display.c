#include "display.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct display_o
{
    struct SDL_Window* window;
    struct SDL_Renderer* render;

    uint32_t logical_width;
    uint32_t logical_height;
};

struct display_o* display_create(uint32_t width, uint32_t height)
{
    // @Note @Todo: see later about custom allocators.
    struct display_o* display = malloc(sizeof(struct display_o));
    display->logical_width = width,
    display->logical_height = height,

    display->window = SDL_CreateWindow(
        "Test",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_RESIZABLE);

    if (display->window)
    {
        display->render = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);

        if (display->render)
        {
            SDL_RenderSetLogicalSize(display->render, display->logical_width, display->logical_height);
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

void display_destroy(struct display_o* display)
{
    assert(display);

    SDL_DestroyRenderer(display->render);
    SDL_DestroyWindow(display->window);
    free(display);
}

void display_set_title(struct display_o* display, const char* title)
{
    assert(display);

    // @Todo: does SDL internally make a copy of the string ?
    SDL_SetWindowTitle(display->window, title);
}

struct SDL_Renderer* display_get_renderer(struct display_o* display)
{
    assert(display);
    return display->render;
}
