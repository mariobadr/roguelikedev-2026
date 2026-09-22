#include "bsp.h"

#include "core/rand.h"

/**
 * Split the node at index up to depth levels.
 */
static void
split_node(struct sptl_bsp_tree* tree,
           int index,
           struct rand_state* rng,
           int depth,
           struct sptl_bsp_policy const* policy)
{
  if (depth >= tree->max_depth) {
    // cannot exceed the maximum depth
    return;
  }

  struct sptl_bsp_node* node = array_at(&tree->nodes, index);

  // Check whether we can even split the node further
  bool can_split_x = node->rect.w >= 2 * policy->min_width;
  bool can_split_y = node->rect.h >= 2 * policy->min_height;
  if (!can_split_x && !can_split_y) {
    return;
  }

  // Determine how this node will be split
  if (can_split_x && can_split_y) {
    // Can split either way; decide based on width:height ratio
    double wh_ratio = (double)node->rect.w / node->rect.h;
    double hw_ratio = (double)node->rect.h / node->rect.w;

    if (wh_ratio > policy->max_wh_ratio) {
      node->axis = SPTL_BSP_SPLIT_X;
    } else if (hw_ratio > policy->max_hw_ratio) {
      node->axis = SPTL_BSP_SPLIT_Y;
    } else {
      node->axis = rand_next_between(rng, SPTL_BSP_SPLIT_X, SPTL_BSP_SPLIT_Y);
    }

  } else if (can_split_x) {
    // Can only split on the x-axis
    node->axis = SPTL_BSP_SPLIT_X;
  } else {
    // Can only split on the y-axis
    node->axis = SPTL_BSP_SPLIT_Y;
  }

  // Do the splitting
  struct sptl_bsp_node* left = array_at(&tree->nodes, sptl_bsp_left_of(index));
  left->axis = SPTL_BSP_SPLIT_NONE;
  struct sptl_bsp_node* right =
    array_at(&tree->nodes, sptl_bsp_right_of(index));
  right->axis = SPTL_BSP_SPLIT_NONE;

  if (node->axis == SPTL_BSP_SPLIT_X) {
    // splitting here should not impact the height
    int const split =
      (int)rand_next_between(rng,
                             node->rect.x + policy->min_width,
                             node->rect.x + node->rect.w - policy->min_width);

    // left child starts where parent started
    left->rect.x = node->rect.x;
    left->rect.y = node->rect.y;
    left->rect.w = split - node->rect.x;
    left->rect.h = node->rect.h;

    // right child starts at the split line
    right->rect.x = split;
    right->rect.y = node->rect.y;
    right->rect.w = (node->rect.x + node->rect.w) - split;
    right->rect.h = node->rect.h;
  } else {
    // splitting here should not impact the width
    int const split =
      (int)rand_next_between(rng,
                             node->rect.y + policy->min_height,
                             node->rect.y + node->rect.h - policy->min_height);

    // left child starts where parent started
    left->rect.x = node->rect.x;
    left->rect.y = node->rect.y;
    left->rect.w = node->rect.w;
    left->rect.h = split - node->rect.y;

    // right child starts at the split line
    right->rect.x = node->rect.x;
    right->rect.y = split;
    right->rect.w = node->rect.w;
    right->rect.h = (node->rect.y + node->rect.h) - split;
  }

  // Continue splitting
  tree->leaf_count += 1; // this node will no longer be a leaf
  split_node(tree, sptl_bsp_left_of(index), rng, depth + 1, policy);
  split_node(tree, sptl_bsp_right_of(index), rng, depth + 1, policy);
}

bool
sptl_alloc_bsp_tree(struct sptl_bsp_tree* tree, int max_depth, SDL_Rect rect)
{
  int const node_count = SPTL_BSP_MAX_NODES(max_depth);
  if (!array_alloc(&tree->nodes, node_count)) {
    return false;
  }

  tree->nodes.len = node_count;
  tree->max_depth = max_depth;

  // set up the root node
  struct sptl_bsp_node* root = array_at(&tree->nodes, 0);
  root->rect = rect;
  root->axis = SPTL_BSP_SPLIT_NONE;
  tree->leaf_count = 1; // root is initially a leaf

  return true;
}

void
sptl_free_bsp_tree(struct sptl_bsp_tree* tree)
{
  array_free(&tree->nodes);
}

void
sptl_build_bsp_tree(struct sptl_bsp_tree* tree,
                    struct rand_state* rng,
                    struct sptl_bsp_policy const* policy)
{
  split_node(tree, 0, rng, 0, policy);
}
