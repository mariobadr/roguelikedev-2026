#include "command.h"

#include <SDL3/SDL_assert.h>

#include "mechanics.h"
#include "world.h"

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
  struct rl_level const* level = rl_get_current_level(world);
  if (!grid_contains(&level->map, dst.x, dst.y)) {
    return cmd;
  }

  struct rl_actor const* target = rl_find_actor(world, dst);
  if (target != NULL) {
    cmd.type = RL_COMMAND_ATTACK;
    cmd.target = target->id;
  } else if (rl_is_walkable(*grid_at(&level->map, dst.x, dst.y))) {
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
  if (actor == NULL) {
    return false;
  }

  switch (cmd->type) {
    case RL_COMMAND_MOVE:
      return rl_move(world, actor->id, cmd->dst);
    case RL_COMMAND_ATTACK:
      return rl_attack_melee(world, actor->id, cmd->target, events, rng);
    case RL_COMMAND_PICK_UP:
      return rl_pick_up_item(world, actor->id, cmd->dst, events);
    case RL_COMMAND_USE_ITEM:
      return rl_use_item(world, actor->id, cmd->target, events, rng);
    case RL_COMMAND_WAIT:
      return true;
    default:
      break;
  }

  return false;
}
