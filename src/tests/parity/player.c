/* parity/player: machine-readable native player-start fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "generate.h"
#include "init.h"
#include "player-birth.h"
#include "trap.h"
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

static int test_player_start_fixture(void *state)
{
	struct chunk *c = cave_new(7, 9);
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int x, y, i;
	bool placed;
	(void)state;

	for (i = 0; i < RAND_DEG; ++i) saved_rng[i] = STATE[i];
	c->depth = 1;
	for (y = 0; y < c->height; ++y) {
		for (x = 0; x < c->width; ++x) {
			square_set_feat(c, loc(x, y),
				(x == 0 || y == 0 || x == c->width - 1 ||
				 y == c->height - 1) ? FEAT_PERM : FEAT_FLOOR);
		}
	}
	player->depth = 1;
	player->upkeep->create_down_stair = true;
	player->upkeep->create_up_stair = false;
	player->opts.opt[OPT_birth_connect_stairs] = true;
	Rand_quick = false;
	Rand_state_init(7);
	placed = new_player_spot(c, player);
	printf("NATIVE_PARITY_PLAYER {\"seed\":7,\"depth\":1,\"status\":\"%s\",\"x\":%d,\"y\":%d,\"feature\":%d,\"player_installed\":%s,\"rng_state_i\":%u}\n",
		placed ? "completed" : "failed", player->grid.x, player->grid.y,
		square(c, player->grid)->feat,
		placed ? "true" : "false", state_i);
	eq(placed, true);
	eq(square(c, player->grid)->feat, FEAT_MORE);
	cave_free(c);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

const char *suite_name = "parity/player";
struct test tests[] = {
	{ "player-start-fixture", test_player_start_fixture },
	{ NULL, NULL },
};
