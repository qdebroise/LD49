#include "display.h"
#include "linalg.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

static const uint32_t WINDOW_WIDTH = 1280;
static const uint32_t WINDOW_HEIGHT = 720;

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error initializing the SDL: %s\n", SDL_GetError());
        return 1;
    }

    display_t display = display_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_Renderer* r = display.render;

    // Model defined around it origin point, (0, 0) generally.
    // vec3_t bl = {-50, -50, 1};
    // vec3_t tr = {50, 50, 1};

    uint32_t last_time = SDL_GetTicks();
    uint32_t time_accumulator = 0;
    uint32_t update_frames = 0;
    uint32_t render_frames = 0;
    uint32_t frametime_sum_ms = 0;
    uint32_t timer_ms = SDL_GetTicks();

    // Run the update loop at a fixed timestep at about 60 UPS (Update Per Second).
    const uint32_t UPDATE_STEP_MS = 1000 / 60;

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
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
                    default: break;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
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
            sprintf(title, "Render: %d FPS (%3.f ms/frame) - Update: %d UPS (%.3f ms/update)\n",
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
            // View matrix computation.
            // mat3_t scaling = mat3_scaling(fabs(cos(angle)) + 0.5);
            // mat3_t translation = mat3_translation(cam.x, cam.y);
            // mat3_t inv_view = mat3_mul(translation, scaling);
            // mat3_t view = mat3_inverse(inv_view);

            time_accumulator -= UPDATE_STEP_MS;
            update_frames += 1;
        }

        // Transformation pipeline.
        // vec2_t viewport = {1280, 720};
        // vec2_t ndc = {center.x / viewport.x + 0.5, 1 - (center.y / viewport.y + 0.5)};
        // vec2_t screen = {ndc.x * viewport.x, ndc.y * viewport.y};

        // Render
        SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
        SDL_RenderClear(r);

        // SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
        // SDL_RenderFillRect(r, &(SDL_Rect){screen.x - width / 2, screen.y - height / 2, width, height});

        SDL_RenderPresent(r);
        render_frames += 1;
    }

    display_destroy(display);
    SDL_Quit();
    return 0;
}
