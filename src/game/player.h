/**
 * @file player.h
 */
#ifndef GINC_ROGUELIKE_PLAYER_H
#define GINC_ROGUELIKE_PLAYER_H

#include "game/handles.h"

struct rl_player
{
  /** The actor this player controls. */
  handle(rl_actor) actor;

  /** XP earned toward the actor's next level. */
  int xp;
};

#endif // GINC_ROGUELIKE_PLAYER_H
