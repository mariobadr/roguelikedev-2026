/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_fov;
struct rl_ribbon;
struct rl_world;

/**
 * A view of the playable map.
 */
struct rl_map_view
{
  /** Where this view exists on the screen. */
  SDL_FRect viewport;
  /** The width of a cell in the map. */
  int cell_width;
  /** The height of a cell in the map. */
  int cell_height;
};

/**
 * Initialize map.
 */
void
rl_init_map_view(struct rl_map_view* map,
                 SDL_FRect const* viewport,
                 int cell_width,
                 int cell_height);

/**
 * Get the width and height of map. Width and/or height can be NULL.
 */
void
rl_map_view_size(struct rl_map_view const* map, int* width, int* height);

/**
 * Find the cell that corresponds to the screen coordinates in pos.
 *
 * @return whether pos was actually in the map's viewport.
 */
bool
rl_map_view_cell_at(struct rl_map_view const* map,
                    SDL_FPoint pos,
                    SDL_Point* cell);

/**
 * Update ribbon based on the map view's current state.
 */
void
rl_map_view_ribbon(struct rl_map_view const* view, struct rl_ribbon* ribbon);

/**
 * Draw the map at its viewport.
 */
void
rl_draw_map_view(struct rl_map_view const* map,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_world const* world,
                 struct rl_fov const* fov);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
