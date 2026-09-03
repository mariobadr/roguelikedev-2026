#include "item.h"

static struct rl_item_def const RL_ITEM_DEFS[] = {
  [RL_ITEM_POTION_HEALTH_MINOR] = {
    .class = RL_ITEM_CLASS_POTION,
    .name = "Health Potion",
    .effect = RL_ITEM_EFFECT_HEAL,
    .power = 10, },
};

struct rl_item_def const*
rl_get_item_def(enum rl_item_type type)
{
  return &RL_ITEM_DEFS[type];
}
