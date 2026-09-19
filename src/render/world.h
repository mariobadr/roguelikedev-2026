/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_RENDER_WORLD_H
#define GINC_ROGUELIKE_RENDER_WORLD_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

#include "graphics/console.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;
struct rl_camera;
struct rl_world;

struct rl_world_renderer
{
  /** The buffer's world region intersected with the level bounds. */
  SDL_Rect visible;
  grid(gfx_console) terrain;
  grid(gfx_console) light;
};

bool
rl_init_world_renderer(struct rl_world_renderer* wr,
                     int columns,
                     int rows);

void
rl_free_world_renderer(struct rl_world_renderer* wr);

void
rl_prepare_world_renderer(struct rl_world_renderer* wr,
                        struct rl_world const* world,
                        struct rl_camera const* camera);

/** Draw with the camera used to prepare the buffers and a matching cell size. */
void
rl_draw_world(struct rl_world_renderer const* wr,
            struct rl_world const* world,
            struct rl_camera const* camera,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font);

void
rl_draw_world_target_area(struct rl_world_renderer const* wr,
                        struct rl_camera const* camera,
                        SDL_Renderer* renderer,
                        SDL_Rect const* bounds,
                        grid(boolean) const* mask);

void
rl_draw_world_cursor(struct rl_camera const* camera,
                   SDL_Renderer* renderer,
                   SDL_Point cursor);

#endif // GINC_ROGUELIKE_RENDER_WORLD_H
