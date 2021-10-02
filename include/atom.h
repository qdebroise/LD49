#ifndef ATOM_H_
#define ATOM_H_

#include "linalg.h"

#include <stdint.h>

struct camera_o;
struct player_o;
struct SDL_Renderer;

struct atom_system_o;

struct atom_system_o* atom_system_create(struct SDL_Renderer*);
void atom_system_destroy(struct atom_system_o*);

void atom_system_generate_atoms(struct atom_system_o*, uint32_t n);
void atom_system_draw(struct atom_system_o*, struct camera_o*, struct SDL_Renderer*);
void atom_system_update(struct atom_system_o*, struct player_o*, float dt);

#endif // ATOM_H_


