#include "game_over.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_stdinc.h>

#include "graphics/console.h"
#include "input/input.h"

#include "ui/anchor.h"

#include "render/palette.h"

#include "client/font.h"
#include "client/ribbon.h"
#include "client/screen.h"
#include "client/text.h"

/**
 * From: https://patorjk.com/software/taag/#p=display&f=Graceful
 */
static char const* const GAME_OVER_BANNER[] = {
  "  ___   __   _  _  ____     __   _  _  ____  ____ ",
  " / __) / _\\ ( \\/ )(  __)   /  \\ / )( \\(  __)(  _ \\",
  "( (_ \\/    \\/ \\/ \\ ) _)   (  O )\\ \\/ / ) _)  )   /",
  " \\___/\\_/\\_/\\_)(_/(____)   \\__/  \\__/ (____)(__\\_)",
};

struct screen_state
{
  struct gfx_tileset const* font;
  SDL_FPoint banner_pos;
};

static void
measure_banner(struct gfx_tileset const* font, float* width, float* height)
{
  size_t cols = 0;
  for (size_t i = 0; i < SDL_arraysize(GAME_OVER_BANNER); i++) {
    cols = SDL_max(cols, SDL_strlen(GAME_OVER_BANNER[i]));
  }

  *width = (float)cols * (float)font->tile_width;
  *height = (float)SDL_arraysize(GAME_OVER_BANNER) * (float)font->tile_height;
}

static void
create_layout(struct screen_state* s, SDL_FRect const* bounds)
{
  float banner_width = 0.0f;
  float banner_height = 0.0f;
  measure_banner(s->font, &banner_width, &banner_height);

  struct ui_position const centre = { UI_ANCHOR_CENTRE, { 0.0f, 0.0f } };
  SDL_FRect const block =
    ui_resolve(centre, bounds, banner_width, banner_height);

  s->banner_pos = (SDL_FPoint){ block.x, block.y };
}

static void
init_screen(struct screen_state* s,
            SDL_FRect const* bounds,
            struct gfx_tileset const* font)
{
  s->font = font;

  create_layout(s, bounds);
}

static void
free_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  if (s == NULL) {
    return;
  }

  SDL_free(s);
}

static void
enter_screen(void* data)
{
  (void)data;
}

static void
exit_screen(void* data)
{
  (void)data;
}

static struct rl_screen_transition
update_screen(void* data, struct inpt_state const* istate, float dt)
{
  (void)dt;

  struct rl_screen_transition transition = { 0 };
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  inpt_button const escape = istate->keys[SDL_SCANCODE_ESCAPE];

  if (inpt_was_pressed(escape)) {
    transition.type = RL_SCREEN_TRANSITION_POP;
  }

  return transition;
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  (void)data;

  rl_append_text(
    &content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "Game Over");
  rl_append_text(
    &content->text[RL_RIBBON_RIGHT], &RL_COLOUR_YELLOW[3], "[Esc] Main Menu");
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  // render the banner
  for (size_t i = 0; i < SDL_arraysize(GAME_OVER_BANNER); i++) {
    SDL_FPoint const at = {
      s->banner_pos.x,
      s->banner_pos.y + (float)i * (float)s->font->tile_height,
    };
    gfx_print_console(renderer,
                      s->font,
                      str_view_from_cstr(GAME_OVER_BANNER[i]),
                      RL_COLOUR_GRAY[5],
                      RL_COLOUR_BLACK,
                      at);
  }
}

bool
rl_alloc_game_over_screen(struct rl_screen* screen,
                          SDL_FRect const* bounds,
                          struct gfx_tileset const* font)
{
  screen->state = SDL_calloc(1, sizeof(struct screen_state));
  if (screen->state == NULL) {
    return false;
  }

  init_screen(screen->state, bounds, font);

  screen->free = free_screen;
  screen->enter = enter_screen;
  screen->exit = exit_screen;
  screen->describe_ribbon = describe_ribbon;
  screen->update = update_screen;
  screen->render = render_screen;

  return true;
}
