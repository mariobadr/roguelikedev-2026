#include "character.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "game/actor.h"
#include "game/equipment.h"
#include "game/experience.h"
#include "game/item.h"
#include "game/world.h"

#include "graphics/tileset.h"

#include "render/palette.h"
#include "render/progress.h"

#include "ui/layout.h"
#include "ui/progress.h"
#include "ui/rectcut.h"

#include "client/ribbon.h"
#include "client/text.h"
#include "client/view.h"

enum stat_row
{
  STAT_LEVEL,
  STAT_HP,
  STAT_STRENGTH,
  STAT_AGILITY,
  STAT_ARMOUR,
  STAT_COUNT,
};

#define FIRST_SLOT RL_EQUIPMENT_SLOT_WEAPON
#define SLOT_COUNT (RL_EQUIPMENT_SLOT_COUNT - FIRST_SLOT)

static char const* const SLOT_LABELS[RL_EQUIPMENT_SLOT_COUNT] = {
  [RL_EQUIPMENT_SLOT_WEAPON] = "Weapon",
  [RL_EQUIPMENT_SLOT_ARMOUR] = "Armour",
};

#define LINE_COUNT (STAT_COUNT + SLOT_COUNT)
#define ROW_COUNT SDL_max(STAT_COUNT, SLOT_COUNT)
#define LABEL_COLUMNS 10

struct line
{
  struct rl_text text;
  SDL_FPoint at;
};

struct view_state
{
  struct rl_world const* world;
  float glyph_width;
  float line_height;
  // the stat rows, then the slots from FIRST_SLOT
  struct line lines[LINE_COUNT];
  SDL_FRect xp_row;
  struct line xp_label;
  struct line xp_value;
  SDL_FPoint xp_bar_origin;
  struct ui_progress xp_bar;
};

static void
layout_column(struct line* lines,
              int count,
              SDL_FRect const* column,
              float line_height)
{
  SDL_FRect rects[LINE_COUNT];
  SDL_assert(count <= LINE_COUNT);

  struct ui_position const pos = { UI_ANCHOR_TOP_LEFT, { 0.0f, 0.0f } };
  struct ui_strip const strip = { count, column->w, line_height, 0.0f };
  ui_layout_column(pos, column, strip, rects);

  for (int i = 0; i < count; ++i) {
    lines[i].at = (SDL_FPoint){ rects[i].x, rects[i].y };
  }
}

static void
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                struct gfx_tileset const* font)
{
  float const glyph_width = (float)font->tile_width;
  float const line_height = (float)font->tile_height;
  // the columns, a blank line, then the XP row
  SDL_assert(viewport->h >= (ROW_COUNT + 2) * line_height);

  s->world = world;
  s->glyph_width = glyph_width;
  s->line_height = line_height;

  SDL_FRect below = *viewport;
  SDL_FRect remaining = ui_cut_top(&below, ROW_COUNT * line_height);
  ui_cut_top(&below, line_height);
  s->xp_row = ui_cut_top(&below, line_height);

  float const half = SDL_floorf(remaining.w / 2.0f / glyph_width) * glyph_width;
  SDL_FRect const left = ui_cut_left(&remaining, half);
  ui_cut_left(&remaining, SDL_min(remaining.w, glyph_width));

  layout_column(s->lines, STAT_COUNT, &left, line_height);
  layout_column(s->lines + STAT_COUNT, SLOT_COUNT, &remaining, line_height);
}

static void
append_label(struct rl_text* text, char const* label)
{
  rl_append_text_format(text, &RL_COLOUR_GRAY[5], "%-*s", LABEL_COLUMNS, label);
}

static void
append_bonus(struct rl_text* text, int bonus)
{
  if (bonus != 0) {
    rl_append_text_format(text, &RL_COLOUR_GREEN[5], " (%+d)", bonus);
  }
}

static void
append_stat(struct rl_text* text, char const* label, int total, int base)
{
  append_label(text, label);
  rl_append_text_format(text, NULL, "%d", total);
  append_bonus(text, total - base);
}

static struct rl_text
stat_text(struct rl_actor const* rogue,
          struct rl_actor_stats const* total,
          enum stat_row row)
{
  struct rl_text text = { 0 };

