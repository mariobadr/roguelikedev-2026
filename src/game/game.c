#include "game.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>

#include "actor.h"
#include "actor_def.h"
#include "ai.h"
#include "command.h"
#include "equipment.h"
#include "experience.h"
#include "generate.h"
#include "mechanics.h"
#include "pathfinding.h"

static void
award_kill_xp(struct rl_world* world,
              struct rl_event const* event,
              alist(rl_event)* events)
{
  if (event->type != RL_EVENT_DEATH) {
    // no kill xp to reward
    return;
  }

  handle(rl_actor) const rogue_handle = rl_get_rogue(world);
  if (!handle_equal(event->as.death.killer, rogue_handle)) {
    // the rogue did not make the kill
    return;
  }

  if (handle_equal(event->as.death.actor, rogue_handle)) {
    // the rogue killed itself
    return;
  }

  struct rl_actor const* rogue = rl_borrow_actor(world, rogue_handle);
  SDL_assert(rogue != NULL);

  if (!rl_actor_is_alive(rogue)) {
    // the rogue is dead
    return;
  }

  struct rl_actor const* victim = rl_borrow_actor(world, event->as.death.actor);
  SDL_assert(victim != NULL);

  rl_gain_xp(world, rl_xp_reward(rogue->level, victim->level), events);
}

static void
drop_death_loot(struct rl_world* world,
                struct rl_event const* event,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  if (event->type != RL_EVENT_DEATH) {
    // no loot to drop
    return;
  }

  handle(rl_actor) const victim = event->as.death.actor;
  rl_drop_loot(world, victim, events, rng);
}

static bool
slays_dragon(struct rl_world const* world, struct rl_event const* event)
{
  if (event->type != RL_EVENT_DEATH) {
    return false;
  }

  struct rl_actor const* victim = rl_borrow_actor(world, event->as.death.actor);
  SDL_assert(victim != NULL);

  return victim->type == RL_ACTOR_DRAGON;
}

static bool
resolve_command(struct rl_world* world,
                struct rl_command const* cmd,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  // only process the command's original events
  size_t const first_event = alist_len(events);
  bool const turn_taken = rl_apply_command(world, cmd, events, rng);
  size_t const end_event = alist_len(events);

  for (size_t i = first_event; i < end_event; i++) {
    award_kill_xp(world, alist_at(events, i), events);
    drop_death_loot(world, alist_at(events, i), events, rng);
  }

  return turn_taken;
}

static void
update_actors(struct rl_world* world,
              alist(rl_event)* events,
              struct rand_state* rng)
{
  handle(rl_actor) const rogue_handle = rl_get_rogue(world);
  struct rl_actor const* rogue = rl_borrow_actor(world, rogue_handle);
  if (!rl_actor_is_alive(rogue)) {
    // the player is dead
    return;
  }

  // build the distance map where the target is the player
  struct rl_level const* level = rl_get_current_level(world);
  if (!rl_build_dijkstra_map(
        &world->player.distances, &level->map, rogue->pos)) {
    return;
  }

  // wake up actors in the player's field-of-view and/or
  // move actors closer to the player
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    handle(rl_actor) const actor_handle = *alist_at(&level->actors, i);
    struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
    if (handle_equal(actor->handle, rogue_handle)) {
      // the player is not controlled by the AI
      continue;
    }

    if (!rl_actor_is_alive(actor)) {
      // actor is dead
      continue;
    }

    bool const was_awake = actor->awake;
    if (!rl_wake_actor(actor, &world->player.fov)) {
      // actor is asleep
      continue;
    }

    if (!was_awake) {
      struct rl_event event = { 0 };
      event.type = RL_EVENT_AWAKEN;
      event.as.awaken.actor = actor->handle;
      *alist_push(events) = event;
    }

    // choose and execute each actor's command before updating the next actor
    struct rl_command cmd = rl_next_ai_command(actor, world);
    resolve_command(world, &cmd, events, rng);

    // the rogue lives in world->actors, which may have been reallocated
    rogue = rl_borrow_actor(world, rogue_handle);
    if (!rl_actor_is_alive(rogue)) {
      // the player is dead
      return;
    }
  }
}

void
rl_free_game(struct rl_game* game)
{
  if (game == NULL) {
    return;
  }

  rl_free_world(&game->world);
  SDL_zerop(game);
}

static bool
init_starting_equipment(struct rl_world* world, handle(rl_actor) rogue_handle)
{
  handle(rl_item) const weapon_handle =
    rl_create_item(world, RL_ITEM_WEAPON_DAGGER, 1);
  handle(rl_item) const armour_handle =
    rl_create_item(world, RL_ITEM_ARMOUR_LEATHER, 1);

  struct rl_actor* rogue = rl_borrow_mut_actor(world, rogue_handle);
  struct rl_item* weapon = rl_borrow_mut_item(world, weapon_handle);
  struct rl_item* armour = rl_borrow_mut_item(world, armour_handle);
  if (weapon == NULL || armour == NULL) {
    return false;
  }

  weapon->ltype = RL_ITEM_LOCATION_HELD;
  weapon->on.actor = rogue_handle;
  armour->ltype = RL_ITEM_LOCATION_HELD;
  armour->on.actor = rogue_handle;

  rl_equip(rogue, weapon);
  rl_equip(rogue, armour);

  return true;
}

// Initialize a zeroed game; the caller owns cleanup on failure.
static bool
init_game(struct rl_game* game, int width, int height, Uint64 seed)
{
  if (!rl_alloc_world(&game->world)) {
    return false;
  }

  rand_seed(&game->rng, seed);
  game->turns = 0;

  // create the main character
  handle(rl_actor) rogue_handle =
    rl_create_actor(&game->world, RL_ACTOR_ROGUE, 1);
  struct rl_actor* rogue = rl_borrow_mut_actor(&game->world, rogue_handle);
  if (rogue == NULL) {
    // oh noes
    return false;
  }

  rogue->awake = true; // not really necessary
  if (!init_starting_equipment(&game->world, rogue_handle)) {
    return false;
  }

  game->world.player.actor = rogue_handle;

  // the rogue starts at the entry point of the first level
  if (!rl_push_level(&game->world, width, height, rogue_handle, &game->rng)) {
    return false;
  }
  game->world.current_level = (int)alist_len(&game->world.levels) - 1;

  return rl_create_player(&game->world);
}

bool
rl_new_game(struct rl_game* out, int width, int height, Uint64 seed)
{
  struct rl_game tmp = { 0 };
  if (!init_game(&tmp, width, height, seed)) {
    rl_free_game(&tmp);
    return false;
  }

  *out = tmp;
  return true;
}

bool
rl_update_game(struct rl_game* game,
               struct rl_command const* cmd,
               alist(rl_event)* events)
{
  size_t const first_event = alist_len(events);
  bool turn_taken = resolve_command(&game->world, cmd, events, &game->rng);

  for (size_t i = first_event; i < alist_len(events); i++) {
    if (slays_dragon(&game->world, alist_at(events, i))) {
      game->won = true;
    }
  }

  if (turn_taken) {
    game->turns++;

    rl_update_visibility(&game->world);

    // nothing acts after the final blow
    if (!game->won) {
      update_actors(&game->world, events, &game->rng);
    }
  }

  return turn_taken;
}
