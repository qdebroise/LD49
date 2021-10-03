#include "atom.h"

#include "array.h"
#include "camera.h"
#include "linalg.h"
#include "player.h"
#include "render.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdlib.h>

// @Todo: move this somewhere else.
static const uint32_t ATOM_SIZE = 100;
static const uint32_t NEUTRON_SIZE = 10;

typedef struct atom_t atom_t;
typedef struct atom_state_t atom_state_t;
typedef struct neutron_t neutron_t;
typedef struct atom_system_o atom_system_o;

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

// @Todo: change the API to atom_system_o, use neutrons pool and atom pool.
struct atom_system_o
{
    atom_t* atoms;

    SDL_Texture* atom_texture;
    SDL_Texture* neutron_texture;
};

struct atom_system_o* atom_system_create(struct SDL_Renderer* render)
{
    // @Note @Todo: see later about custom allocators.
    struct atom_system_o* system = malloc(sizeof(struct atom_system_o));
    system->atoms = NULL;
    system->atom_texture = load_bmp_to_texture(render, "assets/images/atom.bmp");
    system->neutron_texture = load_bmp_to_texture(render, "assets/images/neutron.bmp");

    return system;
}

void atom_system_destroy(struct atom_system_o* as)
{
    assert(as);

    for (uint32_t i = 0; i < array_size(as->atoms); ++i)
    {
        array_free(as->atoms[i].neutrons);
    }
    array_free(as->atoms);

    SDL_DestroyTexture(as->atom_texture);
    SDL_DestroyTexture(as->neutron_texture);
    free(as);
}

void atom_system_generate_atoms(struct atom_system_o* as, world_t world, uint32_t n)
{
    assert(as);

    const int32_t lower_x = world.bounds.west;
    const int32_t upper_x = world.bounds.east;
    const int32_t lower_y = world.bounds.south;
    const int32_t upper_y = world.bounds.north;
    atom_t* atoms = NULL; // array.

    for (uint32_t i = 0; i < n; ++i)
    {

        // @Todo: better pseudo-random generator.
        atom_t atom = {
            .pos = {
                .x = (rand() % (upper_x - lower_x + 1)) + lower_x,
                .y = (rand() % (upper_y - lower_y + 1)) + lower_y,
            },
            .state = {
                .num_exceeding_neutrons = 10,
                .unstability_duration_ms = 1000,
            },
        };
        array_push(atoms, atom);
    }

    if (as->atoms)
    {
        array_free(as->atoms);
        as->atoms = NULL;
    }

    as->atoms = atoms;
}

void atom_system_update(struct atom_system_o* as, struct player_o* player, float dt)
{
    assert(as && player);

    for (uint32_t i = 0; i < array_size(as->atoms); ++i)
    {
        atom_t* atom = &as->atoms[i];

        if (array_empty(atom->neutrons) && atom->state.num_exceeding_neutrons > 0)
        {
            // @Todo: emit several neutrons at a time
            // @Todo: emit in different patterns/behavior depending on the atom type.

            // Emit a new neutron in a random direction.
            atom->state.num_exceeding_neutrons -= 1;
            vec2_t dir = vec2_normalize((vec2_t){
                    ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f,
                    ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f});

            neutron_t neutron = {
                .pos = atom->pos,
                .dir = dir,
                .speed = ((float)rand() / RAND_MAX) * 0.5f,
                .bounding_circle_radius = NEUTRON_SIZE,
            };

            // printf("Emitting neutron: %f %f %f %f %f\n", neutron.pos.x, neutron.pos.y, neutron.dir.x, neutron.dir.y, neutron.speed);

            array_push(atom->neutrons, neutron);
        }

        // @Todo: some spatial collision detection ?

        for (uint32_t j = 0; j < array_size(atom->neutrons); ++j)
        {
            neutron_t* neutron = &atom->neutrons[j];
            neutron->pos = vec2_add(neutron->pos, vec2_mul_scalar(neutron->dir, neutron->speed * dt));

            bool delete_neutron = false;

            if (player_intersect_circle(
                    player, (circle_t){neutron->pos, neutron->bounding_circle_radius}))
            {
                // @Todo: player hit
                delete_neutron = true;
            }
            // @Todo: this is only temporary. Need proper boundary checking.
            else if (fabs(neutron->pos.x) > 2000 || fabs(neutron->pos.y) > 1000)
            {
                delete_neutron = true;
            }

            if (delete_neutron)
            {
                // Order isn't important, remove and replace with last in the array.
                if (array_size(atom->neutrons) == 1)
                {
                    array_pop(atom->neutrons);
                }
                else
                {
                    array_remove_fast(atom->neutrons, j);
                }
            }
        }
    }
}

void atom_system_draw(struct atom_system_o* as, struct camera_o* camera, struct SDL_Renderer* render)
{
    assert(as && camera && render);

    // @Todo: culling

    for (uint32_t i = 0; i < array_size(as->atoms); ++i)
    {
        atom_t atom = as->atoms[i];

        SDL_Rect rect = sdl_rect_from_pos_and_size(camera, atom.pos, (vec2_t){ATOM_SIZE, ATOM_SIZE});

        /*
        atom.state.num_exceeding_neutrons == 0
            ? SDL_SetRenderDrawColor(render, 255, 0, 0, 255)
            : SDL_SetRenderDrawColor(render, 0, 0, 255, 255);

        SDL_RenderDrawRect(render, &rect);
        SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
        */
        SDL_RenderCopy(render, as->atom_texture, NULL, &rect);

        for (uint32_t j = 0; j < array_size(atom.neutrons); ++j)
        {
            const neutron_t* neutron = &atom.neutrons[j];

            SDL_Rect rect = sdl_rect_from_pos_and_size(
                camera, neutron->pos, (vec2_t){NEUTRON_SIZE, NEUTRON_SIZE});
            // SDL_RenderDrawRect(render, &rect);
            SDL_RenderCopy(render, as->neutron_texture, NULL, &rect);
        }
    }
}
