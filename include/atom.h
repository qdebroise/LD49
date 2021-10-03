#ifndef ATOM_H_
#define ATOM_H_

#include "linalg.h"
#include "world.h"

#include <stdint.h>

struct audio_system_o;
struct camera_o;
struct player_o;
struct SDL_Renderer;

struct atom_system_o;

struct atom_system_o* atom_system_create(struct SDL_Renderer*);
void atom_system_destroy(struct atom_system_o*);

void atom_system_generate_atoms(struct atom_system_o*, struct player_o*, world_t, uint32_t n);
void atom_system_draw(struct atom_system_o*, struct camera_o*, struct SDL_Renderer*);
void atom_system_update(
    struct atom_system_o*,
    struct audio_system_o*,
    struct player_o*,
    world_t,
    float dt);
bool atom_system_all_stable(const struct atom_system_o*);

#endif // ATOM_H_


