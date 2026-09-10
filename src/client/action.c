#include "action.h"

#include <SDL3/SDL_assert.h>

#include "game/actor.h"
#include "game/world.h"

static struct rl_command
build_debug_use_item(int actor_id, struct rl_world const* world)
{
  struct rl_command cmd = { 0 };
  cmd.actor = actor_id;

  for (int id = 0; id < alist_len(&world->items); id++) {
    struct rl_item const* item = rl_get_item(world, id);

    if (item->ltype == RL_ITEM_LOCATION_HELD && item->on.actor == actor_id) {
      cmd.type = RL_COMMAND_USE_ITEM;
      cmd.target = id;
      break;
    }
  }

  return cmd;
}

static struct rl_command
build_pickup(int actor_id, struct rl_world const* world)
{
  struct rl_command cmd = { 0 };

  struct rl_actor const* actor = rl_get_actor(world, actor_id);
  if (actor == NULL) {
    return cmd;
  }

  cmd.type = RL_COMMAND_PICK_UP;
  cmd.actor = actor_id;
  cmd.dst = actor->pos;

  return cmd;
}

struct rl_command
rl_build_command(int actor_id,
                 enum rl_action action,
                 struct rl_world const* world)
{
  struct rl_command cmd = { 0 };

  switch (action) {
    case RL_ACTION_MOVE_UP:
      return rl_new_bump_command(actor_id, (SDL_Point){ 0, -1 }, world);
    case RL_ACTION_MOVE_DOWN:
      return rl_new_bump_command(actor_id, (SDL_Point){ 0, 1 }, world);
    case RL_ACTION_MOVE_LEFT:
      return rl_new_bump_command(actor_id, (SDL_Point){ -1, 0 }, world);
    case RL_ACTION_MOVE_RIGHT:
      return rl_new_bump_command(actor_id, (SDL_Point){ 1, 0 }, world);
    case RL_ACTION_SELECT:
      return build_pickup(actor_id, world);
    default:
      break;
  }

  cmd.actor = actor_id;
  return cmd;
}
