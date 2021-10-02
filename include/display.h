#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

struct SDL_Window;
struct SDL_Renderer;

typedef struct display_t display_t;

struct display_t
{
    struct SDL_Window* window;
    struct SDL_Renderer* render;

    uint32_t logical_width;
    uint32_t logical_height;
};

display_t display_create(uint32_t width, uint32_t height);
void display_destroy(display_t);
void display_set_title(display_t, const char* title);

#endif // DISPLAY_H_


