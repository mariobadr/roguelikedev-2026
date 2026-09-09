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
struct rl_font;

/**
 * Unique identifiers for each type of screen.
 */
enum rl_screen_id
{
  RL_SCREEN_GAMEPLAY, //< The main screen
  RL_SCREEN_COUNT,    //< The number of screens
};

/**
 * The possible ways to transition between screens.
 */
enum rl_screen_transition_type
{
  RL_SCREEN_TRANSITION_NONE, //< Do not transition
  RL_SCREEN_TRANSITION_PUSH, //< Push new screen to top
  RL_SCREEN_TRANSITION_POP,  //< Pop top screen
  RL_SCREEN_TRANSITION_SWAP, //< Swap out top screen
};

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
   * A callback that is called once per frame.
   */
  struct rl_screen_transition (*update)(void* data,
                                        struct inpt_state const* istate,
                                        float dt);

  /**
   * A callback that is called once per frame.
   */
  void (*render)(void const* data, SDL_Renderer* renderer);
};

bool
rl_alloc_gameplay_screen(struct rl_screen* screen, struct rl_font const* font);

#endif // GINC_ROGUELIKE_SCREEN_H
