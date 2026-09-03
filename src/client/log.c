#include "log.h"

#include "game/event.h"
#include "game/world.h"

#include "client/graphics.h"
#include "client/palette.h"

static void
append(struct rl_log_line* line, char const* text, SDL_FColor fg)
{
  for (int i = 0; text[i] != '\0' && line->len < 64; i++) {
    line->msg[line->len++] = (struct rl_log_char){ text[i], fg };
  }
}

struct rl_log_line
rl_build_attack_log(struct rl_world const* world,
                    struct rl_event_attack const* event)
{
  struct rl_entity const* attacker = rl_get_entity(world, event->attacker);
  struct rl_entity const* defender = rl_get_entity(world, event->defender);

  // build up the log line piece by piece
  struct rl_log_line line = { 0 };

  // <attacker> <hits or misses> <defender>
  append(&line, attacker->name, rl_get_entity_gfx(attacker).fg);
  append(&line, event->damage < 0 ? " misses " : " hits ", RL_COLOUR_GRAY[5]);
  append(&line, defender->name, rl_get_entity_gfx(defender).fg);

  if (event->damage >= 0) {
    // if we didn't miss, append how much damage was done
    char buf[16];
    SDL_snprintf(buf, sizeof buf, " for %d", event->damage);
    append(&line, buf, RL_COLOUR_GRAY[5]);
  }

  append(&line, ".", RL_COLOUR_GRAY[5]);
  return line;
}

struct rl_log_line
rl_build_death_log(struct rl_world const* world,
                   struct rl_event_death const* event)
{
  struct rl_entity const* entity = rl_get_entity(world, event->entity);
  struct rl_entity const* killer = rl_get_entity(world, event->killer);

  // build up the log line piece by piece
  struct rl_log_line line = { 0 };

  // <killer> killed <entity>
  append(&line, killer->name, rl_get_entity_gfx(killer).fg);
  append(&line, " killed ", RL_COLOUR_GRAY[5]);
  append(&line, entity->name, rl_get_entity_gfx(entity).fg);

  append(&line, ".", RL_COLOUR_GRAY[5]);
  return line;
}

struct rl_log_line
rl_build_awaken_log(struct rl_world const* world,
                    struct rl_event_awaken const* event)
{
  struct rl_entity const* entity = rl_get_entity(world, event->entity);

  // build up the log line piece by piece
  struct rl_log_line line = { 0 };

  append(&line, "A ", RL_COLOUR_GRAY[5]);
  append(&line, entity->name, rl_get_entity_gfx(entity).fg);
  append(&line, " woke up!", RL_COLOUR_GRAY[5]);

  return line;
}
