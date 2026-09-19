/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_VIEW_H
#define GINC_ROGUELIKE_WORLD_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_command;
struct rl_item_def;
struct rl_view;
struct rl_world;

/**
 * Possible results when asking the world view to select a tile.
 */
enum rl_world_selection_result
{
  RL_WORLD_SELECTION_NONE,
  RL_WORLD_SELECTION_CONFIRMED,
  RL_WORLD_SELECTION_CANCELLED,
};

/**
 * Allocate a world view.
 */
bool
rl_alloc_world_view(struct rl_view* view,
                  struct rl_world const* world,
                  SDL_FRect const* viewport,
                  int cell_width,
                  int cell_height);

/**
 * Consume a pending command.
 */
bool
rl_world_view_take_command(struct rl_view* view, struct rl_command* out);

/**
 * Enter cursor-selection mode for the tile-targeted item def, with the cursor
 * starting at origin.
 *
 * @return whether selection mode was entered.
 */
bool
rl_world_view_begin_select(struct rl_view* view,
                         SDL_Point origin,
                         struct rl_item_def const* def);

/**
 * Consume the result of a selection, if one is pending.
 */
enum rl_world_selection_result
rl_world_view_take_selection(struct rl_view* view, SDL_Point* out);

#endif // GINC_ROGUELIKE_WORLD_VIEW_H
