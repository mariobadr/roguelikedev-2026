#include "game_log.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "game/event.h"
#include "game/item.h"
#include "game/world.h"

#include "render/graphics.h"
#include "render/palette.h"

static SDL_FColor
actor_colour(struct rl_actor const* actor)
{
  return rl_get_actor_gfx(actor).fg;
}

/**
 * How an attack of one kind is described.
 */
struct attack_phrasing
{
  /** The noun following the attacker's name, or NULL to name no instrument. */
  char const* instrument;
  /** The colour of the instrument. */
  SDL_FColor const* colour;
  /** The verb for a hit. */
  char const* hit;
  /** The verb for a critical hit. */
  char const* critical;
  /** The verb for a miss. */
  char const* miss;
};

static struct attack_phrasing const ATTACK_PHRASINGS[] = {
  [RL_ATTACK_MELEE] = {
    .instrument = NULL,
    .colour = NULL,
    .hit = " hits ",
    .critical = " critically hits ",
    .miss = " misses ",
  },
  [RL_ATTACK_FIRE] = {
    .instrument = "fireball",
    .colour = &RL_COLOUR_ORANGE[5],
    .hit = " scorches ",
    .critical = " incinerates ",
    .miss = " fails to burn ",
  },
  [RL_ATTACK_LIGHTNING] = {
    .instrument = "lightning",
    .colour = &RL_COLOUR_YELLOW[4],
    .hit = " shocks ",
    .critical = " electrocutes ",
    .miss = " arcs past ",
  },
};

static struct rl_text
build_attack_log(struct rl_world const* world,
                 struct rl_event_attack const* event)
{
  struct rl_actor const* attacker = rl_borrow_actor(world, event->attacker);
  struct rl_actor const* defender = rl_borrow_actor(world, event->defender);
  SDL_FColor const attacker_colour = actor_colour(attacker);
  SDL_FColor const defender_colour = actor_colour(defender);
  struct attack_phrasing const* phrasing = &ATTACK_PHRASINGS[event->kind];

  struct rl_text msg = { 0 };

  // <attacker>['s <instrument>] <hits or misses> <defender>
  rl_append_text(&msg, &attacker_colour, attacker->name);
  if (phrasing->instrument != NULL) {
    rl_append_text(&msg, NULL, "'s ");
    rl_append_text(&msg, phrasing->colour, phrasing->instrument);
  }

  char const* verb = phrasing->hit;
  if (event->damage < 0) {
    verb = phrasing->miss;
  } else if (event->critical) {
    verb = phrasing->critical;
  }
  rl_append_text(&msg, NULL, verb);
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
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  struct rl_actor const* killer = rl_borrow_actor(world, event->killer);
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
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
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
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  struct rl_item const* item = rl_borrow_item(world, event->item);
  SDL_FColor const actor_colour_ = actor_colour(actor);
  SDL_FColor const item_colour = rl_get_item_gfx(item).fg;

  char name[RL_TEXT_CAPACITY];
  rl_format_item_name(item, name, sizeof(name));

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &actor_colour_, actor->name);
  rl_append_text(&msg, NULL, " picked up a ");
  rl_append_text(&msg, &item_colour, name);

  return msg;
}

static struct rl_text
build_equip_log(struct rl_world const* world,
                struct rl_event_equipment const* event)
{
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  struct rl_item const* item = rl_borrow_item(world, event->item);
  struct rl_item const* replaced = rl_borrow_item(world, event->replaced);
  SDL_FColor const actor_colour_ = actor_colour(actor);
  SDL_FColor const item_colour = rl_get_item_gfx(item).fg;

