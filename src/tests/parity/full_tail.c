/* parity/full_tail: machine-readable native generation pipeline fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "player.h"
#include "player-birth.h"
#include "z-rand.h"

#include <stdio.h>

int setup_tests(void **state)
{
	(void)state;
	set_file_paths();
	if (!init_angband()) return 1;
	if (!player_make_simple(NULL, NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}
	return 0;
}

int teardown_tests(void *state)
{
	(void)state;
	cleanup_angband();
	return 0;
}

static int test_full_tail_outer_pipeline(void *state)
{
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int i;
	(void)state;

	for (i = 0; i < RAND_DEG; ++i) saved_rng[i] = STATE[i];
	player->depth = 1;
	Rand_quick = false;
	Rand_state_init(7);
	prepare_next_level(player);
	on_new_level();
	if (!cave) return 1;
	printf("NATIVE_PARITY_FULL_TAIL {\"seed\":7,\"depth\":%d,\"player_x\":%d,\"player_y\":%d,\"monsters\":%d,\"objects\":%d,\"rng_state_i\":%u}\n",
		cave->depth, player->grid.x, player->grid.y, cave->mon_cnt,
		cave->obj_max, state_i);
	eq(cave->depth, player->depth);
	eq(player->grid.x >= 0, true);
	eq(player->grid.y >= 0, true);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

const char *suite_name = "parity/full_tail";
struct test tests[] = {
	{ "full-tail-outer-pipeline", test_full_tail_outer_pipeline },
	{ NULL, NULL },
};
