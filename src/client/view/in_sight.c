#include "in_sight.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "container/alist.h"

#include "game/fov.h"
#include "game/world.h"

#include "ui/rectcut.h"
#include "ui/str_wrap.h"

#include "graphics/console.h"
#include "graphics/tileset.h"

#include "client/palette.h"
#include "client/render.h"
#include "client/view.h"

struct text_row
{
  struct ui_string_span span;
  SDL_FPoint origin;
};

alist_define_as(struct text_row, text_row);

struct actor_row
{
  char const* name;
  char hp_text[32];
  SDL_FPoint name_origin;
  SDL_FPoint hp_origin;
  SDL_FRect bar;
  SDL_FRect fill;
};

alist_define_as(struct actor_row, actor_row);

struct view_state
{
  struct rl_world const* world;
  struct rl_fov const* fov;

  // Static layout/geometry
  float glyph_width;
  float line_height;
  float actor_height;
  int columns;
  bool fits;
  SDL_FRect rogue_bounds;
  SDL_FRect body;

  // Presentation data
  alist(rl_actor_handle) monsters;
  alist(actor_row) actors;
  alist(text_row) summary;
  char summary_text[64];
  char more_text[32]; // empty when every monster fits
  SDL_FPoint more_origin;
};

static void
layout_view(struct view_state* s, SDL_FRect const* viewport)
{
  s->actor_height = 2.0f * s->line_height;
  s->fits = viewport->w > 0.0f && viewport->h >= s->actor_height;
  if (!s->fits) {
    return;
  }

  s->columns = (int)(viewport->w / s->glyph_width);

  SDL_FRect remaining = *viewport;
  s->rogue_bounds = ui_cut_top(&remaining, s->actor_height);
  ui_cut_top(&remaining, SDL_min(remaining.h, s->line_height));
  s->body = remaining;
}

static bool
init_view_state(struct view_state* s,
                struct rl_world const* world,
                struct rl_fov const* fov,
                SDL_FRect const* viewport,
                struct gfx_tileset const* font)
{
  s->world = world;
  s->fov = fov;
  s->glyph_width = (float)font->tile_width;
  s->line_height = (float)font->tile_height;

  layout_view(s, viewport);

  if (!alist_alloc(&s->monsters, 16)) {
    return false;
  }

  if (!alist_alloc(&s->actors, 16)) {
    return false;
  }

  if (!alist_alloc(&s->summary, 4)) {
    return false;
  }

  return true;
}

static void
free_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  if (s == NULL) {
    return;
  }

  alist_free(&s->summary);
  alist_free(&s->actors);
  alist_free(&s->monsters);
  SDL_free(s);
}

static int
visible_item_count(struct view_state const* s)
{
  struct rl_level const* level = rl_get_current_level(s->world);

  int count = 0;
  for (size_t i = 0; i < alist_len(&level->items); ++i) {
    struct rl_item const* item =
      rl_borrow_item(s->world, *alist_at(&level->items, i));
    if (item != NULL && rl_is_tile_visible(s->fov, item->on.map)) {
      ++count;
    }
  }
  return count;
}

static bool
collect_monsters(struct view_state* s)
{
  alist_clear(&s->monsters);

  handle(rl_actor) const rogue = rl_get_rogue(s->world);
  struct rl_level const* level = rl_get_current_level(s->world);

  for (size_t i = 0; i < alist_len(&level->actors); ++i) {
    struct rl_actor const* actor =
      rl_borrow_actor(s->world, *alist_at(&level->actors, i));
    if (actor == NULL || handle_equal(actor->handle, rogue) ||
        !rl_actor_is_alive(actor) || !rl_is_tile_visible(s->fov, actor->pos)) {
      continue;
    }

    handle(rl_actor)* entry = alist_push(&s->monsters);
    if (entry == NULL) {
      return false;
    }
    *entry = actor->handle;
  }

  return true;
}

static bool
push_actor_row(struct view_state* s,
               struct rl_actor const* actor,
               SDL_FRect bounds)
{
  struct actor_row* row = alist_push(&s->actors);
  if (row == NULL) {
    return false;
  }

  row->name = actor->name;
  SDL_snprintf(
    row->hp_text, sizeof(row->hp_text), "%d/%d", actor->hp, actor->max_hp);

  SDL_FRect const name = ui_cut_top(&bounds, s->line_height);
  row->name_origin = (SDL_FPoint){ name.x, name.y };

  float const text_width = (float)SDL_strlen(row->hp_text) * s->glyph_width;
  SDL_FRect const label = ui_cut_right(&bounds, SDL_min(bounds.w, text_width));
  row->hp_origin = (SDL_FPoint){ label.x, label.y };

  ui_cut_right(&bounds, SDL_min(bounds.w, s->glyph_width));

  // Leave a pixel above and below the bar on the HP line.
  bounds.y += 1.0f;
  bounds.h = SDL_max(1.0f, s->line_height - 2.0f);
  row->bar = bounds;

  float const fraction = (float)actor->hp / actor->max_hp;
  row->fill = bounds;
  row->fill.w = SDL_floorf(bounds.w * fraction);

  return true;
}

