#include "camera.h"
#include "linalg.h"

#include <assert.h>
#include <stdlib.h>

typedef struct camera_o camera_o;

struct camera_o
{
    vec2_t pos;
    vec2_t viewport;
    mat3_t inv_view;
    mat3_t view;
};

camera_o* camera_create(vec2_t pos, vec2_t viewport)
{
    // @Note @Todo: see later about custom allocators.
    camera_o* camera = malloc(sizeof(struct camera_o));
    camera->pos = pos;
    camera->viewport = viewport;
    camera->inv_view = mat3_translation(pos.x, pos.y);
    camera->view = mat3_inverse(camera->inv_view);
    return camera;
}

void camera_destroy(struct camera_o* camera)
{
    assert(camera);
    free(camera);
}

void camera_handle_event(struct camera_o* camera, SDL_Event event)
{
    assert(camera);
}

void camera_update(struct camera_o* camera)
{
    assert(camera);
}

vec2_t camera_screen_to_world(struct camera_o* camera, vec2_t screen)
{
    assert(camera);

    vec3_t ndc = {
        screen.x / (camera->viewport.x / 2) - 1,
        1 - (screen.y / (camera->viewport.y / 2)),
        1,
    };
    vec3_t world = {
        .x = ndc.x * (camera->viewport.x / 2),
        .y = ndc.y * (camera->viewport.y / 2),
        .z = 1,
    };
    world = mat3_mul_vec(camera->inv_view, world);
    return (vec2_t){world.x, world.y};
}

vec2_t camera_world_to_screen(struct camera_o* camera, vec2_t world)
{
    assert(camera);

    vec3_t world_cc = mat3_mul_vec(camera->view, (vec3_t){world.x, world.y, 1});
    vec2_t ndc = {
        world_cc.x / camera->viewport.x + 0.5,
        1 - (world_cc.y / camera->viewport.y + 0.5),
    };
    vec2_t screen = {ndc.x * camera->viewport.x, ndc.y * camera->viewport.y};
    return screen;
}

mat3_t camera_view(struct camera_o* camera)
{
    assert(camera);
    return camera->view;
}

vec2_t camera_pos(struct camera_o* camera)
{
    return camera->pos;
}
