#include "array.h"
#include "atom.h"
#include "audio.h"
#include "camera.h"
#include "camera_scrolling.h"
#include "display.h"
#include "linalg.h"
#include "player.h"
#include "render.h"
#include "world.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static const char* GAME_TITLE = "LD49 - Death?Box";
static const uint32_t DISPLAY_WIDTH = 1280;
static const uint32_t DISPLAY_HEIGHT = 720;

// @Todo: the way game states are managed is crap. :(
enum game_state
{
    GAME_STATE_TITLESCREEN,
    GAME_STATE_PLAYING,
    GAME_STATE_CREDITS,
    GAME_STATE_QUIT,
};

typedef struct game_t
{
    enum game_state state;
} game_t;

static inline SDL_Rect bbox2_to_sdl_rect(bbox2_t bbox)
{
    const vec2_t size = bbox2_size(bbox);
    return (SDL_Rect){
        .x = bbox.min.x,
        .y = bbox.min.y,
        .w = size.x,
        .h = size.y,
    };
}

static void start_credit_loop(struct display_o* display, game_t* game_ctx)
{
    assert(game_ctx->state == GAME_STATE_CREDITS);

    SDL_Renderer* render = display_get_renderer(display);
    SDL_Texture* credit_texture = load_bmp_to_texture(render, "assets/images/credits.bmp");
    SDL_Texture* back_texture = load_bmp_to_texture(render, "assets/images/back.bmp");

    vec2_t center = {DISPLAY_WIDTH / 2.0f, DISPLAY_HEIGHT / 2.0f};
    vec2_t button_size = {400, 100};

    float vertical_offset = 250;

    bbox2_t back_bbox = {
        {center.x - button_size.x/2, center.y - button_size.y/2 + vertical_offset},
        {center.x + button_size.x/2, center.y + button_size.y/2 + vertical_offset}};

    SDL_Rect credits_rect = {
        DISPLAY_WIDTH / 2 - 200,
        100,
        640,
        360,
    };

    bool running = true;
    while (running)
    {
        //
        // Events
        //

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                game_ctx->state = GAME_STATE_QUIT;
                running = false;
                break;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        game_ctx->state = GAME_STATE_QUIT;
                        running = false;
                        break;
                    default: break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                vec2_t mouse_pos = {event.button.x, event.button.y};

                if (bbox2_contain(back_bbox, mouse_pos))
                {
                    game_ctx->state = GAME_STATE_TITLESCREEN;
                    return;
                }
            }
        }

        //
        // Render
        //

        SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
        SDL_RenderClear(render);
        SDL_Rect back_rect = bbox2_to_sdl_rect(back_bbox);
        SDL_RenderCopy(render, back_texture, NULL, &back_rect);
        SDL_RenderCopy(render, credit_texture, NULL, &credits_rect);
        SDL_RenderPresent(render);

        SDL_Delay(100); // Static menu so we can delay quit a lot.
    }

    SDL_DestroyTexture(credit_texture);
    SDL_DestroyTexture(back_texture);
}