static void
prepare_summary(struct view_state* s, int count, SDL_FRect* remaining)
{
  SDL_snprintf(s->summary_text,
               sizeof(s->summary_text),
               "You see %d %s nearby.",
               count,
               count == 1 ? "item" : "items");

  if (s->columns <= 0) {
    return;
  }

  char const* cursor = s->summary_text;
  struct ui_string_span span;
  while (remaining->h >= s->line_height) {
    if (!ui_wrap_next(&cursor, s->columns, &span)) {
      break;
    }

    struct text_row* row = alist_push(&s->summary);
    if (row == NULL) {
      return;
    }

    SDL_FRect const line = ui_cut_top(remaining, s->line_height);
    row->span = span;
    row->origin = (SDL_FPoint){ line.x, line.y };
  }
}

static void
prepare_monsters(struct view_state* s, SDL_FRect* remaining)
{
  int const monster_count = (int)alist_len(&s->monsters);
  float const stride = s->actor_height + s->line_height;

  int capacity = (int)((remaining->h + s->line_height) / stride);
  bool const overflow = monster_count > capacity;

  SDL_FRect footer = { 0 };
  if (overflow && remaining->h >= s->line_height) {
    footer = ui_cut_bottom(remaining, s->line_height);
    capacity = (int)((remaining->h + s->line_height) / stride);
  }

  int const drawn = SDL_min(monster_count, capacity);
  for (int i = 0; i < drawn; ++i) {
    struct rl_actor const* actor =
      rl_borrow_actor(s->world, *alist_at(&s->monsters, i));
    if (!push_actor_row(s, actor, ui_cut_top(remaining, s->actor_height))) {
      return;
    }
    ui_cut_top(remaining, SDL_min(remaining->h, s->line_height));
  }

  if (overflow && footer.h > 0.0f) {
    SDL_snprintf(
      s->more_text, sizeof(s->more_text), "+%d more", monster_count - drawn);
    s->more_origin = (SDL_FPoint){ footer.x, footer.y };
  }
}

// TODO: figure out how to add this to either client/render or libs/graphics
static void
draw_text_span(SDL_Renderer* renderer,
               struct gfx_tileset const* font,
               struct text_row const* row)
{
  struct gfx_console_cell cell = {
    .fg = RL_COLOUR_GRAY[5],
    .bg = RL_COLOUR_BLACK,
  };

  SDL_FPoint at = row->origin;
  for (int i = 0; i < row->span.length; ++i) {
    cell.index = (Uint8)row->span.data[i];
    SDL_FRect dst = gfx_tileset_dst(font, at, 1);
    gfx_draw_console_cell(renderer, font, &cell, &dst);
    at.x += font->tile_width;
  }
}

static void
draw_health_bar(SDL_Renderer* renderer, struct actor_row const* row)
{
  if (row->bar.w <= 0.0f) {
    return;
  }

  SDL_FColor const background = RL_COLOUR_GRAY[8];
  SDL_SetRenderDrawColorFloat(
    renderer, background.r, background.g, background.b, background.a);
  SDL_RenderFillRect(renderer, &row->bar);

  SDL_FColor const fill = RL_COLOUR_GREEN[5];
  SDL_SetRenderDrawColorFloat(renderer, fill.r, fill.g, fill.b, fill.a);
  SDL_RenderFillRect(renderer, &row->fill);
}

static void
draw_actor(SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           struct actor_row const* row)
{
  rl_draw_string(renderer,
                 font,
                 row->name,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 row->name_origin);
  rl_draw_string(renderer,
                 font,
                 row->hp_text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 row->hp_origin);
  draw_health_bar(renderer, row);
}

static void
update_ribbon(void const* data, struct rl_ribbon* ribbon)
{
  (void)data;
  (void)ribbon;
}

static bool
update_view(void* data, struct inpt_state const* istate)
{
  (void)data;
  (void)istate;

  return false;
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  alist_clear(&s->actors);
  alist_clear(&s->summary);
  s->more_text[0] = '\0';

  if (!s->fits || !collect_monsters(s)) {
    return;
  }

  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  if (!push_actor_row(s, rogue, s->rogue_bounds)) {
    return;
  }

  SDL_FRect remaining = s->body;

  int const item_count = visible_item_count(s);
  if (item_count > 0) {
    prepare_summary(s, item_count, &remaining);
    ui_cut_top(&remaining, SDL_min(remaining.h, s->line_height));
  }

  prepare_monsters(s, &remaining);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  for (size_t i = 0; i < alist_len(&s->actors); ++i) {
    draw_actor(renderer, font, alist_at(&s->actors, i));
  }

  for (size_t i = 0; i < alist_len(&s->summary); ++i) {
    draw_text_span(renderer, font, alist_at(&s->summary, i));
  }

  if (s->more_text[0] != '\0') {
    rl_draw_string(renderer,
                   font,
                   s->more_text,
                   RL_COLOUR_GRAY[5],
                   RL_COLOUR_BLACK,
                   s->more_origin);
  }
}

bool
rl_alloc_in_sight_view(struct rl_view* view,
                       struct rl_world const* world,
                       struct rl_fov const* fov,
                       SDL_FRect const* viewport,
                       struct gfx_tileset const* font)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  if (!init_view_state(view->state, world, fov, viewport, font)) {
    free_view(view->state);
    view->state = NULL;
    return false;
  }

  view->free = free_view;
  view->update_ribbon = update_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}
