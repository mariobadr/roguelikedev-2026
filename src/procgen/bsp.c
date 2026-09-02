#include "bsp.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "rand.h"

bool
rl_bsp_tree_init(struct rl_bsp_tree* tree, int max_depth, SDL_Rect rect)
{
  int const node_count = RL_BSP_MAX_NODES(max_depth);
  if (!array_alloc(&tree->nodes, node_count)) {
    SDL_Log("array_alloc failed: %s", SDL_GetError());
    return false;
  }

  tree->nodes.len = node_count;
  tree->max_depth = max_depth;

  // set up the root node
  array_at(&tree->nodes, 0)->rect = rect;
  tree->leaf_count = 1; // root is initially a leaf

  return true;
}

void
rl_bsp_tree_free(struct rl_bsp_tree* tree)
{
  array_free(&tree->nodes);
}

void
rl_bsp_split(struct rl_bsp_tree* tree,
             int index,
             struct rand_state* rng,
             int depth,
             struct rl_bsp_policy const* policy)
{
  if (depth >= tree->max_depth) {
    // cannot exceed the maximum depth
    return;
  }

  struct rl_bsp_node* node = array_at(&tree->nodes, index);

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
      node->axis = RL_BSP_SPLIT_X;
    } else if (hw_ratio > policy->max_hw_ratio) {
      node->axis = RL_BSP_SPLIT_Y;
    } else {
      node->axis = rand_next_between(rng, RL_BSP_SPLIT_X, RL_BSP_SPLIT_Y);
    }

  } else if (can_split_x) {
    // Can only split on the x-axis
    node->axis = RL_BSP_SPLIT_X;
  } else {
    // Can only split on the y-axis
    node->axis = RL_BSP_SPLIT_Y;
  }

  // Do the splitting
  struct rl_bsp_node* left = array_at(&tree->nodes, rl_bsp_left_of(index));
  left->axis = RL_BSP_SPLIT_NONE;
  struct rl_bsp_node* right = array_at(&tree->nodes, rl_bsp_right_of(index));
  right->axis = RL_BSP_SPLIT_NONE;

  if (node->axis == RL_BSP_SPLIT_X) {
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
  rl_bsp_split(tree, rl_bsp_left_of(index), rng, depth + 1, policy);
  rl_bsp_split(tree, rl_bsp_right_of(index), rng, depth + 1, policy);
}
