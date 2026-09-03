/**
 * @file text.h
 */
#ifndef GINC_ROGUELIKE_TEXT_H
#define GINC_ROGUELIKE_TEXT_H

enum rl_text_style
{
  RL_TEXT_NORMAL,
  RL_TEXT_PLAYER,
  RL_TEXT_ENEMY,
};

struct rl_text_run
{
  enum rl_text_style style;
  char text[64];
};

struct rl_text
{
  struct rl_text_run runs[8];
  int run_count;
};

#endif // GINC_ROGUELIKE_TEXT_H