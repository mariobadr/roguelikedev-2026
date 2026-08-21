/**
 * @file bsp.h
 */
#ifndef GINC_ROGUELIKE_BSP_H
#define GINC_ROGUELIKE_BSP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rand_state;

/** Return the number of nodes needed for a full BSP tree. */
#define RL_BSP_MAX_NODES(max_depth) (((size_t)1 << ((max_depth) + 1)) - 1)

/**
 * How a node is split.
 */
enum rl_bsp_split_axis
{
  RL_BSP_UNUSED,     //< Invalid node
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
 * @return whether node is a leaf.
 */
static inline bool
rl_bsp_node_is_leaf(struct rl_bsp_node const* node)
{
  return node->axis == RL_BSP_SPLIT_NONE;
}

/**
 * A BSP tree.
 */
struct rl_bsp_tree
{
  /** The maximum depth of the tree. */
  int max_depth;
  /** Total number of nodes created. */
  int node_count;
  /** Total number of leaf nodes. */
  int leaf_count;
  /** All possible nodes in the tree. */
  struct rl_bsp_node* nodes;
};

/**
 * Parameters that influence how a BSP tree is created.
 */
struct rl_bsp_policy
{
  /** The minimum width of a rect in the tree. */
  int min_width;
  /** The minimum height of a rect in the tree. */
  int min_height;
  /** The maximum width:height ratio. */
  double max_wh_ratio;
  /** The maximum height:width ratio. */
  double max_hw_ratio;
};

/**
 * Initializes a tree with the given root region.
 *
 * All nodes start as leaves (i.e., RL_BSP_SPLIT_NONE) until split.
 *
 * @param tree      The tree to initialize.
 * @param max_depth The maximum depth of the tree.
 * @param rect      The root region that will be split.
 *
 * @return whether initialization was successful
 */
bool
rl_bsp_tree_init(struct rl_bsp_tree* tree, int max_depth, SDL_Rect rect);

/**
 * Free up resources used by tree.
 *
 * @param tree The tree to free.
 */
void
rl_bsp_tree_free(struct rl_bsp_tree* tree);

/**
 * Split the node at index up to depth levels.
 *
 * @param tree    The tree being updated.
 * @param index   The index of the node of the tree.
 * @param rng     The random number generator.
 * @param depth   The current depth.
 * @param policy  The policy impacting generation.
 */
void
rl_bsp_split(struct rl_bsp_tree* tree,
             int index,
             struct rand_state* rng,
             int depth,
             struct rl_bsp_policy const* policy);

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

#endif // GINC_ROGUELIKE_BSP_H
