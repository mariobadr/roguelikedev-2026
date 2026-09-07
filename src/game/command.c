#include "command.h"

#include <SDL3/SDL_assert.h>

#include "combat.h"
#include "world.h"

static bool
try_attack(struct rl_actor* attacker,
           struct rl_world* world,
           SDL_Point dst,
           alist(rl_event) * events,
           struct rand_state* rng)
{
  struct rl_actor const* target = rl_find_actor(world, dst);
  if (target == NULL) {
    return false;
  }

  struct rl_actor* defender = rl_edit_actor(world, target->id);
  int const damage = rl_attack_actor(attacker, defender, rng);

  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->id;
  event.as.attack.defender = defender->id;
  event.as.attack.damage = damage;
  *alist_push(events) = event;

  if (defender->hp <= 0) {
    event.type = RL_EVENT_DEATH;
    event.as.death.actor = defender->id;
    event.as.death.killer = attacker->id;
    *alist_push(events) = event;
  }

  return true;
}

static bool
try_move(struct rl_actor* actor, struct rl_world const* world, SDL_Point dst)
{
  SDL_assert(grid_contains(&world->level.map, dst.x, dst.y));

  if (rl_is_walkable(*grid_at(&world->level.map, dst.x, dst.y))) {
    actor->pos = dst;
    return true;
  }

  return false;
}

static bool
do_move(struct rl_actor* actor,
        struct rl_world* world,
        SDL_Point direction,
        alist(rl_event) * events,
        struct rand_state* rng)
{
  SDL_Point dst = { 0 };
  dst.x = actor->pos.x + direction.x;
  dst.y = actor->pos.y + direction.y;

  if (try_attack(actor, world, dst, events, rng)) {
    return true;
  }

  if (try_move(actor, world, dst)) {
    return true;
  }

  return false;
}

static struct rl_command
create_move_command(int dx, int dy)
{
  struct rl_command cmd = { 0 };

  cmd.type = RL_COMMAND_MOVE;
  cmd.direction.x = dx;
  cmd.direction.y = dy;

  return cmd;
}

struct rl_command
rl_build_command(int actor_id, enum rl_action action)
{
  struct rl_command cmd = { 0 };

  switch (action) {
    case RL_ACTION_MOVE_UP:
      cmd = create_move_command(0, -1);
      break;
    case RL_ACTION_MOVE_DOWN:
      cmd = create_move_command(0, 1);
      break;
    case RL_ACTION_MOVE_LEFT:
      cmd = create_move_command(-1, 0);
      break;
    case RL_ACTION_MOVE_RIGHT:
      cmd = create_move_command(1, 0);
      break;
    case RL_ACTION_INTERACT:
      break; // TODO
    default:
      return cmd;
  }

  cmd.actor = actor_id;
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
      consume_turn = do_move(actor, world, cmd->direction, events, rng);
      break;
    default:
      break;
  }

  return consume_turn;
}
