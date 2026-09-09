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

#include "client/cell.h"
#include "client/client.h"
#include "client/ui.h"
#include "input/input.h"
#include "input/input_event.h"

struct application
{
  /* Platform state*/
  SDL_Window* window;
  SDL_Renderer* renderer;

  /* Game loop state */
  Uint64 freq;
  Uint64 last;

  /* Input state */
  struct inpt_state input;

  /* Client state */
  struct rl_client client;
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

  rl_free_client(&app->client);
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
        renderer,
        (RL_UI_WIDTH)*rl_cell_width(),
        (RL_UI_HEIGHT)*rl_cell_height(),
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
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
    SDL_CreateWindow("A Roguelike", 1600, 900, SDL_WINDOW_RESIZABLE);
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

  if (!rl_init_client(&app->client, app->renderer)) {
    destroy_application(app);
    return NULL;
  }

  inpt_init_state(&app->input);

  app->freq = SDL_GetPerformanceFrequency();
  app->last = SDL_GetPerformanceCounter();

  return app;
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
  inpt_handle_event(&app->input, event);

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

  // avoid very large delta times
  delta = SDL_min(delta, app->freq / 4);
  float const frame_dt = (float)delta / (float)app->freq;

  if (!rl_update_client(&app->client, &app->input, frame_dt)) {
    // the client is done, so the app is done
    return SDL_APP_SUCCESS;
  }

  rl_render_client(&app->client, app->renderer);
  SDL_RenderPresent(app->renderer);

  // reset transient input for next frame
  inpt_reset_state(&app->input);

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
