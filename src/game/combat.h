/**
 * @file combat.h
 */
#ifndef GINC_ROGUELIKE_COMBAT_H
#define GINC_ROGUELIKE_COMBAT_H

// forward declarations
struct rand_state;
struct rl_actor;

/**
 * @return how much damage was done (or -1 for a miss).
 */
int
rl_attack_actor(struct rl_actor const* attacker,
                struct rl_actor* defender,
                struct rand_state* rng);

#endif // GINC_ROGUELIKE_COMBAT_H
