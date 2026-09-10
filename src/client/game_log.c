#include "game_log.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "game/event.h"
#include "game/item.h"
#include "game/world.h"

#include "client/graphics.h"

static SDL_FColor
actor_colour(struct rl_actor const* actor)
{
  return rl_get_actor_gfx(actor).fg;
}

static struct rl_text
build_attack_log(struct rl_world const* world,
                 struct rl_event_attack const* event)
{
  struct rl_actor const* attacker = rl_get_actor(world, event->attacker);
  struct rl_actor const* defender = rl_get_actor(world, event->defender);
  SDL_FColor const attacker_colour = actor_colour(attacker);
  SDL_FColor const defender_colour = actor_colour(defender);

  struct rl_text msg = { 0 };

  // <attacker> <hits or misses> <defender>
  rl_append_text(&msg, &attacker_colour, attacker->name);
  rl_append_text(&msg, NULL, event->damage < 0 ? " misses " : " hits ");
  rl_append_text(&msg, &defender_colour, defender->name);

  if (event->damage >= 0) {
    // if we didn't miss, append how much damage was done
    rl_append_text_format(&msg, NULL, " for %d.", event->damage);
  } else {
    rl_append_text(&msg, NULL, ".");
  }

  return msg;
}

static struct rl_text
build_death_log(struct rl_world const* world,
                struct rl_event_death const* event)
{
  struct rl_actor const* actor = rl_get_actor(world, event->actor);
  struct rl_actor const* killer = rl_get_actor(world, event->killer);
  SDL_FColor const killer_colour = actor_colour(killer);
  SDL_FColor const actor_colour_ = actor_colour(actor);

  struct rl_text msg = { 0 };

  // <killer> killed <actor>
  rl_append_text(&msg, &killer_colour, killer->name);
  rl_append_text(&msg, NULL, " killed ");
  rl_append_text(&msg, &actor_colour_, actor->name);
  rl_append_text(&msg, NULL, ".");

  return msg;
}

static struct rl_text
build_awaken_log(struct rl_world const* world,
                 struct rl_event_awaken const* event)
{
  struct rl_actor const* actor = rl_get_actor(world, event->actor);
  SDL_FColor const colour = actor_colour(actor);

  struct rl_text msg = { 0 };

  rl_append_text(&msg, NULL, "A ");
  rl_append_text(&msg, &colour, actor->name);
  rl_append_text(&msg, NULL, " woke up!");

  return msg;
}

static struct rl_text
build_pickup_log(struct rl_world const* world,
                 struct rl_event_pickup const* event)
{
  struct rl_actor const* actor = rl_get_actor(world, event->actor);
  struct rl_item const* item = rl_get_item(world, event->item);
  struct rl_item_def const* idef = rl_get_item_def(item->itype);
  SDL_FColor const actor_colour_ = actor_colour(actor);
  SDL_FColor const item_colour = rl_get_item_gfx(item).fg;

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &actor_colour_, actor->name);
  rl_append_text(&msg, NULL, " picked up a ");
  rl_append_text(&msg, &item_colour, idef->name);

  return msg;
}

static struct rl_text
build_heal_log(struct rl_world const* world, struct rl_event_heal const* event)
{
  struct rl_actor const* actor = rl_get_actor(world, event->actor);
  SDL_FColor const colour = actor_colour(actor);

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &colour, actor->name);
  rl_append_text_format(
    &msg, NULL, " gained %d of %d HP.", event->effective, event->total);

  return msg;
}

bool
rl_init_game_log(struct rl_game_log* log)
{
  if (!alist_alloc(&log->messages, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_game_log(log);
    return false;
  }

  return true;
}

void
rl_free_game_log(struct rl_game_log* log)
{
  alist_free(&log->messages);
}

bool
rl_log_text(struct rl_game_log* log, struct rl_text const* message)
{
  struct rl_text* slot = alist_push(&log->messages);
  if (slot == NULL) {
    return false;
  }

  *slot = *message;
  return true;
}

void
rl_log_event(struct rl_game_log* log,
             struct rl_event const* event,
             struct rl_world const* world)
{
  struct rl_text msg;

  switch (event->type) {
    case RL_EVENT_ATTACK:
      msg = build_attack_log(world, &event->as.attack);
      break;
    case RL_EVENT_DEATH:
      msg = build_death_log(world, &event->as.death);
      break;
    case RL_EVENT_AWAKEN:
      msg = build_awaken_log(world, &event->as.awaken);
      break;
    case RL_EVENT_PICKUP:
      msg = build_pickup_log(world, &event->as.pickup);
      break;
    case RL_EVENT_HEAL:
      msg = build_heal_log(world, &event->as.heal);
      break;
    default:
      return;
  }

  rl_log_text(log, &msg);
}
