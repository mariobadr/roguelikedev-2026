# Contributing

To build the project, see [BUILDING.md](BUILDING.md).
Formatting is handled by [clang-format](https://clang.llvm.org/docs/ClangFormat.html) using the `.clang-format` file in the root of the repository.

## Documentation guidelines

This project uses Doxygen-style comments with 
British spelling (e.g., colour, initialise, summarise).
In general, a "docstring" should describe what something is and how to use it, not how it is implemented.
The implementation may change over time, leaving the docstring stale.

### Header files

Some general rules for header files are:

- Start each header with an `@file` block.
- Repeat the include guard as a comment on the closing `#endif`.
- Group forward declarations under `// forward declarations`
- Group types from other libraries (such as SDL) under `// external forward declarations`.

### Documenting `struct`s

Give the struct a multi-line docstring, and put a single-line docstring above each struct member.
For example,

```c
/**
 * A texture holding a grid of equally sized tiles.
 */
struct gfx_tileset
{
  /** The width of one tile, in pixels. */
  int tile_width;
  /** The height of one tile, in pixels. */
  int tile_height;
};
```

To avoid repeated checks throughout the code (see [Error handling conventions](#error-handling-conventions)), list the struct's representation invariants.
For that, you should use `@invariant` in its docstring.
State each invariant once, on the struct that owns every member it mentions.
For example,

```c
/**
 * One level of the dungeon.
 *
 * @invariant the border of map is unwalkable.
 */
struct rl_level
```

### Documenting `enum`s

Give the enum a docstring, and put a short trailing comment on each member that needs one.
Comments should be lowercase.
For example,

```c
/**
 * The different targeting requirements of an item.
 */
enum rl_item_target
{
  RL_ITEM_TARGET_NONE,    //< no target
  RL_ITEM_TARGET_TILE,    //< a chosen tile
  RL_ITEM_TARGET_CLOSEST, //< closest visible enemy
};
```

### Documenting functions

Start with a description in the imperative form (e.g., "Free" not "Frees").
But avoid documenting the obvious.
For example, a brief description may be omitted when `@return` says everything.
Similarly, use `@param` for a caller obligation that is not obvious.

In terms of formatting, follow the description with a blank line, then `@param`, then `@return`, with a blank line between `@param` and `@return`.
End the `@param` and `@return` lines with a full stop.

Finally, for booleans, prefer using "whether" rather than stating both "true" and "false".
This may not always be possible.

Some examples include:

```c
/**
 * Add a new actor of the given type and level to the world.
 *
 * @param level must be at least 1.
 *
 * @return the new actor's handle, or an invalid handle if allocation failed.
 */
handle(rl_actor)
rl_create_actor(struct rl_world* world, enum rl_actor_type type, int level);

/**
 * @return whether an actor can walk on this tile.
 */
bool
rl_is_walkable(enum rl_tile tile);
```

## Error handling conventions

There are many situations where things may fail, especially with respect to memory allocation and container growth.
You should handle allocation failure where a failure can already be reported (e.g., when creating the world, generating a level, or loading a save; these return `bool` and clean up after themselves).

Growing a container during a turn (e.g., pushing an event) is treated as infallible.
In other words, it will segfault.
But if it did, then the container couldn't grow, which means we're out of memory (somehow in our very simple little roguelike).
Running out of memory mid-turn is not recoverable, so it is not handled.

You should also focus on validating untrusted input once, where it enters the game.
For example:
- Commands built by the client are validated when they are applied.
- Save files are validated when they are loaded; a save that breaks an invariant of the world is rejected as corrupt.

Past those boundaries, code may reasonably assume the world's invariants hold.
Do not check for states that cannot occur (e.g., a stale handle in a level's actor list).
If something must be explicit, consider documenting a caller obligation with `@param` (see [Documenting functions](#documenting-functions)).
Or, for debug checks, use `SDL_assert` for an invariant violation that would otherwise be silent or hard to trace.
Otherwise, write no check at all.

Note that file I/O is always handled.
Reading or writing a save can fail for reasons outside the game, and the failure is reported (via `SDL_Log`).
This makes debugging easier.
