#ifndef ATOM_H_
#define ATOM_H_

#include "linalg.h"

#include <stdint.h>

struct camera_o;
struct player_o;
struct SDL_Renderer;

// @Todo: change the API to atom_system_o, use neutrons pool and atom pool.

typedef struct atom_t atom_t;
typedef struct atom_state_t atom_state_t;
typedef struct neutron_t neutron_t;

struct neutron_t
{
    vec2_t pos;
    vec2_t dir;
    float speed;
    float bounding_circle_radius;
};

struct atom_state_t
{
    uint32_t num_exceeding_neutrons;
    uint32_t unstability_duration_ms;
};

struct atom_t
{
    vec2_t pos;
    atom_state_t state;
    /* array */ neutron_t* neutrons;
};

atom_t* atoms_generate(uint32_t n);
void atoms_draw(/* array */ const atom_t*, struct camera_o*, struct SDL_Renderer*);
void atoms_update(/* array */ atom_t*, struct player_o*, float dt);

#endif // ATOM_H_


