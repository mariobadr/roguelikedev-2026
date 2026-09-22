#include "command.h"

#include <SDL3/SDL_assert.h>

#include "combat.h"
#include "mechanics.h"
#include "world.h"

struct rl_command
rl_new_bump_command(struct rl_actor const* actor,
                    SDL_Point dir,
                    struct rl_world const* world)
{
  struct rl_command cmd = { 0 };
  cmd.type = RL_COMMAND_NONE;

  cmd.actor = actor->handle;
  if (!rl_actor_is_alive(actor)) {
    return cmd;
  }

  SDL_Point dst = { 0 };
  dst.x = actor->pos.x + dir.x;
  dst.y = actor->pos.y + dir.y;

  // TODO: need to check other things?
  struct rl_level const* level = rl_get_current_level(world);
  handle(rl_actor) const target = rl_find_actor(world, level, dst);
  if (handle_is_nonnull(target)) {
    cmd.type = RL_COMMAND_ATTACK;
    cmd.target_actor = target;
  } else if (rl_is_walkable(*grid_at(&level->map, dst.x, dst.y))) {
    cmd.type = RL_COMMAND_MOVE;
    cmd.dst = dst;
  }

  return cmd;
}

bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* cmd,
                 alist(rl_event)* events,
                 struct rand_state* rng)
{
  if (cmd->type == RL_COMMAND_NONE) {
    return false;
  }

  struct rl_actor const* actor = rl_borrow_actor(world, cmd->actor);
  if (actor == NULL || !rl_actor_is_alive(actor)) {
    return false;
  }

  switch (cmd->type) {
    case RL_COMMAND_MOVE:
      return rl_move(world, cmd->actor, cmd->dst);
    case RL_COMMAND_ATTACK:
      return rl_attack_melee(world, cmd->actor, cmd->target_actor, events, rng);
    case RL_COMMAND_PICK_UP:
      return rl_pick_up_item(world, cmd->actor, cmd->dst, events);
    case RL_COMMAND_TAKE_STAIRS:
      return rl_take_stairs(world, cmd->actor, events, rng);
    case RL_COMMAND_USE_ITEM:
      return rl_use_item(
        world, cmd->actor, cmd->use_item.item, cmd->use_item.dst, events, rng);
    case RL_COMMAND_WAIT:
      return true;
    case RL_COMMAND_NONE:
      break;
  }

  return false;
}
