#include "atom.h"

#include "audio.h"
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
static const uint32_t NEUTRON_SIZE = 8;

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
    uint32_t num_left;
    uint32_t num_exceeding_neutrons;
    uint32_t unstability_duration_ms;
};

struct atom_t
{
    vec2_t pos;
    atom_state_t state;
    /* array */ neutron_t* neutrons;
    void (*emit_neutron)(struct atom_t*, float);
};

// @Todo: change the API to atom_system_o, use neutrons pool and atom pool.
struct atom_system_o
{
    atom_t* atoms;
    float angle;
    float angle_increment;

    SDL_Texture* atom_texture;
    SDL_Texture* neutron_texture;
};

static void emit_neutron_random(atom_t* atom, float dt)
{
    vec2_t dir = vec2_normalize((vec2_t){
            ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f,
            ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f});

    neutron_t neutron = {
        .pos = atom->pos,
        .dir = dir,
        .speed = (((float)rand() / RAND_MAX) * 0.4f + 0.1f) * 0.05f * dt,
        .bounding_circle_radius = NEUTRON_SIZE,
    };

    array_push(atom->neutrons, neutron);
}

static void emit_neutron_circle(atom_t* atom, float dt)
{
    static const uint32_t NUM_EMIT = 8;
    const float angle_step = 2*PI_f / NUM_EMIT;

    for (uint32_t i = 0; i < NUM_EMIT; ++i)
    {
        vec2_t dir = {cosf(angle_step*i), sinf(angle_step*i)};
        neutron_t neutron = {
            .pos = atom->pos,
            .dir = dir,
            .speed = (((float)rand() / RAND_MAX) * 0.4f + 0.1f) * 0.05f * dt,
            .bounding_circle_radius = NEUTRON_SIZE,
        };

        array_push(atom->neutrons, neutron);
    }
}

struct atom_system_o* atom_system_create(struct SDL_Renderer* render)
{
    // @Note @Todo: see later about custom allocators.
    struct atom_system_o* system = malloc(sizeof(struct atom_system_o));
    system->atoms = NULL;
    system->atom_texture = load_bmp_to_texture(render, "assets/images/atom.bmp");
    system->neutron_texture = load_bmp_to_texture(render, "assets/images/neutron.bmp");
    system->angle = 0;
    system->angle_increment = 0.0005;

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

void atom_system_generate_atoms(
    struct atom_system_o* as,
    struct player_o* player,
    world_t world,
    uint32_t n)
{
    assert(as);

    const int32_t lower_x = world.bounds.west;
    const int32_t upper_x = world.bounds.east;
    const int32_t lower_y = world.bounds.south;
    const int32_t upper_y = world.bounds.north;
    atom_t* atoms = NULL; // array.

    const float atom_bounding_circle_radius = sqrtf(2*ATOM_SIZE*ATOM_SIZE);

    // @Todo: this is using brute force to place atoms. Do something better! Place them using
    // poisson-disc sampling or some spatial partionning.
    uint32_t left = n;
    while (left)
    {
        vec2_t candidate_pos = {
            .x = (rand() % (upper_x - lower_x + 1)) + lower_x,
            .y = (rand() % (upper_y - lower_y + 1)) + lower_y,
        };

        bool valid_candidate = true;

        const atom_t* end = atoms + array_size(atoms);
        for (const atom_t* atom = atoms; atom < end; atom++)
        {
            circle_t c1 = {.center = atom->pos, .radius = atom_bounding_circle_radius};
            circle_t c2 = {.center = candidate_pos, .radius = atom_bounding_circle_radius};
            if (circle_intersect(c1, c2) || player_intersect_circle(player, c2))
            {
                valid_candidate = false;
                break;
            }
        }

        if (valid_candidate)
        {
            atom_t atom = {
                .pos = candidate_pos,
                .state = {
                    .num_left = 10,
                    .num_exceeding_neutrons = 10,
                    .unstability_duration_ms = 1000,
                },
                .neutrons = NULL,
                .emit_neutron = &emit_neutron_random,
                // .emit_neutron = &emit_neutron_circle,
            };
            array_push(atoms, atom);
            left--;
        }
    }

    if (as->atoms)
    {
        array_free(as->atoms);
    }

    as->atoms = atoms;
}

bool atom_system_all_stable(const struct atom_system_o* as)
{
    uint32_t num_stable_atoms = 0;
    const atom_t* end = as->atoms + array_size(as->atoms);
    for (const atom_t* atom = as->atoms; atom < end; atom++)
    {
        if (atom->state.num_left == 0)
        {
            num_stable_atoms++;
        }
    }

    return num_stable_atoms == array_size(as->atoms);
}

void atom_system_update(
    struct atom_system_o* as,
    struct audio_system_o* audio,
    struct player_o* player,
    world_t world,
    float dt)
{
    assert(as && player);

    // @Todo: smooth things out.
    as->angle += as->angle_increment * dt;
    if (as->angle >= PI_4_f/2 || as->angle <= - PI_4_f/2)
    {
        as->angle_increment = -as->angle_increment;
    }

    bool neutron_emitted_this_update = false;
    bool atom_stable_this_update = false;

    for (uint32_t i = 0; i < array_size(as->atoms); ++i)
    {
        atom_t* atom = &as->atoms[i];

        if (array_empty(atom->neutrons) && atom->state.num_left > 0)
        {
            // Emit a new neutron in a random direction.

            // @Todo: emit several neutrons at a time
            // @Todo: emit in different patterns/behavior depending on the atom type.

            atom->state.num_left -= 1;

            neutron_emitted_this_update = true;
            if (atom->state.num_left == 0)
            {
                atom_stable_this_update = true;
            }

            atom->emit_neutron(atom, dt);
            /*
            vec2_t dir = vec2_normalize((vec2_t){
                    ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f,
                    ((float)rand() / RAND_MAX - 0.5f) * 2 * 2*PI_f});

            neutron_t neutron = {
                .pos = atom->pos,
                .dir = dir,
                .speed = (((float)rand() / RAND_MAX) * 0.4f + 0.1f) * 0.05f * dt,
                .bounding_circle_radius = NEUTRON_SIZE,
            };

            array_push(atom->neutrons, neutron);
            */
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
                player_die(player);
                delete_neutron = true;
            }
            else if (neutron->pos.x >= world.bounds.east || neutron->pos.x <= world.bounds.west
                || neutron->pos.y >= world.bounds.north || neutron->pos.y <= world.bounds.south)
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

            // printf("%d %d\n", array_size(atom->neutrons), atom->state.num_left);
        }
    }

