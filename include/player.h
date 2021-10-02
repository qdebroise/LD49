#ifndef PLAYER_H_
#define PLAYER_H_

#include "linalg.h"

#include <SDL2/SDL_events.h>

struct player_o;
struct camera_o;
struct SDL_Renderer;

struct player_o* player_create(void);
void player_destroy(struct player_o*);
void player_update(struct player_o*);
void player_handle_event(struct player_o*, struct camera_o*, SDL_Event event);
void player_draw(struct player_o*, struct camera_o*, struct SDL_Renderer*);

bool player_intersect_circle(struct player_o*, circle_t);

#endif // PLAYER_H_