static void start_titlescreen_loop(struct display_o* display, game_t* game_ctx)
{
    assert(game_ctx->state == GAME_STATE_TITLESCREEN);

    SDL_Renderer* render = display_get_renderer(display);
    SDL_Texture* play_texture = load_bmp_to_texture(render, "assets/images/play_button.bmp");
    SDL_Texture* quit_texture = load_bmp_to_texture(render, "assets/images/quit_button.bmp");
    SDL_Texture* to_credits_texture = load_bmp_to_texture(render, "assets/images/to_credits.bmp");
    SDL_Texture* title_texture = load_bmp_to_texture(render, "assets/images/title.bmp");

    vec2_t center = {DISPLAY_WIDTH / 2.0f, DISPLAY_HEIGHT / 2.0f};
    vec2_t button_size = {400, 100};

    float play_vertical_offset = 100;
    float quit_vertical_offset = 250;

    bbox2_t play_bbox = {
        {center.x - button_size.x/2, center.y - button_size.y/2 + play_vertical_offset},
        {center.x + button_size.x/2, center.y + button_size.y/2 + play_vertical_offset}};
    bbox2_t quit_bbox = {
        {center.x - button_size.x/2, center.y - button_size.y/2 + quit_vertical_offset},
        {center.x + button_size.x/2, center.y + button_size.y/2 + quit_vertical_offset}};

    vec2_t credit_size = {50, 50};
    vec2_t credit_center = {DISPLAY_WIDTH - credit_size.x / 2 - 20, credit_size.y / 2 + 20};
    bbox2_t credit_bbox = {
        {credit_center.x - credit_size.x/2, credit_center.y - credit_size.y/2},
        {credit_center.x + credit_size.x/2, credit_center.y + credit_size.y/2}};

    SDL_Rect title_rect = {
        DISPLAY_WIDTH / 2 - 200,
        100,
        400,
        200,
    };

    while (1)
    {
        //
        // Events
        //

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                game_ctx->state = GAME_STATE_QUIT;
                return;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        game_ctx->state = GAME_STATE_QUIT;
                        return;
                    default: break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                vec2_t mouse_pos = {event.button.x, event.button.y};

                if (bbox2_contain(play_bbox, mouse_pos))
                {
                    game_ctx->state = GAME_STATE_PLAYING;
                    return;
                }
                else if (bbox2_contain(quit_bbox, mouse_pos))
                {
                    game_ctx->state = GAME_STATE_QUIT;
                    return;
                }
                else if (bbox2_contain(credit_bbox, mouse_pos))
                {
                    game_ctx->state = GAME_STATE_CREDITS;
                    return;
                }
            }
        }

        //
        // Render
        //

        SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
        SDL_RenderClear(render);

        SDL_Rect play_rect = bbox2_to_sdl_rect(play_bbox);
        SDL_Rect quit_rect = bbox2_to_sdl_rect(quit_bbox);
        SDL_Rect credit_rect = bbox2_to_sdl_rect(credit_bbox);

        SDL_RenderCopy(render, play_texture, NULL, &play_rect);
        SDL_RenderCopy(render, quit_texture, NULL, &quit_rect);
        SDL_RenderCopy(render, to_credits_texture, NULL, &credit_rect);
        SDL_RenderCopy(render, title_texture, NULL, &title_rect);

        SDL_RenderPresent(render);

        SDL_Delay(100); // Static menu so we can delay quit a lot.
    }

    SDL_DestroyTexture(play_texture);
    SDL_DestroyTexture(quit_texture);
    SDL_DestroyTexture(to_credits_texture);
    SDL_DestroyTexture(title_texture);
}

