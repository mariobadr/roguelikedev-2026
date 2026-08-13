/**
 * @file bsp.h
 */
#ifndef GINC_ROGUELIKE_BSP_H
#define GINC_ROGUELIKE_BSP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rand_state;

/** The probability to avoid a split (for larger rooms). */
#define RL_BSP_STOP_CHANCE 35
/** The minimum size of a rect in the BSP tree. */
#define RL_BSP_MIN_SIZE 5
/** The maximum depth of the BSP tree. */
#define RL_BSP_MAX_DEPTH 7
/** The number of nodes needed for a BSP of maximum depth. */
#define RL_BSP_CAPACITY ((1 << (RL_BSP_MAX_DEPTH + 1)) - 1)

enum rl_bsp_split_axis
{
  RL_BSP_SPLIT_NONE, //< Leaf node
  RL_BSP_SPLIT_X,    //< Node split on x-axis
  RL_BSP_SPLIT_Y     //< Node split on y-axis
};

/**
 * A node in a BSP tree.
 */
struct rl_bsp_node
{
  /** The region of space this node occupies. */
  SDL_Rect rect;
  /** How this node was split, if at all. */
  enum rl_bsp_split_axis axis;
};

/**
 * A BSP tree.
 */
struct rl_bsp_tree
{
  /** All nodes in the tree. */
  struct rl_bsp_node nodes[RL_BSP_CAPACITY];
};

/**
 * @return the index of the left child of the node at index.
 */
static inline int
rl_bsp_left_of(int index)
{
  return 2 * index + 1;
}

/**
 * @return the index of the right child of the node at index.
 */
static inline int
rl_bsp_right_of(int index)
{
  return 2 * index + 2;
}

/**
 * @return the index of the parent of the node at index.
 */
static inline int
rl_bsp_parent_of(int index)
{
  return (index - 1) / 2;
}

/**
 * Initializes a tree with the given root region.
 *
 * All nodes start as leaves (i.e., RL_BSP_SPLIT_NONE) until split.
 *
 * @param tree The tree to initialize.
 * @param rect The root region that will be split.
 */
void
rl_bsp_tree_init(struct rl_bsp_tree* tree, SDL_Rect rect);

/**
 * Split the node at index up to depth levels.
 *
 * @param tree  The tree being updated.
 * @param index The index of the node of the tree.
 * @param rng   The random number generator.
 * @param depth The current depth.
 */
void
rl_bsp_split(struct rl_bsp_tree* tree,
             int index,
             struct rand_state* rng,
             int depth);

#endif // GINC_ROGUELIKE_BSP_H
