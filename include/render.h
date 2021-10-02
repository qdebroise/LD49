#ifndef RENDER_H_
#define RENDER_H_

struct SDL_Texture;
struct SDL_Renderer;

struct SDL_Texture* load_bmp_to_texture(struct SDL_Renderer*, const char* file);

#endif // RENDER_H_


