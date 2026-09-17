#include "run.h"

#include "client/save.h"

struct rl_run_result
rl_start_run(struct rl_run* run, int width, int height, Uint64 seed)
{
  struct rl_game tmp = { 0 };

  if (!rl_new_game(&tmp, width, height, seed)) {
    return (struct rl_run_result){ .type = RL_RUN_RESULT_GAME_ERROR };
  }

  struct rl_save_id id = rl_save_id_invalid();
  enum rl_save_result const save_result = rl_create_save(&tmp, &id);
  if (save_result != RL_SAVE_OK) {
    rl_free_game(&tmp);
    return (struct rl_run_result){ .type = RL_RUN_RESULT_SAVE_ERROR,
                                   .save_error = save_result };
  }

  rl_free_game(&run->game);
  run->game = tmp;
  run->save_id = id;

  return (struct rl_run_result){ .type = RL_RUN_RESULT_OK };
}

struct rl_run_result
rl_resume_run(struct rl_run* run, struct rl_save_id id)
{
  struct rl_game tmp = { 0 };

  enum rl_save_result const load_result = rl_load_game(id, &tmp);
  if (load_result != RL_SAVE_OK) {
    return (struct rl_run_result){ .type = RL_RUN_RESULT_SAVE_ERROR,
                                   .save_error = load_result };
  }

  rl_free_game(&run->game);
  run->game = tmp;
  run->save_id = id;

  return (struct rl_run_result){ .type = RL_RUN_RESULT_OK };
}

enum rl_save_result
rl_save_run(struct rl_run const* run)
{
  enum rl_save_result result = { 0 };

  struct rl_actor const* rogue =
    rl_borrow_actor(&run->game.world, rl_get_rogue(&run->game.world));
  if (!rl_actor_is_alive(rogue)) {
    result = rl_finish_save(run->save_id, RL_RUN_DEAD);
  }

  if (result == RL_SAVE_OK) {
    result = rl_save_game(run->save_id, &run->game);
  }

  return result;
}

void
rl_free_run(struct rl_run* run)
{
  if (run == NULL) {
    return;
  }

  rl_free_game(&run->game);
  run->save_id = rl_save_id_invalid();
}
