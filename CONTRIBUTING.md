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
