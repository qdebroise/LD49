#include "player.h"

#include "camera.h"
#include "linalg.h"
#include "render.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdlib.h>

typedef struct player_o player_o;

// @Todo: move this somewhere else.
static const uint32_t PLAYER_SIZE = 30;

struct player_o
{
    vec2_t pos;
    vec2_t dir;
    float bounding_circle_radius;

    vec2_t target;
    bool move;
    float speed;

    SDL_Texture* texture;
};

player_o* player_create(struct SDL_Renderer* render)
{
    // @Note @Todo: see later about custom allocators.
    player_o* player = malloc(sizeof(struct player_o));
    player->pos = (vec2_t){0, 0};
    player->bounding_circle_radius = PLAYER_SIZE;
    player->texture = load_bmp_to_texture(render, "assets/images/cat.bmp");

    assert(player->texture);

    return player;
}

void player_destroy(struct player_o* player)
{
    SDL_DestroyTexture(player->texture);
    assert(player);
    free(player);
}

void player_update(struct player_o* player, float dt)
{
    assert(player);

    static const float SLOWDOWN_FACTOR = 0.9f;
    static const float DISTANCE_PERCENT = 0.005f;

    // Slowdown.
    if (!player->move)
    {
        player->speed *= SLOWDOWN_FACTOR;
    }

    vec2_t to_target = vec2_sub(player->target, player->pos);
    if (!(to_target.x < 1e-5 && to_target.y < 1e-5)) // @Todo: this is a bit hacky.
    {
        player->dir = vec2_normalize(to_target);
    }
    player->speed = DISTANCE_PERCENT * vec2_length(to_target);
    player->pos = vec2_add(player->pos, vec2_mul_scalar(player->dir, player->speed * dt));
}

void player_handle_event(struct player_o* player, struct camera_o* camera, SDL_Event event)
{
    assert(player && camera);

    static const float INITIAL_SPEED = 0.01f;

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        player->move = true;
        player->speed = INITIAL_SPEED;

        vec2_t mouse = camera_screen_to_world(camera, (vec2_t){event.button.x, event.button.y});
        player->dir = vec2_normalize(vec2_sub(mouse, player->pos));
        player->target = mouse;
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        // player->move = false;
    }
    else if (event.type == SDL_MOUSEMOTION && player->move)
    {
        vec2_t mouse = camera_screen_to_world(camera, (vec2_t){event.button.x, event.button.y});
        player->dir = vec2_normalize(vec2_sub(mouse, player->pos));
        player->target = mouse;
    }
}

void player_draw(struct player_o* player, struct camera_o* camera, struct SDL_Renderer* render)
{
    assert(player && camera && render);

    SDL_Rect rect = sdl_rect_from_pos_and_size(
        camera, player->pos, (vec2_t){PLAYER_SIZE, PLAYER_SIZE});

    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    SDL_RenderDrawRect(render, &rect);
    SDL_RenderCopy(render, player->texture, NULL, &rect);
}

bool player_intersect_circle(struct player_o* player, circle_t other)
{
    assert(player);
    return circle_intersect((circle_t){player->pos, player->bounding_circle_radius}, other);
}

vec2_t player_position(const struct player_o* player)
{
    assert(player);
    return player->pos;
}
