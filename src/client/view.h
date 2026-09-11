/**
 * @file view.h
 */
#ifndef GINC_ROGUELIKE_VIEW_H
#define GINC_ROGUELIKE_VIEW_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;
struct rl_command;
struct rl_font;
struct rl_ribbon;

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

  void (*update_ribbon)(void const* data, struct rl_ribbon* ribbon);

  bool (*update)(void* data,
                 struct inpt_state const* istate,
                 struct rl_command* out);

  void (*prepare)(void* data);

  void (*render)(void const* data,
                 SDL_Renderer* renderer,
                 struct rl_font const* font);
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
rl_view_update_ribbon(struct rl_view const* view, struct rl_ribbon* ribbon)
{
  if (view->update_ribbon == NULL) {
    return;
  }

  view->update_ribbon(view->state, ribbon);
}

static inline bool
rl_update_view(struct rl_view* view,
               struct inpt_state const* istate,
               struct rl_command* out)
{
  if (view->update == NULL) {
    return false;
  }

  return view->update(view->state, istate, out);
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
               struct rl_font const* font)
{
  if (view->render == NULL) {
    return;
  }

  view->render(view->state, renderer, font);
}

#endif // GINC_ROGUELIKE_VIEW_H