  switch (row) {
    case STAT_LEVEL:
      append_label(&text, "Level");
      rl_append_text_format(&text, NULL, "%d", rogue->level);
      break;
    case STAT_HP:
      append_label(&text, "HP");
      rl_append_text_format(&text, NULL, "%d/%d", rogue->hp, total->max_hp);
      break;
    case STAT_STRENGTH:
      append_stat(&text, "Strength", total->strength, rogue->stats.strength);
      break;
    case STAT_AGILITY:
      append_stat(&text, "Agility", total->agility, rogue->stats.agility);
      break;
    case STAT_ARMOUR:
      append_stat(&text, "Armour", total->armor, rogue->stats.armor);
      break;
    default:
      break;
  }

  return text;
}

static struct rl_text
slot_text(struct rl_world const* world,
          struct rl_actor const* rogue,
          enum rl_equipment_slot slot)
{
  struct rl_text text = { 0 };

  char const* label = SLOT_LABELS[slot];
  append_label(&text, label != NULL ? label : "");

  struct rl_item const* item =
    rl_borrow_item(world, rl_get_equipped_item(rogue, slot));
  if (item != NULL) {
    char name[RL_TEXT_CAPACITY];
    rl_format_item_name(item, name, sizeof(name));
    rl_append_text(&text, NULL, name);
  } else {
    rl_append_text(&text, &RL_COLOUR_GRAY[5], "(none)");
  }

  return text;
}

static void
prepare_xp(struct view_state* s, struct rl_actor const* rogue)
{
  int const xp = s->world->player.xp;
  int const required = rl_xp_required(rogue->level);

  s->xp_label.text = (struct rl_text){ 0 };
  append_label(&s->xp_label.text, "XP");
  s->xp_value.text = (struct rl_text){ 0 };
  rl_append_text_format(&s->xp_value.text, NULL, "%d/%d", xp, required);

  SDL_FRect bounds = s->xp_row;
  s->xp_label.at = (SDL_FPoint){ bounds.x, bounds.y };
  ui_cut_left(&bounds, SDL_min(bounds.w, LABEL_COLUMNS * s->glyph_width));

  float const value_width = (float)s->xp_value.text.length * s->glyph_width;
  SDL_FRect const value = ui_cut_right(&bounds, SDL_min(bounds.w, value_width));
  s->xp_value.at = (SDL_FPoint){ value.x, value.y };

  ui_cut_right(&bounds, SDL_min(bounds.w, s->glyph_width));

  // Leave a pixel above and below the bar.
  bounds.y += 1.0f;
  bounds.h = SDL_max(1.0f, s->line_height - 2.0f);
  s->xp_bar_origin = (SDL_FPoint){ bounds.x, bounds.y };
  s->xp_bar =
    ui_progress_layout((SDL_FPoint){ bounds.w, bounds.h }, xp, required);
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  (void)data;

  rl_append_text(
    &content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "Character");
  rl_append_text(
    &content->text[RL_RIBBON_RIGHT], &RL_COLOUR_YELLOW[3], "[Q] back");
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  struct rl_actor_stats const total = rl_get_actor_stats(s->world, rogue);

  for (int row = 0; row < STAT_COUNT; ++row) {
    s->lines[row].text = stat_text(rogue, &total, (enum stat_row)row);
  }

  for (int i = 0; i < SLOT_COUNT; ++i) {
    s->lines[STAT_COUNT + i].text =
      slot_text(s->world, rogue, (enum rl_equipment_slot)(FIRST_SLOT + i));
  }

  prepare_xp(s, rogue);
}

static void
draw_line(SDL_Renderer* renderer,
          struct gfx_tileset const* font,
          struct line const* line)
{
  rl_draw_text(
    renderer, font, &line->text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, line->at);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  for (int i = 0; i < LINE_COUNT; ++i) {
    draw_line(renderer, font, &s->lines[i]);
  }

  draw_line(renderer, font, &s->xp_label);
  draw_line(renderer, font, &s->xp_value);
  rl_draw_progress(renderer,
                   &s->xp_bar,
                   RL_COLOUR_GRAPE[5],
                   RL_COLOUR_GRAY[8],
                   s->xp_bar_origin);
}

bool
rl_alloc_character_view(struct rl_view* view,
                        struct rl_world const* world,
                        SDL_FRect const* viewport,
                        struct gfx_tileset const* font)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  init_view_state(view->state, world, viewport, font);

  view->free = SDL_free;
  view->describe_ribbon = describe_ribbon;
  view->update = NULL;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}
