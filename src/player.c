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

void player_update(struct player_o* player)
{
    assert(player);
}

void player_handle_event(struct player_o* player, struct camera_o* camera, SDL_Event event)
{
    assert(player && camera);

    if (event.type == SDL_MOUSEMOTION)
    {
        player->pos = camera_screen_to_world(camera, (vec2_t){event.motion.x, event.motion.y});
    }
}

void player_draw(struct player_o* player, struct camera_o* camera, struct SDL_Renderer* render)
{
    assert(player && camera && render);

    vec2_t bl = {player->pos.x - PLAYER_SIZE, player->pos.y - PLAYER_SIZE};
    vec2_t tr = {player->pos.x + PLAYER_SIZE, player->pos.y + PLAYER_SIZE};

    vec2_t screen_bl = camera_world_to_screen(camera, bl);
    vec2_t screen_tr = camera_world_to_screen(camera, tr);
    float width = fabs(screen_tr.x - screen_bl.x);
    float height = fabs(screen_tr.y - screen_bl.y);

    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    // SDL_RenderFillRect needs top-left corner and not bottom-left.
    // SDL_RenderFillRect(render, &(SDL_Rect){screen_bl.x, screen_bl.y - height, width, height});

    SDL_RenderCopy(render, player->texture, NULL, &(SDL_Rect){screen_bl.x, screen_bl.y - height, width, height});
}

bool player_intersect_circle(struct player_o* player, circle_t other)
{
    assert(player);
    return circle_intersect((circle_t){player->pos, player->bounding_circle_radius}, other);
}
