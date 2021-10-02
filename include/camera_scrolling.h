#ifndef CAMERA_SCROLLING_H_
#define CAMERA_SCROLLING_H_

struct camera_scrolling_system_o;
struct camera_o;
struct player_o;

struct camera_scrolling_system_o* camera_scrolling_system_create(void);
void camera_scrolling_system_destroy(struct camera_scrolling_system_o*);

void camera_scrolling_system_update(
    struct camera_scrolling_system_o*,
    struct camera_o*,
    struct player_o*);

#endif // CAMERA_SCROLLING_H_


