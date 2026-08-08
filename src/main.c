#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_platform.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_video.h>

#include "game.h"
#include "input.h"
#include "resources.h"

struct application
{
  SDL_Window* window;
  SDL_Renderer* renderer;

  struct rl_resources resources;
  struct inpt_state istate;

  Uint64 freq;
  Uint64 last;

  struct rl_game game;
};

/**
 * Logs the SDL version and platform to the application log.
 */
static void
log_system_info(void)
{
  int const version = SDL_GetVersion();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
              "Compiled with SDL %d.%d.%d; linked with SDL %d.%d.%d",
              SDL_MAJOR_VERSION,
              SDL_MINOR_VERSION,
              SDL_MICRO_VERSION,
              SDL_VERSIONNUM_MAJOR(version),
              SDL_VERSIONNUM_MINOR(version),
              SDL_VERSIONNUM_MICRO(version));

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Running on %s", SDL_GetPlatform());
}

/**
 * Destroys an application and releases its resources.
 *
 * @param app The application to destroy.
 */
static void
destroy_application(struct application* app)
{
  if (app == NULL) {
    return;
  }

  rl_destroy_resources(&app->resources);

  SDL_DestroyRenderer(app->renderer);
  SDL_DestroyWindow(app->window);

  SDL_free(app);
}

/**
 * Creates and configures the renderer for a window.
 *
 * @param window The window to render to.
 *
 * @return The new renderer, or NULL on failure.
 */
static SDL_Renderer*
create_renderer(SDL_Window* window)
{
  SDL_DisplayID display = SDL_GetDisplayForWindow(window);
  if (display == 0) {
    SDL_Log("SDL_GetDisplayForWindow failed: %s", SDL_GetError());
    return NULL;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    return NULL;
  }

  if (!SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART)) {
    SDL_Log("SDL_SetDefaultTextureScaleMode failed: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    return NULL;
  }

  if (!SDL_SetRenderLogicalPresentation(
        renderer, 144, 176, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
    SDL_Log("SDL_SetRenderLogicalPresentation failed: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    return NULL;
  }

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
              "SDL renderer created: %s",
              SDL_GetRendererName(renderer));

  return renderer;
}

/**
 * Creates the application.
 *
 * @return The new application, or NULL on failure.
 */
static struct application*
create_application(void)
{
  struct application* app = SDL_calloc(1, sizeof(struct application));
  if (app == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return NULL;
  }

  app->window =
    SDL_CreateWindow("A Roguelike", 256 * 3, 176 * 3, SDL_WINDOW_RESIZABLE);
  if (app->window == NULL) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    destroy_application(app);
    return NULL;
  }

  app->renderer = create_renderer(app->window);
  if (app->renderer == NULL) {
    destroy_application(app);
    return NULL;
  }

  if (!rl_load_resources(&app->resources, app->renderer)) {
    destroy_application(app);
    return NULL;
  }

  inpt_init_state(&app->istate);

  if (!rl_init_game(&app->game, &app->resources)) {
    destroy_application(app);
    return NULL;
  }

  app->freq = SDL_GetPerformanceFrequency();
  app->last = SDL_GetPerformanceCounter();

  return app;
}

/**
 * Update a keyboard key in istate based on event.
 *
 * @param istate the input state to update
 * @param event  the keyboard event to process
 */
static void
handle_key_event(struct inpt_state* istate, SDL_KeyboardEvent* event)
{
  inpt_set_button(&istate->keys[event->scancode], event->down);
}

/**
 * Update the mouse's position in istate based on event.
 *
 * @param istate the input state to update
 * @param event  the mouse motion event to process
 */
static void
handle_mouse_motion_event(struct inpt_state* istate,
                          SDL_MouseMotionEvent* event)
{
  istate->mouse.position.x = event->x;
  istate->mouse.position.y = event->y;
}

/**
 * Update a mouse button in istate based on event.
 *
 * @param istate the input state to update
 * @param event  the mouse button event to process
 */
static void
handle_mouse_button_event(struct inpt_state* istate,
                          SDL_MouseButtonEvent* event)
{
  inpt_set_button(&istate->mouse.buttons[event->button], event->down);
}

/**
 * Update istate when event corresponds to user input.
 *
 * @param istate the input state to update
 * @param event the event to process
 *
 * @return whether istate was updated
 */
static bool
handle_input_event(struct inpt_state* istate, SDL_Event* event)
{
  SDL_assert(istate != NULL && event != NULL);

  switch (event->type) {
    case SDL_EVENT_MOUSE_MOTION:
      handle_mouse_motion_event(istate, &event->motion);
      return true;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
      handle_mouse_button_event(istate, &event->button);
      return true;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
      handle_key_event(istate, &event->key);
      return true;

    default:
      break;
  }

  return false;
}

SDL_AppResult
SDL_AppInit(void** appstate, int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  log_system_info();

  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_InitSubSystem failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    SDL_Log("SDL_InitSubSystem failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  *appstate = create_application();
  if (*appstate == NULL) {
    return SDL_APP_FAILURE;
  }

  // target 60 Hz for game logic updates
  SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent(void* appstate, SDL_Event* event)
{
  struct application* app = (struct application*)appstate;
  SDL_assert(app != NULL);

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  SDL_ConvertEventToRenderCoordinates(app->renderer, event);
  handle_input_event(&app->istate, event);

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppIterate(void* appstate)
{
  struct application* app = (struct application*)appstate;
  SDL_assert(app != NULL);

  Uint64 now = SDL_GetPerformanceCounter();
  Uint64 delta = now - app->last;
  app->last = now;

  if (!rl_handle_input(&app->game, &app->istate)) {
    return SDL_APP_SUCCESS;
  }

  // avoid very large delta times
  delta = SDL_min(delta, app->freq / 4);
  float const frame_dt = (float)delta / (float)app->freq;

  // update game state
  rl_update_game(&app->game, frame_dt);

  // render game state
  rl_render_game(&app->game, app->renderer);
  SDL_RenderPresent(app->renderer);

  // reset transient input for next frame
  inpt_reset_state(&app->istate);

  return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void* appstate, SDL_AppResult result)
{
  SDL_Log("Quitting application (result: %d)", result);

  struct application* app = (struct application*)appstate;
  destroy_application(app);

  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
