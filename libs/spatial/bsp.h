/**
 * @file bsp.h
 */
#ifndef GINC_SPATIAL_BSP_H
#define GINC_SPATIAL_BSP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/array.h"

// forward declarations
struct rand_state;

/**
 * @return the number of nodes needed for a full BSP tree of max_depth.
 */
#define SPTL_BSP_MAX_NODES(max_depth) (((size_t)1 << ((max_depth) + 1)) - 1)

/**
 * How a node is split.
 */
enum sptl_bsp_split_axis
{
  SPTL_BSP_UNUSED,     //< invalid node
  SPTL_BSP_SPLIT_NONE, //< leaf node
  SPTL_BSP_SPLIT_X,    //< node split on x-axis
  SPTL_BSP_SPLIT_Y     //< node split on y-axis
};

/**
 * A node in a BSP tree.
 */
struct sptl_bsp_node
{
  /** The region of space this node occupies. */
  SDL_Rect rect;
  /** How this node was split, if at all. */
  enum sptl_bsp_split_axis axis;
};

/**
 * An array of nodes.
 */
array_define_as(struct sptl_bsp_node, sptl_bsp_node);

/**
 * A BSP tree.
 */
struct sptl_bsp_tree
{
  /** The maximum depth of the tree. */
  int max_depth;
  /** Total number of leaf nodes. */
  int leaf_count;
  /** All possible nodes in the tree. */
  array(sptl_bsp_node) nodes;
};

/**
 * Parameters that influence how a BSP tree is created.
 */
struct sptl_bsp_policy
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
 * Initialise a tree with the given root region.
 *
 * The root starts as a leaf (i.e., SPTL_BSP_SPLIT_NONE). The remaining nodes
 * are SPTL_BSP_UNUSED until a split reaches them.
 *
 * @param tree      The tree to initialise.
 * @param max_depth The maximum depth of the tree.
 * @param rect      The root region that will be split.
 *
 * @return whether initialisation was successful.
 */
bool
sptl_alloc_bsp_tree(struct sptl_bsp_tree* tree, int max_depth, SDL_Rect rect);

/**
 * Free up resources used by tree.
 *
 * @param tree The tree to free.
 */
void
sptl_free_bsp_tree(struct sptl_bsp_tree* tree);

/**
 * Split the tree, from its root, until the policy or its maximum depth stops
 * it.
 *
 * @param tree    The tree being updated.
 * @param rng     The random number generator.
 * @param policy  The policy impacting generation.
 */
void
sptl_build_bsp_tree(struct sptl_bsp_tree* tree,
                    struct rand_state* rng,
                    struct sptl_bsp_policy const* policy);

/**
 * @return whether node is a leaf.
 */
static inline bool
sptl_bsp_node_is_leaf(struct sptl_bsp_node const* node)
{
  return node->axis == SPTL_BSP_SPLIT_NONE;
}

/**
 * @return the index of the left child of the node at index.
 */
static inline int
sptl_bsp_left_of(int index)
{
  return 2 * index + 1;
}

/**
 * @return the index of the right child of the node at index.
 */
static inline int
sptl_bsp_right_of(int index)
{
  return 2 * index + 2;
}

/**
 * @return the index of the parent of the node at index.
 */
static inline int
sptl_bsp_parent_of(int index)
{
  return (index - 1) / 2;
}

#endif // GINC_SPATIAL_BSP_H
