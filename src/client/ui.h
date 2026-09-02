/**
 * @file ui.h
 *
 * Roughly:
 *
 * ┌──────────────────────────────┬───────────────┐
 * │          top panel           │               │
 * ├──────────────────────────────┤               │
 * │                              │  right panel  │
 * │             map              │               │
 * │                              │               │
 * ├──────────────────────────────┤               │
 * │         bottom panel         │               │
 * └──────────────────────────────┴───────────────┘
 */
#ifndef GINC_ROGUELIKE_UI_H
#define GINC_ROGUELIKE_UI_H

/** The width of the entire UI. */
#define RL_UI_WIDTH 80
/** The height of the entire UI. */
#define RL_UI_HEIGHT 45

/** The margin (in tiles) between UI panels. */
#define RL_UI_MARGIN 1

/** The starting x-coordinate of the right panel. */
#define RL_UI_RPANEL_X (RL_UI_MAP_X + RL_UI_MAP_WIDTH + RL_UI_MARGIN)
/** The starting y-coordinate of the right panel. */
#define RL_UI_RPANEL_Y 0
/** The width of the right panel. */
#define RL_UI_RPANEL_WIDTH 15

/** The starting x-coordinate of the top panel. */
#define RL_UI_TPANEL_X 0
/** The starting y-coordinate of the top panel. */
#define RL_UI_TPANEL_Y 0
/** The width of the top panel. */
#define RL_UI_TPANEL_WIDTH (RL_UI_MAP_WIDTH)
/** The height of the top panel. */
#define RL_UI_TPANEL_HEIGHT 1

/** The starting x-coordinate of the bottom panel. */
#define RL_UI_BPANEL_X 0
/** The starting y-coordinate of the bottom panel. */
#define RL_UI_BPANEL_Y (RL_UI_MAP_Y + RL_UI_MAP_HEIGHT + RL_UI_MARGIN)
/** The width of the bottom panel. */
#define RL_UI_BPANEL_WIDTH (RL_UI_MAP_WIDTH)
/** The height of the bottom panel. */
#define RL_UI_BPANEL_HEIGHT 7

/** The starting x-coordinate of the map. */
#define RL_UI_MAP_X 0
/** The starting y-coordinate of the map. */
#define RL_UI_MAP_Y (RL_UI_TPANEL_HEIGHT + RL_UI_MARGIN)
/** The width of the map view. */
#define RL_UI_MAP_WIDTH (RL_UI_WIDTH - RL_UI_RPANEL_WIDTH - RL_UI_MARGIN)
/** The height of the map view. */
#define RL_UI_MAP_HEIGHT                                                       \
  (RL_UI_HEIGHT - RL_UI_TPANEL_HEIGHT - RL_UI_BPANEL_HEIGHT - 2 * RL_UI_MARGIN)

#endif // GINC_ROGUELIKE_UI_H
