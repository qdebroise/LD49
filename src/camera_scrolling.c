#include "camera_scrolling.h"

#include "camera.h"
#include "linalg.h"
#include "player.h"

#include <assert.h>
#include <stdlib.h>

struct camera_scrolling_system_o
{
    vec2_t dir;
};

struct camera_scrolling_system_o* camera_scrolling_system_create(void)
{
    struct camera_scrolling_system_o* scroll = malloc(sizeof(struct camera_scrolling_system_o));
    scroll->dir = (vec2_t){0, 0};
    return scroll;
}

void camera_scrolling_system_destroy(struct camera_scrolling_system_o* scroll)
{
    assert(scroll);
    free(scroll);
}

void camera_scrolling_system_update(
    struct camera_scrolling_system_o* scroll,
    struct camera_o* camera,
    struct player_o* player)
{
    assert(scroll && camera && player);

    static const float DISTANCE_PERCENT = 0.015f;

    vec2_t player_pos = player_position(player);
    vec2_t camera_pos = camera_position(camera);

    if (player_pos.x == camera_pos.x && player_pos.y == camera_pos.y) return;

    float speed = DISTANCE_PERCENT * vec2_dist(player_pos, camera_pos);

    scroll->dir = vec2_normalize(vec2_sub(player_pos, camera_pos));
    camera_pos = vec2_add(camera_pos, vec2_mul_scalar(scroll->dir, speed));
    camera_look_at(camera, camera_pos);
}
