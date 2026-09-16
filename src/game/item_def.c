#include "item_def.h"

static struct rl_item_def const RL_ITEM_DEFS[] = {
  [RL_ITEM_POTION_HEALTH_MINOR] = {
    .class = RL_ITEM_CLASS_POTION,
    .effect = RL_ITEM_EFFECT_HEAL,
    .target = RL_ITEM_TARGET_NONE,
    .name = "Minor Health Potion",
    .power = 10, },
  [RL_ITEM_SCROLL_FIREBALL] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .effect = RL_ITEM_EFFECT_DAMAGE_AREA,
    .target = RL_ITEM_TARGET_TILE,
    .name = "Scroll of Minor Fireball",
    .power = 8,
    .area_radius = 3,
  },
  [RL_ITEM_SCROLL_LIGHTNING] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .effect = RL_ITEM_EFFECT_DAMAGE_NEAREST,
    .target = RL_ITEM_TARGET_CLOSEST,
    .name = "Scroll of Minor Lightning",
    .power = 12,
  }
};

struct rl_item_def const*
rl_get_item_def(enum rl_item_type type)
{
  return &RL_ITEM_DEFS[type];
}
