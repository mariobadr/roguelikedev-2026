/**
 * @file palette.h
 */
#ifndef GINC_ROGUELIKE_PALETTE_H
#define GINC_ROGUELIKE_PALETTE_H

#include <SDL3/SDL_pixels.h>

static const SDL_FColor RL_COLOUR_NONE = { 0.00f, 0.00f, 0.00f, 0.00f };
static const SDL_FColor RL_COLOUR_WHITE = { 1.00f, 1.00f, 1.00f, 1.00f };

// shades of slate gray
static const SDL_FColor RL_COLOUR_SLATE_0 = { 0.03f, 0.03f, 0.05f, 1.00f };
static const SDL_FColor RL_COLOUR_SLATE_1 = { 0.08f, 0.08f, 0.13f, 1.00f };
static const SDL_FColor RL_COLOUR_SLATE_2 = { 0.15f, 0.15f, 0.22f, 1.00f };
static const SDL_FColor RL_COLOUR_SLATE_3 = { 0.28f, 0.28f, 0.38f, 1.00f };
static const SDL_FColor RL_COLOUR_SLATE_4 = { 0.50f, 0.50f, 0.63f, 1.00f };
static const SDL_FColor RL_COLOUR_SLATE_5 = { 0.78f, 0.78f, 0.88f, 1.00f };

#endif // GINC_ROGUELIKE_PALETTE_H