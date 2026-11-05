/* parity/trap: machine-readable native explicit trap fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
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

static int test_explicit_trap_fixture(void *state)
{
	struct chunk *c = cave_new(7, 9);
	struct trap_kind *kind = lookup_trap("web");
	struct trap *trap;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int x, y, i;
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
	if (!kind) {
		cave_free(c);
		return 1;
	}
	Rand_quick = false;
	Rand_state_init(7);
	place_trap(c, loc(1, 1), kind->tidx, 1);
	trap = square_trap(c, loc(1, 1));
	if (!trap) {
		cave_free(c);
		return 1;
	}
	printf("NATIVE_PARITY_TRAP {\"seed\":7,\"depth\":1,\"requested\":1,\"failed\":0,\"placed\":1,\"rng_state_i\":%u,\"changed\":[{\"x\":1,\"y\":1,\"trap_id\":%d,\"power\":%d}]}\n",
		state_i, trap->t_idx, trap->power);
	eq(trap->t_idx, kind->tidx);
	cave_free(c);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

const char *suite_name = "parity/trap";
struct test tests[] = {
	{ "explicit-trap-fixture", test_explicit_trap_fixture },
	{ NULL, NULL },
};
