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
struct gfx_camera;
struct gfx_grid_view;
struct rl_world;

/**
 * Buffers for drawing the current level through a camera.
 */
struct rl_world_renderer
{
  /** The world region covered by the buffers, in cells. */
  SDL_Rect bounds;
  /** The buffer's world region intersected with the level bounds. */
  SDL_Rect visible;
  /** The level's terrain over bounds. */
  grid(gfx_console) terrain;
  /** The player's light over bounds. */
  grid(gfx_console) light;
};

/**
 * Allocates buffers of columns x rows cells.
 *
 * @return false if allocation fails
 */
bool
rl_init_world_renderer(struct rl_world_renderer* wr, int columns, int rows);

/**
 * Frees the buffers.
 */
void
rl_free_world_renderer(struct rl_world_renderer* wr);

/**
 * Fills the buffers from the current level for the cells the camera sees.
 */
void
rl_prepare_world_renderer(struct rl_world_renderer* wr,
                          struct rl_world const* world,
                          struct gfx_camera const* camera,
                          struct gfx_grid_view const* view);

/**
 * Draws the prepared terrain, the visible items and actors, then the light.
 *
 * @param font its tile size must equal the cell size of view
 */
void
rl_draw_world(struct rl_world_renderer const* wr,
              struct rl_world const* world,
              struct gfx_camera const* camera,
              struct gfx_grid_view const* view,
              SDL_Renderer* renderer,
              struct gfx_tileset const* font);

/**
 * Highlights the cells of bounds that are set in mask.
 *
 * @param mask one entry per cell of bounds
 */
void
rl_draw_world_target_area(struct rl_world_renderer const* wr,
                          struct gfx_camera const* camera,
                          struct gfx_grid_view const* view,
                          SDL_Renderer* renderer,
                          SDL_Rect const* bounds,
                          grid(boolean) const* mask);

/**
 * Outlines the cell at cursor.
 */
void
rl_draw_world_cursor(struct gfx_camera const* camera,
                     struct gfx_grid_view const* view,
                     SDL_Renderer* renderer,
                     SDL_Point cursor);

#endif // GINC_ROGUELIKE_RENDER_WORLD_H
