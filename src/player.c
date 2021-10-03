#include "player.h"

#include "camera.h"
#include "linalg.h"
#include "render.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdlib.h>

typedef struct player_o player_o;

// @Todo: move this somewhere else.
static const vec2_t PLAYER_SIZE = {60, 40};

struct player_o
{
    vec2_t pos;
    vec2_t dir;
    float bounding_circle_radius;

    vec2_t target;
    bool move;
    float speed;

    bool is_dead;

    SDL_Texture* texture;
};

player_o* player_create(struct SDL_Renderer* render)
{
    // @Note @Todo: see later about custom allocators.
    player_o* player = malloc(sizeof(struct player_o));
    player->pos = (vec2_t){0, 0};
    player->bounding_circle_radius = PLAYER_SIZE.x;
    player->is_dead = false;
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

void player_update(struct player_o* player, world_t world, float dt)
{
    assert(player);

    static const float SLOWDOWN_FACTOR = 0.9f;
    static const float DISTANCE_PERCENT = 0.005f;

    // Slowdown.
    if (!player->move)
    {
        player->speed *= SLOWDOWN_FACTOR;
    }

    // Update the target point for this update taking boundaries into account.
    // This has the nice property of offering slowdown when close to world borders.
    vec2_t target = player->target;
    if (target.x > world.bounds.east) { target.x = world.bounds.east; }
    else if (target.x < world.bounds.west) { target.x = world.bounds.west; }
    if (target.y < world.bounds.south) { target.y = world.bounds.south; }
    else if (target.y > world.bounds.north) { target.y = world.bounds.north; }

    vec2_t to_target = vec2_sub(target, player->pos);
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
        camera, player->pos, PLAYER_SIZE);

    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    // SDL_RenderDrawRect(render, &rect);
    SDL_RendererFlip flip = player->dir.x >= 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(render, player->texture, NULL, &rect, 0, NULL, flip);
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

void player_die(struct player_o* player)
{
    assert(player);
    player->is_dead = true;
}

bool player_is_dead(const struct player_o* player)
{
    assert(player);
    return player->is_dead;
}

