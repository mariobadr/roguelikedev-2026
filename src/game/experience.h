/**
 * @file experience.h
 */
#ifndef GINC_ROGUELIKE_EXPERIENCE_H
#define GINC_ROGUELIKE_EXPERIENCE_H

/**
 * @return the total experiene points needed to get to the next level.
 */
int
rl_xp_required(int player_level);

/**
 * @return the experience points to reward.
 */
int
rl_xp_reward(int player_level, int enemy_level);

#endif // GINC_ROGUELIKE_EXPERIENCE_H
