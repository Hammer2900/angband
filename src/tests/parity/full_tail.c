/* parity/full_tail: machine-readable native generation pipeline fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "player.h"
#include "player-birth.h"
#include "z-rand.h"

#include <stdio.h>

static int generation_start_seen;
static int generation_end_seen;
static bool generation_end_success;

static void generation_event(game_event_type type, game_event_data *data, void *user)
{
	(void)user;
	if (type == EVENT_GEN_LEVEL_START) generation_start_seen = 1;
	if (type == EVENT_GEN_LEVEL_END) {
		generation_end_seen = 1;
		generation_end_success = data->flag;
	}
}

int setup_tests(void **state)
{
	(void)state;
	set_file_paths();
	if (!init_angband()) return 1;
	event_add_handler(EVENT_GEN_LEVEL_START, generation_event, NULL);
	event_add_handler(EVENT_GEN_LEVEL_END, generation_event, NULL);
	if (!player_make_simple(NULL, NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}
	return 0;
}

int teardown_tests(void *state)
{
	(void)state;
	event_remove_handler(EVENT_GEN_LEVEL_START, generation_event, NULL);
	event_remove_handler(EVENT_GEN_LEVEL_END, generation_event, NULL);
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
	generation_start_seen = generation_end_seen = 0;
	generation_end_success = false;
	Rand_state_init(7);
	prepare_next_level(player);
	on_new_level();
	if (!cave) return 1;
	printf("NATIVE_PARITY_FULL_TAIL {\"seed\":7,\"depth\":%d,\"player_x\":%d,\"player_y\":%d,\"monsters\":%d,\"objects\":%d,\"rng_state_i\":%u,\"generation_start_seen\":%s,\"generation_end_seen\":%s,\"generation_end_success\":%s}\n",
		cave->depth, player->grid.x, player->grid.y, cave->mon_cnt,
		cave->obj_max, state_i, generation_start_seen ? "true" : "false",
		generation_end_seen ? "true" : "false",
		generation_end_success ? "true" : "false");
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
