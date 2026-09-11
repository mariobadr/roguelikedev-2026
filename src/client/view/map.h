/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_fov;
struct rl_view;
struct rl_world;

/**
 * Possible results when asking the map view to select a tile.
 */
enum rl_map_selection_result
{
  RL_MAP_SELECTION_NONE,
  RL_MAP_SELECTION_CONFIRMED,
  RL_MAP_SELECTION_CANCELLED,
};

/**
 * Allocate a map view.
 */
bool
rl_alloc_map_view(struct rl_view* view,
                  struct rl_world const* world,
                  struct rl_fov const* fov,
                  SDL_FRect const* viewport,
                  int cell_width,
                  int cell_height);

/**
 * Enter cursor-selection mode, with the cursor starting at origin.
 */
void
rl_map_view_begin_select(struct rl_view* view, SDL_Point origin);

/**
 * Consume the result of a selection, if one is pending.
 */
enum rl_map_selection_result
rl_map_view_take_selection(struct rl_view* view, SDL_Point* out);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
