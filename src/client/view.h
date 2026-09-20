/**
 * @file view.h
 */
#ifndef GINC_ROGUELIKE_VIEW_H
#define GINC_ROGUELIKE_VIEW_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;
struct inpt_state;
struct rl_ribbon_content;

/**
 * Unique identifiers for each type of view.
 */
enum rl_view_id
{
  RL_VIEW_IN_SIGHT,
  RL_VIEW_INVENTORY,
  RL_VIEW_LOG,
  RL_VIEW_WORLD,
  RL_VIEW_COUNT,
};

/**
 * A collection of callbacks, and their underlying state, that represent a
 * view.
 */
struct rl_view
{
  /**
   * A pointer passed to all the view state callbacks.
   */
  void* state;

  /**
   * A callback that is called before the view is destroyed.
   */
  void (*free)(void* data);

  /**
   * An optional callback that fills in the ribbon text for the view's current
   * state.
   */
  void (*describe_ribbon)(void const* data, struct rl_ribbon_content* content);

  /**
   * An optional callback that handles input. It returns whether the input was
   * handled.
   */
  bool (*update)(void* data, struct inpt_state const* istate);

  /**
   * An optional callback that is called before the view is rendered.
   */
  void (*prepare)(void* data);

  /**
   * An optional callback that draws the view.
   */
  void (*render)(void const* data,
                 SDL_Renderer* renderer,
                 struct gfx_tileset const* font);
};

/**
 * Free the view.
 */
static inline void
rl_free_view(struct rl_view* view)
{
  if (view->free == NULL) {
    return;
  }

  view->free(view->state);
}

/**
 * Ask the view to fill in content; does nothing if it has no callback.
 */
static inline void
rl_view_describe_ribbon(struct rl_view const* view,
                        struct rl_ribbon_content* content)
{
  if (view->describe_ribbon == NULL) {
    return;
  }

  view->describe_ribbon(view->state, content);
}

/**
 * Let the view handle input.
 *
 * @return whether the input was handled.
 */
static inline bool
rl_update_view(struct rl_view* view, struct inpt_state const* istate)
{
  if (view->update == NULL) {
    return false;
  }

  return view->update(view->state, istate);
}

/**
 * Prepare the view for rendering.
 */
static inline void
rl_prepare_view(struct rl_view* view)
{
  if (view->prepare == NULL) {
    return;
  }

  view->prepare(view->state);
}

/**
 * Render the view.
 */
static inline void
rl_render_view(struct rl_view const* view,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font)
{
  if (view->render == NULL) {
    return;
  }

  view->render(view->state, renderer, font);
}

#endif // GINC_ROGUELIKE_VIEW_H