static void start_game_loop(
    struct display_o* display,
    struct audio_system_o* audio_system,
    game_t* game_ctx)
{
    assert(game_ctx->state == GAME_STATE_PLAYING);

    // Run the update loop at a fixed timestep at about 60 UPS (Update Per Second).
    static const uint32_t UPDATE_STEP_MS = 1000 / 60;

    SDL_Renderer* render = display_get_renderer(display);
    SDL_Texture* background = load_bmp_to_texture(render, "assets/images/background.bmp");

    struct camera_o* camera = camera_create((vec2_t){0, 0}, (vec2_t){DISPLAY_WIDTH, DISPLAY_HEIGHT});
    struct player_o* player = player_create(render);
    struct atom_system_o* atom_system = atom_system_create(render);
    struct camera_scrolling_system_o* scroll = camera_scrolling_system_create();

    world_t world = {
        .bounds = {
            .north = 600,
            .south = -600,
            .east = 800,
            .west = -800,
        },
    };

    atom_system_generate_atoms(atom_system, player, world, 5);

    // @Todo: use SDL_GetPerformanceCounter() coupled with SDL_GetPerformanceFrequency().
    uint32_t last_time = SDL_GetTicks();
    uint32_t time_accumulator = 0;
    uint32_t update_frames = 0;
    uint32_t render_frames = 0;
    uint32_t timer_ms = SDL_GetTicks();

    uint32_t win_count = 0;

    bool running = true;
    while (running)
    {
        //
        // Events
        //

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            camera_handle_event(camera, event);
            player_handle_event(player, camera, event);

            if (event.type == SDL_QUIT)
            {
                game_ctx->state = GAME_STATE_QUIT;
                running = false;
                break;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        game_ctx->state = GAME_STATE_QUIT;
                        running = false;
                        break;
                    default: break;
                }
            }
        }

        //
        // Logic
        //

        const uint32_t now = SDL_GetTicks();
        const uint32_t elapsed = now - last_time;
        const uint32_t timer_elapsed = now - timer_ms;

        if (timer_elapsed >= 1000)
        {
#ifndef DNDEBUG
            char title[256];
            sprintf(title, "Render: %d FPS (%.3f ms/frame) - Update: %d UPS (%.3f ms/update)\n",
                render_frames, 1000.0f / render_frames, update_frames, 1000.0f / update_frames);
            display_set_title(display, &title[0]);
#endif

            timer_ms = now + 1000 - timer_elapsed;
            render_frames = 0;
            update_frames = 0;
        }

        // If elapsed is 0 it means the frametime was below the timer precision. In this case we
        // don't update `last_time`, skip the logic part and only perform the rendering.
        if (elapsed != 0)
        {
            last_time = now;
        }

        time_accumulator += elapsed;

        while (time_accumulator >= UPDATE_STEP_MS)
        {
            if (atom_system_all_stable(atom_system))
            {
                atom_system_generate_atoms(atom_system, player, world, 2*win_count + 5);
                win_count++;
            }

            if (win_count == 3)
            {
                printf("Win!\n");
                game_ctx->state = GAME_STATE_CREDITS;
                running = false;
            }

            camera_update(camera);
            player_update(player, world, UPDATE_STEP_MS);
            camera_scrolling_system_update(scroll, camera, player);
            atom_system_update(atom_system, audio_system, player, world, UPDATE_STEP_MS);

            if (player_is_dead(player))
            {
                printf("Dead!\n");
                game_ctx->state = GAME_STATE_TITLESCREEN;
                running = false;
            }

            time_accumulator -= UPDATE_STEP_MS;
            update_frames += 1;
        }

        //
        // Render
        //

        SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
        SDL_RenderClear(render);

        SDL_Rect background_rect = sdl_rect_from_pos_and_size(camera, (vec2_t){0, 0}, (vec2_t){world.bounds.east, world.bounds.north});
        SDL_RenderCopy(render, background, NULL, NULL);
        SDL_SetRenderDrawColor(render, 81, 64, 32, 255);
        SDL_RenderDrawRect(render, &background_rect);

        player_draw(player, camera, render);
        atom_system_draw(atom_system, camera, render);

        SDL_RenderPresent(render);
        render_frames += 1;
    }

    // Cleanup

    SDL_DestroyTexture(background);

    atom_system_destroy(atom_system);
    player_destroy(player);
    camera_destroy(camera);
}

int main(int argc, char* argv[])
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Error initializing the SDL: %s\n", SDL_GetError());
        return 1;
    }

    struct display_o* display = display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT, GAME_TITLE);
    struct audio_system_o* audio_system = audio_system_create();

    // Init + main loop
    game_t game_ctx = {
        .state = GAME_STATE_TITLESCREEN,
    };

    // @Todo: textures shouldn't be reloaded every time but store somewhere else. Kind of like the
    // audio stuff. But anyway, the game is small so nothing noticeable here :)
    while (game_ctx.state != GAME_STATE_QUIT)
    {
        start_titlescreen_loop(display, &game_ctx);

        if (game_ctx.state == GAME_STATE_PLAYING)
        {
            start_game_loop(display, audio_system, &game_ctx);
        }

        if (game_ctx.state == GAME_STATE_CREDITS)
        {
            start_credit_loop(display, &game_ctx);
        }
    }

    audio_system_destroy(audio_system);
    display_destroy(display);
    SDL_Quit();
    return 0;
}
