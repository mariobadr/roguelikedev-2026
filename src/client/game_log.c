#include "game_log.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "game/event.h"
#include "game/world.h"

static void
push_run(struct rl_text* message, char const* text, enum rl_text_style style)
{
  if (message->run_count >= SDL_arraysize(message->runs)) {
    return;
  }

  struct rl_text_run* run = &message->runs[message->run_count++];
  run->style = style;
  SDL_snprintf(run->text, sizeof run->text, "%s", text);
}

static enum rl_text_style
entity_style(struct rl_entity const* entity)
{
  if (entity->type == RL_ENTITY_ROGUE) {
    return RL_TEXT_PLAYER;
  }

  return RL_TEXT_ENEMY;
}

static struct rl_text
build_attack_log(struct rl_world const* world,
                 struct rl_event_attack const* event)
{
  struct rl_entity const* attacker = rl_get_entity(world, event->attacker);
  struct rl_entity const* defender = rl_get_entity(world, event->defender);

  // build up the message piece by piece
  struct rl_text msg = { 0 };

  // <attacker> <hits or misses> <defender>
  push_run(&msg, attacker->name, entity_style(attacker));
  push_run(&msg, event->damage < 0 ? " misses " : " hits ", RL_TEXT_NORMAL);
  push_run(&msg, defender->name, entity_style(defender));

  if (event->damage >= 0) {
    // if we didn't miss, append how much damage was done
    char damage[32];
    SDL_snprintf(damage, sizeof damage, " for %d", event->damage);
    push_run(&msg, damage, RL_TEXT_NORMAL);
  }

  push_run(&msg, ".", RL_TEXT_NORMAL);
  return msg;
}

static struct rl_text
build_death_log(struct rl_world const* world,
                struct rl_event_death const* event)
{
  struct rl_entity const* entity = rl_get_entity(world, event->entity);
  struct rl_entity const* killer = rl_get_entity(world, event->killer);

  // build up the message piece by piece
  struct rl_text msg = { 0 };

  // <killer> killed <entity>
  push_run(&msg, killer->name, entity_style(killer));
  push_run(&msg, " killed ", RL_TEXT_NORMAL);
  push_run(&msg, entity->name, entity_style(entity));

  push_run(&msg, ".", RL_TEXT_NORMAL);
  return msg;
}

static struct rl_text
build_awaken_log(struct rl_world const* world,
                 struct rl_event_awaken const* event)
{
  struct rl_entity const* entity = rl_get_entity(world, event->entity);

  // build up the message piece by piece
  struct rl_text msg = { 0 };

  push_run(&msg, "A ", RL_TEXT_NORMAL);
  push_run(&msg, entity->name, entity_style(entity));
  push_run(&msg, " woke up!", RL_TEXT_NORMAL);

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

void
rl_game_log_on_event(struct rl_game_log* log,
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
    default:
      return;
  }

  *alist_push(&log->messages) = msg;
}