    if (neutron_emitted_this_update)
    {
        audio_system_play_sound(audio, AUDIO_ENTRY_EMIT_NEUTRON);
    }
    if (atom_stable_this_update)
    {
        audio_system_play_sound(audio, AUDIO_ENTRY_ATOM_STABLE);
    }
}

static void draw_stability_bar(atom_t atom, struct camera_o* camera, SDL_Renderer* render)
{
    static const float OFFSET = ATOM_SIZE;
    static const vec2_t BAR_OUTLINE_SIZE = {ATOM_SIZE, 10};

    vec2_t bar_outline_pos = vec2_add(atom.pos, (vec2_t){0, OFFSET});
    SDL_Rect rect_outline = sdl_rect_from_pos_and_size(camera, bar_outline_pos, BAR_OUTLINE_SIZE);

    float fill_percent = (float)atom.state.num_left / atom.state.num_exceeding_neutrons;
    vec2_t bar_size = {BAR_OUTLINE_SIZE.x * (1 - fill_percent) - 2, BAR_OUTLINE_SIZE.y - 2};
    vec2_t bar_pos = {
        bar_outline_pos.x - BAR_OUTLINE_SIZE.x / 2 + bar_size.x / 2 + 1,
        bar_outline_pos.y,
    };
    SDL_Rect rect_bar = sdl_rect_from_pos_and_size(camera, bar_pos, bar_size);

    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    SDL_RenderDrawRect(render, &rect_outline);
    SDL_SetRenderDrawColor(render, 50, 50, 255, 255);
    SDL_RenderFillRect(render, &rect_bar);
}

void atom_system_draw(struct atom_system_o* as, struct camera_o* camera, struct SDL_Renderer* render)
{
    assert(as && camera && render);

    // @Todo: culling

    for (uint32_t i = 0; i < array_size(as->atoms); ++i)
    {
        atom_t atom = as->atoms[i];

        if (atom.state.num_left > 0)
        {
            SDL_Rect rect = sdl_rect_from_pos_and_size_with_scale(camera, atom.pos, (vec2_t){ATOM_SIZE, ATOM_SIZE}, 1 + sinf(as->angle)*0.3);
            SDL_RenderCopyEx(render, as->atom_texture, NULL, &rect, degrees(sinf(as->angle)), NULL, SDL_FLIP_NONE);
        }
        else
        {
            // @Todo: smooth transition instead of stopping directly.
            SDL_Rect rect = sdl_rect_from_pos_and_size_with_scale(camera, atom.pos, (vec2_t){ATOM_SIZE, ATOM_SIZE}, 0.5);
            SDL_RenderCopy(render, as->atom_texture, NULL, &rect);
        }

        /*
        atom.state.num_left == 0
            ? SDL_SetRenderDrawColor(render, 255, 0, 0, 255)
            : SDL_SetRenderDrawColor(render, 0, 0, 255, 255);

        SDL_RenderDrawRect(render, &rect);
        SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
        */

        for (uint32_t j = 0; j < array_size(atom.neutrons); ++j)
        {
            const neutron_t* neutron = &atom.neutrons[j];

            SDL_Rect rect = sdl_rect_from_pos_and_size(
                camera, neutron->pos, (vec2_t){NEUTRON_SIZE, NEUTRON_SIZE});
            // SDL_RenderDrawRect(render, &rect);
            SDL_RenderCopy(render, as->neutron_texture, NULL, &rect);
        }

        if (atom.state.num_left > 0)
        {
            draw_stability_bar(atom, camera, render);
        }
    }
}
