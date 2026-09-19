#include "experience.h"

#include <SDL3/SDL_assert.h>

int
rl_xp_required(int player_level)
{
  SDL_assert(player_level > 0);

  // So, at level 1, this is 400. Which, based on the xp rewards, is 400 / 50 =
  // 8 level 1 enemies to kill before reaching level 2.
  int const factor = 40 * player_level + 360;

  return player_level * factor;
}

int
rl_xp_reward(int player_level, int enemy_level)
{
  SDL_assert(player_level > 0);
  SDL_assert(enemy_level > 0);

  // So, at level 1, this is 50
  int const base = 45 + 5 * player_level;

  if (enemy_level < player_level) {
    int const gap = player_level - enemy_level;
    if (gap >= 5) {
      // the player is a much higher level than the enemy
      return 0;
    }

    // decrease base by some fraction
    return base * (5 - gap) / 5;
  }

  int const level_difference = enemy_level - player_level;
  int const bonus_levels = level_difference > 4 ? 4 : level_difference;

  // increase base by some amount
  return (base * (20 + bonus_levels) + 10) / 20;
}