  char name[RL_TEXT_CAPACITY];
  rl_format_item_name(item, name, sizeof(name));

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &actor_colour_, actor->name);
  if (replaced == NULL) {
    rl_append_text(&msg, NULL, " equipped a ");
  } else {
    SDL_FColor const replaced_colour = rl_get_item_gfx(replaced).fg;
    char replaced_name[RL_TEXT_CAPACITY];
    rl_format_item_name(replaced, replaced_name, sizeof(replaced_name));

    rl_append_text(&msg, NULL, " swapped a ");
    rl_append_text(&msg, &replaced_colour, replaced_name);
    rl_append_text(&msg, NULL, " for a ");
  }
  rl_append_text(&msg, &item_colour, name);
  rl_append_text(&msg, NULL, ".");

  return msg;
}

static struct rl_text
build_drop_log(struct rl_world const* world, struct rl_event_drop const* event)
{
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  struct rl_item const* item = rl_borrow_item(world, event->item);
  SDL_FColor const actor_colour_ = actor_colour(actor);
  SDL_FColor const item_colour = rl_get_item_gfx(item).fg;

  char name[RL_TEXT_CAPACITY];
  rl_format_item_name(item, name, sizeof(name));

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &actor_colour_, actor->name);
  rl_append_text(&msg, NULL, " dropped a ");
  rl_append_text(&msg, &item_colour, name);
  rl_append_text(&msg, NULL, ".");

  return msg;
}

static struct rl_text
build_heal_log(struct rl_world const* world, struct rl_event_heal const* event)
{
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  SDL_FColor const colour = actor_colour(actor);

  struct rl_text msg = { 0 };

  rl_append_text(&msg, &colour, actor->name);
  rl_append_text_format(
    &msg, NULL, " gained %d of %d HP.", event->effective, event->total);

  return msg;
}

static struct rl_text
build_feedback_log(struct rl_event_feedback const* event)
{
  struct rl_text msg = { 0 };

  rl_append_text(&msg, &RL_COLOUR_BLUE[3], event->message);

  return msg;
}

static struct rl_text
build_level_change_log(struct rl_event_level_change const* event)
{
  struct rl_text msg = { 0 };

  rl_append_text_format(&msg,
                        NULL,
                        event->to_depth > event->from_depth
                          ? "You descend to depth %d."
                          : "You climb up to depth %d.",
                        event->to_depth);

  return msg;
}

static struct rl_text
build_xp_gain_log(struct rl_world const* world,
                  struct rl_event_xp_gain const* event)
{
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  SDL_FColor const colour = actor_colour(actor);

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &colour, actor->name);
  rl_append_text_format(&msg, NULL, " gained %d XP.", event->amount);

  return msg;
}

static struct rl_text
build_level_up_log(struct rl_world const* world,
                   struct rl_event_level_up const* event)
{
  struct rl_actor const* actor = rl_borrow_actor(world, event->actor);
  SDL_FColor const colour = actor_colour(actor);

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &colour, actor->name);
  rl_append_text_format(&msg,
                        NULL,
                        " advanced from level %d to level %d!",
                        event->from_level,
                        event->to_level);

  return msg;
}

bool
rl_init_game_log(struct rl_game_log* log)
{
  if (!alist_alloc(&log->messages, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
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
rl_log_text(struct rl_game_log* log, struct rl_text const* message)
{
  *alist_push(&log->messages) = *message;
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
    case RL_EVENT_EQUIP:
      msg = build_equip_log(world, &event->as.equipment);
      break;
    case RL_EVENT_DROP:
      msg = build_drop_log(world, &event->as.drop);
      break;
    case RL_EVENT_HEAL:
      msg = build_heal_log(world, &event->as.heal);
      break;
    case RL_EVENT_FEEDBACK:
      msg = build_feedback_log(&event->as.feedback);
      break;
    case RL_EVENT_LEVEL_CHANGE:
      msg = build_level_change_log(&event->as.level_change);
      break;
    case RL_EVENT_XP_GAIN:
      msg = build_xp_gain_log(world, &event->as.xp_gain);
      break;
    case RL_EVENT_LEVEL_UP:
      msg = build_level_up_log(world, &event->as.level_up);
      break;
    default:
      return;
  }

  rl_log_text(log, &msg);
}
