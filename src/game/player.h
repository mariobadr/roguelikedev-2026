/**
 * @file player.h
 */
#ifndef GINC_ROGUELIKE_PLAYER_H
#define GINC_ROGUELIKE_PLAYER_H

#include "container/grid.h"

#include "game/fov.h"
#include "game/handles.h"

/**
 * The player's progress and view of the level.
 */
struct rl_player
{
  /** The actor this player controls. */
  handle(rl_actor) actor;
  /** XP earned toward the actor's next level. */
  int xp;
  /** A map of distances to reach the player. */
  grid(int) distances;
  /** Player's field-of-view. */
  struct rl_fov fov;
};

/**
 * Allocate a player for a map of width by height cells.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_player(struct rl_player* player, int width, int height);

/**
 * Free the player.
 */
void
rl_free_player(struct rl_player* player);

#endif // GINC_ROGUELIKE_PLAYER_H
