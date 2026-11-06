/* parity/monster: machine-readable explicit native monster fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "init.h"
#include "mon-make.h"
#include "mon-util.h"
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

static int test_explicit_monster_fixture(void *state)
{
	struct chunk *c = t_build_arena(9, 11);
	struct monster_race *race = lookup_monster("giant white mouse");
	struct monster_group_info info = { 0, 0 };
	struct monster *mon;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int i;
	bool placed;
	(void)state;

	for (i = 0; i < RAND_DEG; ++i) saved_rng[i] = STATE[i];
	if (!race) return 1;
	Rand_quick = false;
	Rand_state_init(7);
	placed = place_new_monster(c, loc(8, 7), race, true, false, info,
		ORIGIN_DROP);
	mon = square_monster(c, loc(8, 7));
	printf("NATIVE_PARITY_MONSTER {\"seed\":7,\"depth\":1,\"selection_mode\":\"explicit_index\",\"placed\":%s,\"race_id\":%u,\"monster_id\":%d,\"x\":%d,\"y\":%d,\"rng_state_i\":%u}\n",
		placed ? "true" : "false", race->ridx,
		mon ? mon->midx : -1, loc(8, 7).x, loc(8, 7).y, state_i);
	eq(placed, true);
	notnull(mon);
	eq(mon->race, race);
	cave_free(c);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

const char *suite_name = "parity/monster";
struct test tests[] = {
	{ "explicit-monster-fixture", test_explicit_monster_fixture },
	{ NULL, NULL },
};
