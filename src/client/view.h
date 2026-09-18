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
  RL_VIEW_MAP,
  RL_VIEW_COUNT,
};

/**
 * A collection of callbacks, and their underlying state, that represent a
 * view.
 */
struct rl_view
{
  void* state;

  void (*free)(void* data);

  void (*describe_ribbon)(void const* data,
                          struct rl_ribbon_content* content);

  bool (*update)(void* data, struct inpt_state const* istate);

  void (*prepare)(void* data);

  void (*render)(void const* data,
                 SDL_Renderer* renderer,
                 struct gfx_tileset const* font);
};

static inline void
rl_free_view(struct rl_view* view)
{
  if (view->free == NULL) {
    return;
  }

  view->free(view->state);
}

static inline void
rl_view_describe_ribbon(struct rl_view const* view,
                        struct rl_ribbon_content* content)
{
  if (view->describe_ribbon == NULL) {
    return;
  }

  view->describe_ribbon(view->state, content);
}

static inline bool
rl_update_view(struct rl_view* view, struct inpt_state const* istate)
{
  if (view->update == NULL) {
    return false;
  }

  return view->update(view->state, istate);
}

static inline void
rl_prepare_view(struct rl_view* view)
{
  if (view->prepare == NULL) {
    return;
  }

  view->prepare(view->state);
}

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
