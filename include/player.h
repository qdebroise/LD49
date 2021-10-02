#ifndef PLAYER_H_
#define PLAYER_H_

#include <SDL2/SDL_events.h>

struct player_o;
struct camera_o;
struct SDL_Renderer;

struct player_o* player_create(void);
void player_destroy(struct player_o*);
void player_update(struct player_o*);
void player_handle_event(struct player_o*, struct camera_o*, SDL_Event event);
void player_draw(struct player_o*, struct camera_o*, struct SDL_Renderer*);

#endif // PLAYER_H_


