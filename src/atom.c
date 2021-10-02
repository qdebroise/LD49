#include "atom.h"

#include "array.h"
#include "camera.h"
#include "linalg.h"
#include "player.h"

#include <SDL2/SDL.h>

#include <stdlib.h>
#include <assert.h>

atom_t* atoms_generate(uint32_t n)
{
    const int32_t lower_x = -1280 / 2;
    const int32_t upper_x = 1280 / 2;
    const int32_t lower_y = -720 / 2;
    const int32_t upper_y = 720 / 2;
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

    return atoms;
}

void atoms_destroy(atom_t* atoms)
{
    for (uint32_t i = 0; i < array_size(atoms); ++i)
    {
        array_free(atoms->neutrons);
    }
    array_free(atoms);
}

void atoms_update(atom_t* atoms, struct player_o* player, float dt)
{
    for (uint32_t i = 0; i < array_size(atoms); ++i)
    {
        atom_t* atom = &atoms[i];

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
                .speed = ((float)rand() / RAND_MAX) * 5 + 0.1,
                .bounding_circle_radius = 5,
            };

            // printf("Emitting neutron: %f %f %f %f %f\n", neutron.pos.x, neutron.pos.y, neutron.dir.x, neutron.dir.y, neutron.speed);

            array_push(atom->neutrons, neutron);
        }

        for (uint32_t j = 0; j < array_size(atom->neutrons); ++j)
        {
            neutron_t* neutron = &atom->neutrons[j];
            neutron->pos = vec2_add(neutron->pos, vec2_mul_scalar(neutron->dir, neutron->speed * dt));

            bool delete_neutron = false;

            if (player_intersect_circle(
                    player, (circle_t){neutron->pos, neutron->bounding_circle_radius}))
            {
                printf("You die\n");
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

void atoms_draw(const atom_t* atoms, struct camera_o* camera, struct SDL_Renderer* render)
{
    assert(camera && render);

    for (uint32_t i = 0; i < array_size(atoms); ++i)
    {
        atom_t atom = atoms[i];

        vec2_t bl = {atom.pos.x - 20, atom.pos.y - 20};
        vec2_t tr = {atom.pos.x + 20, atom.pos.y + 20};

        vec2_t screen_bl = camera_world_to_screen(camera, bl);
        vec2_t screen_tr = camera_world_to_screen(camera, tr);
        float width = fabs(screen_tr.x - screen_bl.x);
        float height = fabs(screen_tr.y - screen_bl.y);

        if (atom.state.num_exceeding_neutrons == 0)
        {
            SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
        }

        // SDL_RenderFillRect needs top-left corner and not bottom-left.
        // SDL_RenderFillRect(render, &(SDL_Rect){screen_bl.x, screen_bl.y - height, width, height});
        SDL_RenderDrawRect(render, &(SDL_Rect){screen_bl.x, screen_bl.y - height, width, height});

        SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
        for (uint32_t j = 0; j < array_size(atom.neutrons); ++j)
        {
            const neutron_t* neutron = &atom.neutrons[j];

            vec2_t screen = camera_world_to_screen(camera, neutron->pos);
            // SDL_RenderDrawPoint(render, screen.x, screen.y);
            SDL_RenderFillRect(render, &(SDL_Rect){screen.x, screen.y - 5, 5, 5});
        }
    }
}
