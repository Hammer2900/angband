/* parity/object: machine-readable explicit native object/gold fixture */

#include "unit-test.h"
#include "test-utils.h"
#include "generate.h"
#include "init.h"
#include "obj-pile.h"
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

static int test_explicit_object_gold_fixture(void *state)
{
	struct chunk *c = t_build_arena(9, 11);
	struct object *object, *gold;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int i;
	(void)state;

	for (i = 0; i < RAND_DEG; ++i) saved_rng[i] = STATE[i];
	c->depth = 1;
	Rand_quick = false;
	Rand_state_init(7);
	place_object(c, loc(2, 2), 1, false, false, ORIGIN_FLOOR, 0);
	place_gold(c, loc(3, 2), 1, ORIGIN_FLOOR);
	object = square_object(c, loc(2, 2));
	gold = square_object(c, loc(3, 2));
	printf("NATIVE_PARITY_OBJECT {\"seed\":7,\"depth\":1,\"object_placed\":%s,\"gold_placed\":%s,\"object_id\":%d,\"gold_id\":%d,\"object_x\":2,\"object_y\":2,\"gold_x\":3,\"gold_y\":2,\"rng_state_i\":%u}\n",
		object ? "true" : "false", gold ? "true" : "false",
		object ? object->oidx : -1, gold ? gold->oidx : -1, state_i);
	notnull(object);
	notnull(gold);
	cave_free(c);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

static int test_allocated_object_gold_fixture(void *state)
{
	struct chunk *c = t_build_arena(9, 11);
	struct object *object;
	struct object *gold;
	int object_count = 0, gold_count = 0;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int i;
	(void)state;

	for (i = 0; i < RAND_DEG; ++i) saved_rng[i] = STATE[i];
	c->depth = 1;
	Rand_quick = false;
	Rand_state_init(7);
	eq(alloc_objects(c, SET_BOTH, TYP_OBJECT, 1, 1, ORIGIN_FLOOR), 0);
	eq(alloc_objects(c, SET_BOTH, TYP_GOLD, 1, 1, ORIGIN_FLOOR), 0);
	for (int y = 1; y < c->height - 1; ++y) {
		for (int x = 1; x < c->width - 1; ++x) {
			struct object *current = square_object(c, loc(x, y));
			for (; current; current = current->next) {
				if (current->tval == TV_GOLD) ++gold_count;
				else ++object_count;
			}
		}
	}
	object = square_object(c, loc(1, 1));
	gold = square_object(c, loc(2, 1));
	printf("NATIVE_PARITY_OBJECT_ALLOC {\"seed\":7,\"depth\":1,\"object_count\":%d,\"gold_count\":%d,\"object_on_first_cell\":%s,\"gold_on_second_cell\":%s,\"rng_state_i\":%u}\n",
		object_count, gold_count, object ? "true" : "false",
		gold ? "true" : "false", state_i);
	cave_free(c);
	for (i = 0; i < RAND_DEG; ++i) STATE[i] = saved_rng[i];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

const char *suite_name = "parity/object";
struct test tests[] = {
	{ "explicit-object-gold-fixture", test_explicit_object_gold_fixture },
	{ "allocated-object-gold-fixture", test_allocated_object_gold_fixture },
	{ NULL, NULL },
};
