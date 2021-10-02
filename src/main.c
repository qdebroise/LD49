#include "array.h"
#include "atom.h"
#include "audio.h"
#include "camera.h"
#include "display.h"
#include "linalg.h"
#include "player.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static const uint32_t DISPLAY_WIDTH = 1280;
static const uint32_t DISPLAY_HEIGHT = 720;

int main(int argc, char* argv[])
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Error initializing the SDL: %s\n", SDL_GetError());
        return 1;
    }

    display_t display = display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_Renderer* render = display.render;

    struct audio_system_o* audio_system = audio_system_create();
    struct camera_o* camera = camera_create((vec2_t){0, 0}, (vec2_t){DISPLAY_WIDTH, DISPLAY_HEIGHT});
    struct player_o* player = player_create(render);
    struct atom_system_o* atom_system = atom_system_create(render);

    // @Todo: use SDL_GetPerformanceCounter() coupled with SDL_GetPerformanceFrequency().
    uint32_t last_time = SDL_GetTicks();
    uint32_t time_accumulator = 0;
    uint32_t update_frames = 0;
    uint32_t render_frames = 0;
    uint32_t timer_ms = SDL_GetTicks();

    // Run the update loop at a fixed timestep at about 60 UPS (Update Per Second).
    static const uint32_t UPDATE_STEP_MS = 1000 / 60;

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            camera_handle_event(camera, event);
            player_handle_event(player, camera, event);

            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_UP:
                        audio_play_sound(audio_system, AUDIO_ENTRY_LASER);
                        break;
                    case SDLK_DOWN:
                        audio_play_sound(audio_system, AUDIO_ENTRY_MUSIC);
                        break;
                    case SDLK_RIGHT:
                        audio_play_sound(audio_system, AUDIO_ENTRY_EXPLOSION);
                        break;
                    default: break;
                }
            }
        }

        // Logic
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
            camera_update(camera);
            player_update(player);
            atom_system_update(atom_system, player, UPDATE_STEP_MS);

            time_accumulator -= UPDATE_STEP_MS;
            update_frames += 1;
        }

        // Render
        SDL_SetRenderDrawColor(render, 104, 159, 56, 255);
        SDL_RenderClear(render);

        player_draw(player, camera, render);
        atom_system_draw(atom_system, camera, render);

        SDL_RenderPresent(render);
        render_frames += 1;
    }

    atom_system_destroy(atom_system);
    player_destroy(player);
    camera_destroy(camera);

    audio_system_destroy(audio_system);
    display_destroy(display);
    SDL_Quit();
    return 0;
}
