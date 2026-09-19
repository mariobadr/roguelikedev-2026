#include "world.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "input/input.h"

#include "game/targeting.h"
#include "game/tile.h"
#include "game/world.h"

#include "graphics/camera.h"
#include "graphics/grid_view.h"

#include "render/palette.h"
#include "render/world.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/ribbon.h"
#include "client/view.h"

/**
 * Possible input modes for the world view.
 */
enum world_mode
{
  WORLD_MODE_MOVE,   //< Move the rogue around
  WORLD_MODE_SELECT, //< Move the cursor around
};

struct world_selection
{
  SDL_Point cursor;
  struct rl_item_def const* def;
  grid(boolean) mask;
  size_t capacity;
  SDL_Rect bounds;
};

struct view_state
{
  // the "model" for this view
  struct rl_world const* world;

  /** Which mode the view is in. */
  enum world_mode mode;

  struct gfx_camera camera;
  struct gfx_grid_view grid_view;
  struct rl_world_renderer renderer;

  // for the MOVE mode
  struct rl_command pending_command;

  // for the SELECT mode
  struct world_selection selection;
  enum rl_world_selection_result pending_select;
  SDL_Point pending_point;
};

static bool
init_view_state(struct view_state* s,
                struct rl_world const* world,
                SDL_FRect const* viewport,
                int cell_width,
                int cell_height)
{
  s->world = world;

  gfx_init_camera(&s->camera, viewport);
  s->grid_view = (struct gfx_grid_view){ cell_width, cell_height };
  SDL_Rect const bounds = gfx_grid_view_bounds(&s->grid_view, &s->camera);
  return rl_init_world_renderer(&s->renderer, bounds.w, bounds.h);
}

static bool
reserve_area(struct world_selection* selection, int w, int h)
{
  size_t const count = (size_t)w * (size_t)h;
  if (count > selection->capacity) {
    bool* data = SDL_realloc(selection->mask.data, count * sizeof(*data));
    if (data == NULL) {
      SDL_Log("SDL_realloc failed: %s", SDL_GetError());
      return false;
    }

    selection->mask.data = data;
    selection->capacity = count;
  }

  selection->mask.width = w;
  selection->mask.height = h;

  return true;
}

/**
 * Refresh after selecting an item or moving the cursor. The world cannot
 * change while selecting, and moving the cursor does not change mask
 * dimensions.
 */
static void
refresh_area(struct view_state* s)
{
  struct world_selection* selection = &s->selection;
  selection->bounds = rl_item_area_bounds(selection->def, selection->cursor);
  rl_fill_item_area(
    selection->def, s->world, selection->cursor, &selection->mask);
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  struct view_state const* s = (struct view_state const*)data;
  SDL_assert(s != NULL);

  rl_append_text(&content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "World");

  struct rl_text* hint = &content->text[RL_RIBBON_RIGHT];
  SDL_FColor const* const colour = &RL_COLOUR_YELLOW[3];
  if (s->mode == WORLD_MODE_SELECT) {
    rl_append_text(hint, colour, "[WASD] move cursor");
    if (rl_is_valid_item_target(
          s->selection.def, s->world, s->selection.cursor)) {
      rl_append_text(hint, colour, "   [E] confirm");
    }
    rl_append_text(hint, colour, "   [Esc] cancel");
    return;
  }

  rl_append_text(hint, colour, "[WASD] move");
  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  switch (rl_available_interaction(rogue, s->world)) {
    case RL_INTERACTION_PICK_UP:
      rl_append_text(hint, colour, "   [E] pick up");
      break;
    case RL_INTERACTION_TAKE_STAIRS:
      rl_append_text(hint, colour, "   [E] take stairs");
      break;
    case RL_INTERACTION_NONE:
      break;
  }
  rl_append_text(hint, colour, "   [Z] wait");
}

static bool
update_move(struct view_state* s, struct inpt_state const* istate)
{
  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));
  if (rogue == NULL) {
    return false;
  }

  enum rl_action action = rl_handle_keyboard_input(istate);
  if (action == RL_ACTION_NONE) {
    grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;

    SDL_Point target;
    if (gfx_screen_to_cell(
          &s->grid_view, &s->camera, istate->mouse.position, &target) &&
        grid_contains(map, target.x, target.y)) {
      action = rl_handle_mouse_input(istate, rogue->pos, target);
    }
  }

  switch (action) {
    case RL_ACTION_MOVE_UP:
    case RL_ACTION_MOVE_DOWN:
    case RL_ACTION_MOVE_LEFT:
    case RL_ACTION_MOVE_RIGHT:
    case RL_ACTION_SELECT:
    case RL_ACTION_WAIT:
      s->pending_command = rl_build_command(rogue, action, s->world);
      return true;
    default:
      return false;
  }
}

static void
end_select(struct view_state* s, enum rl_world_selection_result result)
{
  s->pending_select = result;
  s->mode = WORLD_MODE_MOVE;
  s->selection.def = NULL;
}

