#ifndef RENDER_H_
#define RENDER_H_

#include "linalg.h"

#include <SDL2/SDL_rect.h>

struct camera_o;

struct SDL_Texture;
struct SDL_Renderer;

struct SDL_Texture* load_bmp_to_texture(struct SDL_Renderer*, const char* file);

// Take the min/max from the bounding box in world space and convert them to screen space.
// Then compute the SDL_Rect used for drawing the box.
SDL_Rect sdl_rect_from_pos_and_size(struct camera_o*, vec2_t pos, vec2_t size);

#endif // RENDER_H_


