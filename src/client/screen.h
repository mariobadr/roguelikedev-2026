/**
 * @file screen.h
 */
#ifndef GINC_ROGUELIKE_SCREEN_H
#define GINC_ROGUELIKE_SCREEN_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;
struct rl_ribbon_content;

/**
 * Unique identifiers for each type of screen.
 */
enum rl_screen_id
{
  RL_SCREEN_MAIN_MENU,  //< the title/main menu screen
  RL_SCREEN_SAVE_FILES, //< browse saved runs
  RL_SCREEN_GAMEPLAY,   //< the main screen
  RL_SCREEN_GAME_OVER,  //< the rogue has died
  RL_SCREEN_VICTORY,    //< the rogue has slain the dragon
  RL_SCREEN_COUNT,      //< the number of screens
};

/**
 * The possible ways to transition between screens.
 */
enum rl_screen_transition_type
{
  RL_SCREEN_TRANSITION_NONE, //< do not transition
  RL_SCREEN_TRANSITION_PUSH, //< push new screen to top
  RL_SCREEN_TRANSITION_POP,  //< pop top screen
  RL_SCREEN_TRANSITION_SWAP, //< swap out top screen
};

/**
 * A request to change the active screen.
 */
struct rl_screen_transition
{
  /** The type of this transition. */
  enum rl_screen_transition_type type;
  /** Valid for PUSH and SWAP. */
  enum rl_screen_id target;
};

/**
 * A collection of callbacks, and their underlying state, that represent a
 * screen.
 *
 * A screen may be exited and entered multiple times over the lifetime of the
 * application.
 */
struct rl_screen
{
  /**
   * A pointer passed to all the screen state callbacks.
   */
  void* state;

  /**
   * A callback that is called before the screen is destroyed.
   */
  void (*free)(void* data);

  /**
   * A callback that is called before the screen is displayed to the user.
   */
  void (*enter)(void* data);

  /**
   * A callback that is called before the screen is exited.
   */
  void (*exit)(void* data);

  /**
   * An optional callback that fills in the ribbon text for the screen's current
   * state. The content starts out empty.
   */
  void (*describe_ribbon)(void const* data, struct rl_ribbon_content* content);

  /**
   * A callback that is called once per frame to update the screen. It returns
   * the transition to apply.
   */
  struct rl_screen_transition (*update)(void* data,
                                        struct inpt_state const* istate,
                                        float dt);

  /**
   * A callback that is called once per frame to draw the screen.
   */
  void (*render)(void const* data, SDL_Renderer* renderer);
};

/**
 * Ask the screen to fill in content; does nothing if it has no callback.
 */
static inline void
rl_screen_describe_ribbon(struct rl_screen const* screen,
                          struct rl_ribbon_content* content)
{
  if (screen->describe_ribbon == NULL) {
    return;
  }

  screen->describe_ribbon(screen->state, content);
}

#endif // GINC_ROGUELIKE_SCREEN_H