static bool
update_select(struct view_state* s, struct inpt_state const* istate)
{
  enum rl_action const action = rl_handle_keyboard_input(istate);

  SDL_Point next = s->selection.cursor;
  switch (action) {
    case RL_ACTION_MOVE_UP:
      next.y -= 1;
      break;
    case RL_ACTION_MOVE_DOWN:
      next.y += 1;
      break;
    case RL_ACTION_MOVE_LEFT:
      next.x -= 1;
      break;
    case RL_ACTION_MOVE_RIGHT:
      next.x += 1;
      break;
    case RL_ACTION_SELECT:
      if (!rl_is_valid_item_target(
            s->selection.def, s->world, s->selection.cursor)) {
        return true;
      }
      s->pending_point = s->selection.cursor;
      end_select(s, RL_WORLD_SELECTION_CONFIRMED);
      return true;
    case RL_ACTION_CANCEL:
      end_select(s, RL_WORLD_SELECTION_CANCELLED);
      return true;
    default:
      return false;
  }

  if (rl_is_valid_item_target(s->selection.def, s->world, next)) {
    s->selection.cursor = next;
    refresh_area(s);
  }

  return true;
}

static bool
update_view(void* data, struct inpt_state const* istate)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  if (s->mode == WORLD_MODE_SELECT) {
    return update_select(s, istate);
  }

  return update_move(s, istate);
}

static void
prepare_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue =
    rl_borrow_actor(s->world, rl_get_rogue(s->world));

  SDL_Point const focus =
    s->mode == WORLD_MODE_SELECT ? s->selection.cursor : rogue->pos;
  grid(rl_tile) const* map = &rl_get_current_level(s->world)->map;
  int const columns = (int)s->camera.viewport.w / s->grid_view.cell_width;
  int const rows = (int)s->camera.viewport.h / s->grid_view.cell_height;
  int const max_x = SDL_max(0, grid_width(map) - columns);
  int const max_y = SDL_max(0, grid_height(map) - rows);
  SDL_Point const origin = {
    SDL_clamp(focus.x - columns / 2, 0, max_x),
    SDL_clamp(focus.y - rows / 2, 0, max_y)
  };
  s->camera.position = gfx_cell_to_world(&s->grid_view, origin);

  rl_prepare_world_renderer(&s->renderer, s->world, &s->camera, &s->grid_view);
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct gfx_tileset const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  rl_draw_world(
    &s->renderer, s->world, &s->camera, &s->grid_view, renderer, font);

  if (s->mode == WORLD_MODE_SELECT) {
    rl_draw_world_target_area(&s->renderer,
                              &s->camera,
                              &s->grid_view,
                              renderer,
                              &s->selection.bounds,
                              &s->selection.mask);
    rl_draw_world_cursor(
      &s->camera, &s->grid_view, renderer, s->selection.cursor);
  }
}

static void
free_view(void* data)
{
  struct view_state* s = (struct view_state*)data;
  if (s == NULL) {
    return;
  }

  grid_free(&s->selection.mask);
  rl_free_world_renderer(&s->renderer);
  SDL_free(s);
}

bool
rl_alloc_world_view(struct rl_view* view,
                  struct rl_world const* world,
                  SDL_FRect const* viewport,
                  int cell_width,
                  int cell_height)
{
  SDL_assert(SDL_fmodf(viewport->w, (float)cell_width) == 0.0f);
  SDL_assert(SDL_fmodf(viewport->h, (float)cell_height) == 0.0f);

  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  if (!init_view_state(view->state, world, viewport, cell_width, cell_height)) {
    free_view(view->state);
    view->state = NULL;
    return false;
  }

  view->free = free_view;
  view->describe_ribbon = describe_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}

bool
rl_world_view_take_command(struct rl_view* view, struct rl_command* out)
{
  struct view_state* s = view->state;
  SDL_assert(s != NULL);

  if (s->pending_command.type == RL_COMMAND_NONE) {
    return false;
  }

  *out = s->pending_command;
  s->pending_command = (struct rl_command){ 0 };

  return true;
}

bool
rl_world_view_begin_select(struct rl_view* view,
                         SDL_Point origin,
                         struct rl_item_def const* def)
{
  SDL_assert(def != NULL);

  struct view_state* s = (struct view_state*)view->state;
  SDL_assert(s != NULL);

  // Reserve storage before entering selection mode.
  SDL_Rect const bounds = rl_item_area_bounds(def, origin);
  if (!reserve_area(&s->selection, bounds.w, bounds.h)) {
    return false;
  }

  s->selection.cursor = origin;
  s->selection.def = def;
  refresh_area(s);
  s->pending_select = RL_WORLD_SELECTION_NONE;
  s->mode = WORLD_MODE_SELECT;

  return true;
}

enum rl_world_selection_result
rl_world_view_take_selection(struct rl_view* view, SDL_Point* out)
{
  SDL_assert(out != NULL);

  struct view_state* s = (struct view_state*)view->state;
  SDL_assert(s != NULL);

  enum rl_world_selection_result const result = s->pending_select;
  if (result != RL_WORLD_SELECTION_NONE) {
    if (result == RL_WORLD_SELECTION_CONFIRMED) {
      *out = s->pending_point;
    }

    // reset before returning
    s->pending_select = RL_WORLD_SELECTION_NONE;
  }

  return result;
}
