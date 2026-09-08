#include "command.h"

#include <SDL3/SDL_assert.h>

#include "combat.h"
#include "world.h"

static bool
do_move(struct rl_actor* actor, struct rl_world* world, SDL_Point dst)
{
  SDL_assert(grid_contains(&world->level.map, dst.x, dst.y));

  if (rl_is_walkable(*grid_at(&world->level.map, dst.x, dst.y))) {
    actor->pos = dst;
    return true;
  }

  return false;
}

static bool
do_attack(struct rl_actor* actor,
          struct rl_world* world,
          int defender_id,
          alist(rl_event) * events,
          struct rand_state* rng)
{
  struct rl_actor* defender = rl_edit_actor(world, defender_id);
  if (defender == NULL) {
    return false;
  }

  int const damage = rl_attack_actor(actor, defender, rng);
  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = actor->id;
  event.as.attack.defender = defender->id;
  event.as.attack.damage = damage;
  *alist_push(events) = event;

  if (defender->hp <= 0) {
    event.type = RL_EVENT_DEATH;
    event.as.death.actor = defender->id;
    event.as.death.killer = actor->id;
    *alist_push(events) = event;
  }

  return true;
}

struct rl_command
rl_new_bump_command(int actor_id, SDL_Point dir, struct rl_world const* world)
{
  struct rl_command cmd = { 0 };
  cmd.actor = actor_id;
  cmd.type = RL_COMMAND_NONE;

  struct rl_actor const* actor = rl_get_actor(world, actor_id);
  if (actor == NULL || !rl_actor_is_alive(actor)) {
    return cmd;
  }

  SDL_Point dst = { 0 };
  dst.x = actor->pos.x + dir.x;
  dst.y = actor->pos.y + dir.y;

  // TODO: need to check other things?
  if (!grid_contains(&world->level.map, dst.x, dst.y)) {
    return cmd;
  }

  struct rl_actor const* target = rl_find_actor(world, dst);
  if (target != NULL) {
    cmd.type = RL_COMMAND_ATTACK;
    cmd.target = target->id;
  } else if (rl_is_walkable(*grid_at(&world->level.map, dst.x, dst.y))) {
    cmd.type = RL_COMMAND_MOVE;
    cmd.dst = dst;
  }

  return cmd;
}

bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* cmd,
                 alist(rl_event) * events,
                 struct rand_state* rng)
{
  if (cmd->type == RL_COMMAND_NONE) {
    return false;
  }

  struct rl_actor* actor = rl_edit_actor(world, cmd->actor);
  if (actor == NULL || !rl_actor_is_alive(actor)) {
    return false;
  }

  bool consume_turn = false;
  switch (cmd->type) {
    case RL_COMMAND_MOVE:
      consume_turn = do_move(actor, world, cmd->dst);
      break;
    case RL_COMMAND_ATTACK:
      consume_turn =
        do_attack(actor, world, cmd->target, events, rng);
      break;
    default:
      break;
  }

  return consume_turn;
}
