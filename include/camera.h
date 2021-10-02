#ifndef CAMERA_H_
#define CAMERA_H_

#include <SDL2/SDL_events.h>

#include "linalg.h"

struct camera_o;

struct camera_o* camera_create(vec2_t pos, vec2_t viewport);
void camera_destroy(struct camera_o*);
void camera_handle_event(struct camera_o*, SDL_Event event);
void camera_update(struct camera_o*);
vec2_t camera_pos(struct camera_o*);
mat3_t camera_view(struct camera_o*);
vec2_t camera_screen_to_world(struct camera_o*, vec2_t screen);
vec2_t camera_world_to_screen(struct camera_o* camera, vec2_t world);

#endif // CAMERA_H_


