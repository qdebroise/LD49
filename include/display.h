#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

struct SDL_Window;
struct SDL_Renderer;

struct display_o;

struct display_o* display_create(uint32_t width, uint32_t height);
void display_destroy(struct display_o*);
void display_set_title(struct display_o*, const char* title);
struct SDL_Renderer* display_get_renderer(struct display_o*);

#endif // DISPLAY_H_


