#include "in_sight.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "container/alist.h"

#include "game/fov.h"
#include "game/world.h"

#include "ui/rectcut.h"

#include "client/font.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"
#include "client/view.h"

struct text_span
{
  char const* data;
  int length;
};

struct wrapped_line
{
  struct text_span text;
  char const* next;
};

alist_define_as(int, visible_actor_id);

struct view_state
{
  struct rl_world const* world;
  struct rl_fov const* fov;
  SDL_FRect viewport;
  alist(visible_actor_id) monsters;
  int item_count;
};

static bool
init_view_state(struct view_state* s,
                struct rl_world const* world,
                struct rl_fov const* fov,
                SDL_FRect const* viewport)
{
  s->world = world;
  s->fov = fov;
  s->viewport = *viewport;
  s->item_count = 0;

  if (!alist_alloc(&s->monsters, 16)) {
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

  alist_free(&s->monsters);
  SDL_free(s);
}

static bool
is_visible(struct view_state const* s, SDL_Point pos)
{
  return grid_contains(&s->fov->visible, pos.x, pos.y) &&
         *grid_at(&s->fov->visible, pos.x, pos.y);
}

static int
visible_item_count(struct view_state const* s)
{
  int count = 0;
  for (int id = 0; id < alist_len(&s->world->items); ++id) {
    struct rl_item const* item = rl_get_item(s->world, id);
    if (item->ltype == RL_ITEM_LOCATION_MAP && is_visible(s, item->on.map)) {
      ++count;
    }
  }
  return count;
}

static void
draw_health_bar(SDL_Renderer* renderer, SDL_FRect bounds, int hp, int max_hp)
{
  if (bounds.w <= 0.0f) {
    return;
  }

  // full width rectangle
  SDL_FColor const background = RL_COLOUR_GRAY[8];
  SDL_SetRenderDrawColorFloat(
    renderer, background.r, background.g, background.b, background.a);
  SDL_RenderFillRect(renderer, &bounds);

  // adjust width to remaining health
  float const fraction = (float)hp / max_hp;
  bounds.w = SDL_floorf(bounds.w * fraction);

  SDL_FColor const fill = RL_COLOUR_GREEN[5];
  SDL_SetRenderDrawColorFloat(renderer, fill.r, fill.g, fill.b, fill.a);
  SDL_RenderFillRect(renderer, &bounds);
}

static void
draw_actor(SDL_Renderer* renderer,
           struct rl_font const* font,
           struct rl_actor const* actor,
           SDL_FRect bounds)
{
  float const line_height = (float)font->glyph_height;
  SDL_FRect const name = ui_cut_top(&bounds, line_height);
  rl_draw_string(renderer,
                 font,
                 actor->name,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 (SDL_FPoint){ name.x, name.y });

  char text[32];
  SDL_snprintf(text, sizeof(text), "%d/%d", actor->hp, actor->max_hp);
  float const text_width = rl_font_width(font, SDL_strlen(text));
  SDL_FRect const label = ui_cut_right(&bounds, SDL_min(bounds.w, text_width));
  rl_draw_string(renderer,
                 font,
                 text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 (SDL_FPoint){ label.x, label.y });

  ui_cut_right(&bounds, SDL_min(bounds.w, (float)font->glyph_width));

  // Leave a pixel above and below the bar on the HP line.
  bounds.y += 1.0f;
  bounds.h = SDL_max(1.0f, line_height - 2.0f);
  draw_health_bar(renderer, bounds, actor->hp, actor->max_hp);
}

static struct wrapped_line
next_wrapped_line(char const* text, int columns)
{
  int length = 0;

  /* Take as many characters as will fit. */
  while (text[length] != '\0' && length < columns) {
    ++length;
  }

  /* If more text remains, prefer breaking at a space. */
  if (text[length] != '\0') {
    int split = length;

    while (split > 0 && text[split] != ' ') {
      --split;
    }

    if (split > 0) {
      length = split;
    }
  }

  char const* next = text + length;

  /* Skip spaces before the next line. */
  while (*next == ' ') {
    ++next;
  }

  return (struct wrapped_line){
    .text = {
      .data = text,
      .length = length,
    },
    .next = next,
  };
}

static void
draw_text_span(SDL_Renderer* renderer,
               struct rl_font const* font,
               struct text_span text,
               SDL_FPoint at)
{
  struct rl_cell cell = {
    .fg = RL_COLOUR_GRAY[5],
    .bg = RL_COLOUR_BLACK,
  };

  for (int i = 0; i < text.length; ++i) {
    cell.glyph = (Uint8)text.data[i];
    rl_draw_cell(renderer, font, &cell, at);
    at.x += font->glyph_width;
  }
}

static void
draw_wrapped_string(SDL_Renderer* renderer,
                    struct rl_font const* font,
                    char const* text,
                    SDL_FRect* remaining)
{
  int const columns = (int)(remaining->w / (float)font->glyph_width);
  if (columns <= 0) {
    return;
  }

  char const* cursor = text;
  while (*cursor != '\0' && remaining->h >= font->glyph_height) {
    struct wrapped_line const line = next_wrapped_line(cursor, columns);

    SDL_FRect const row = ui_cut_top(remaining, (float)font->glyph_height);

    draw_text_span(renderer, font, line.text, (SDL_FPoint){ row.x, row.y });

    cursor = line.next;
  }
}

static void
draw_item_summary(SDL_Renderer* renderer,
                  struct rl_font const* font,
                  int count,
                  SDL_FRect* remaining)
{
  char text[64];
  SDL_snprintf(text,
               sizeof(text),
               "You see %d %s nearby.",
               count,
               count == 1 ? "item" : "items");
  draw_wrapped_string(renderer, font, text, remaining);
}

static void
update_ribbon(void const* data, struct rl_ribbon* ribbon)
{
  (void)data;
  (void)ribbon;
}

static bool
update_view(void* data, struct inpt_state const* istate, struct rl_command* out)
{
  (void)data;
  (void)istate;
  (void)out;

  return false;
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  s->item_count = visible_item_count(s);
  alist_clear(&s->monsters);

  for (int id = 0; id < rl_actor_count(s->world); ++id) {
    struct rl_actor const* actor = rl_get_actor(s->world, id);
    if (id == RL_ROGUE_ID || !rl_actor_is_alive(actor) ||
        !is_visible(s, actor->pos)) {
      continue;
    }

    int* entry = alist_push(&s->monsters);
    if (entry == NULL) {
      return;
    }
    *entry = id;
  }
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  float const line_height = (float)font->glyph_height;
  float const actor_height = 2.0f * line_height;

  SDL_FRect remaining = s->viewport;
  if (remaining.w <= 0.0f || remaining.h < actor_height) {
    return;
  }

  struct rl_actor const* rogue = rl_get_actor(s->world, RL_ROGUE_ID);
  draw_actor(renderer, font, rogue, ui_cut_top(&remaining, actor_height));
  ui_cut_top(&remaining, SDL_min(remaining.h, line_height));

  if (s->item_count > 0) {
    draw_item_summary(renderer, font, s->item_count, &remaining);
    ui_cut_top(&remaining, SDL_min(remaining.h, line_height));
  }

  int const monster_count = (int)alist_len(&s->monsters);

  int capacity =
    (int)((remaining.h + line_height) / (actor_height + line_height));
  bool const overflow = monster_count > capacity;

  SDL_FRect footer = { 0 };
  if (overflow && remaining.h >= line_height) {
    footer = ui_cut_bottom(&remaining, line_height);
    capacity =
      (int)((remaining.h + line_height) / (actor_height + line_height));
  }

  int const drawn = SDL_min(monster_count, capacity);
  for (int row = 0; row < drawn; ++row) {
    int const id = *alist_at(&s->monsters, row);
    struct rl_actor const* actor = rl_get_actor(s->world, id);
    draw_actor(renderer, font, actor, ui_cut_top(&remaining, actor_height));
    ui_cut_top(&remaining, SDL_min(remaining.h, line_height));
  }

  if (overflow && footer.h > 0.0f) {
    char text[32];
    SDL_snprintf(text, sizeof(text), "+%d more", monster_count - drawn);
    rl_draw_string(renderer,
                   font,
                   text,
                   RL_COLOUR_GRAY[5],
                   RL_COLOUR_BLACK,
                   (SDL_FPoint){ footer.x, footer.y });
  }
}

bool
rl_alloc_in_sight_view(struct rl_view* view,
                       struct rl_world const* world,
                       struct rl_fov const* fov,
                       SDL_FRect const* viewport)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  if (!init_view_state(view->state, world, fov, viewport)) {
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
