#include "in_sight.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"

#include "game/fov.h"
#include "game/world.h"

#include "ui/progress.h"
#include "ui/rectcut.h"

#include "graphics/console.h"
#include "graphics/tileset.h"

#include "render/palette.h"
#include "render/progress.h"

#include "client/view.h"

struct actor_row
{
  char const* name;
  char level_text[16];
  char hp_text[32];
  SDL_FPoint name_origin;
  SDL_FPoint level_origin;
  SDL_FPoint hp_origin;
  SDL_FPoint hp_bar_origin;
  struct ui_progress hp_bar;
};

alist_define_as(struct actor_row, actor_row);

struct view_state
{
  struct rl_world const* world;

  // Static layout/geometry
  float glyph_width;
  float line_height;
  float actor_height;
  SDL_FRect rogue_bounds;
  SDL_FRect body;

  // Presentation data
  alist(rl_actor_handle) monsters;
  alist(actor_row) actors;
  char more_text[32]; // empty when every monster fits
  SDL_FPoint more_origin;
};

static void
layout_view(struct view_state* s, SDL_FRect const* viewport)
{
  s->actor_height = 2.0f * s->line_height;

  SDL_FRect remaining = *viewport;
  s->rogue_bounds = ui_cut_top(&remaining, s->actor_height);
  ui_cut_top(&remaining, SDL_min(remaining.h, s->line_height));
  s->body = remaining;
}

static bool
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                struct gfx_tileset const* font)
{
  s->world = world;
  s->glyph_width = (float)font->tile_width;
  s->line_height = (float)font->tile_height;

  layout_view(s, viewport);

  if (!alist_alloc(&s->monsters, 16)) {
    return false;
  }

  if (!alist_alloc(&s->actors, 16)) {
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

  alist_free(&s->actors);
  alist_free(&s->monsters);
  SDL_free(s);
}

static void
collect_monsters(struct view_state* s)
{
  alist_clear(&s->monsters);

  handle(rl_actor) const rogue = rl_get_rogue(s->world);
  struct rl_level const* level = rl_get_current_level(s->world);

  for (size_t i = 0; i < alist_len(&level->actors); ++i) {
    struct rl_actor const* actor =
      rl_borrow_actor(s->world, *alist_at(&level->actors, i));
    if (handle_equal(actor->handle, rogue) || !rl_actor_is_alive(actor) ||
        !rl_is_tile_visible(&s->world->player.fov, actor->pos)) {
      continue;
    }

    *alist_push(&s->monsters) = actor->handle;
  }
}

static void
push_actor_row(struct view_state* s,
               struct rl_actor const* actor,
               SDL_FRect bounds)
{
  struct actor_row* row = alist_push(&s->actors);
  row->name = actor->name;
  SDL_snprintf(row->level_text, sizeof(row->level_text), "Lv %d", actor->level);
  SDL_snprintf(row->hp_text,
               sizeof(row->hp_text),
               "%d/%d",
               actor->hp,
               actor->stats.max_hp);

  SDL_FRect name = ui_cut_top(&bounds, s->line_height);
  row->name_origin = (SDL_FPoint){ name.x, name.y };

  float const level_width = (float)SDL_strlen(row->level_text) * s->glyph_width;
  SDL_FRect const level = ui_cut_right(&name, SDL_min(name.w, level_width));
  row->level_origin = (SDL_FPoint){ level.x, level.y };

  float const text_width = (float)SDL_strlen(row->hp_text) * s->glyph_width;
  SDL_FRect const label = ui_cut_right(&bounds, SDL_min(bounds.w, text_width));
  row->hp_origin = (SDL_FPoint){ label.x, label.y };

  ui_cut_right(&bounds, SDL_min(bounds.w, s->glyph_width));

  // Leave a pixel above and below the bar on the HP line.
  bounds.y += 1.0f;
  bounds.h = SDL_max(1.0f, s->line_height - 2.0f);
  row->hp_bar_origin = (SDL_FPoint){ bounds.x, bounds.y };
  row->hp_bar = ui_progress_layout(
    (SDL_FPoint){ bounds.w, bounds.h }, actor->hp, actor->stats.max_hp);
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
    push_actor_row(s, actor, ui_cut_top(remaining, s->actor_height));
    ui_cut_top(remaining, SDL_min(remaining->h, s->line_height));
  }

  if (overflow && footer.h > 0.0f) {
    SDL_snprintf(
      s->more_text, sizeof(s->more_text), "+%d more", monster_count - drawn);
    s->more_origin = (SDL_FPoint){ footer.x, footer.y };
  }
}

static void
draw_actor(SDL_Renderer* renderer,
           struct gfx_tileset const* font,
           struct actor_row const* row)
{
  gfx_print_console(renderer,
                    font,
                    str_view_from_cstr(row->name),
                    RL_COLOUR_GRAY[5],
                    RL_COLOUR_BLACK,
                    row->name_origin);
  gfx_print_console(renderer,
                    font,
                    str_view_from_cstr(row->level_text),
                    RL_COLOUR_GRAY[5],
                    RL_COLOUR_BLACK,
                    row->level_origin);
  gfx_print_console(renderer,
                    font,
                    str_view_from_cstr(row->hp_text),
                    RL_COLOUR_GRAY[5],
                    RL_COLOUR_BLACK,
                    row->hp_origin);
  rl_draw_progress(renderer,
                   &row->hp_bar,
                   RL_COLOUR_GREEN[5],
                   RL_COLOUR_GRAY[8],
                   row->hp_bar_origin);
}

static bool
handle_input(void* data, struct inpt_state const* istate)
{
  (void)data;
  (void)istate;

  return false;
}

static void
prepare_view(void* data, float dt)
{
  (void)dt;
  struct view_state* s = (struct view_state*)data;
  alist_clear(&s->actors);
  s->more_text[0] = '\0';

  collect_monsters(s);

  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  push_actor_row(s, rogue, s->rogue_bounds);

  SDL_FRect remaining = s->body;
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

  if (s->more_text[0] != '\0') {
    gfx_print_console(renderer,
                      font,
                      str_view_from_cstr(s->more_text),
                      RL_COLOUR_GRAY[5],
                      RL_COLOUR_BLACK,
                      s->more_origin);
  }
}

bool
rl_alloc_in_sight_view(struct rl_view* view,
                       struct rl_world const* world,
                       SDL_FRect const* viewport,
                       struct gfx_tileset const* font)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  if (!init_view_state(view->state, world, viewport, font)) {
    free_view(view->state);
    view->state = NULL;
    return false;
  }

  view->free = free_view;
  view->describe_ribbon = NULL;
  view->handle_input = handle_input;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}
