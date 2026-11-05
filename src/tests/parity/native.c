/* parity/native: machine-readable native RNG fixture */

#include "unit-test.h"
#include "unit-test-data.h"
#include "z-rand.h"
#include "game-world.h"
#include "init.h"
#include "mon-attack.h"
#include "mon-make.h"
#include "mon-group.h"
#include "monster.h"
#include "cave.h"
#include "target.h"
#include "generate.h"
#include "ui-input.h"
#include "player-birth.h"
#include "player-quest.h"
#include "trap.h"

#include <stdio.h>

static int test_rng_fixture(void *state)
{
	int i;
	int values[8];
	(void)state;

	Rand_quick = false;
	Rand_state_init(7);
	for (i = 0; i < 8; i++) values[i] = randint0(100);
	printf("NATIVE_PARITY_RNG {\"seed\":7,\"randint0_100\":[");
	for (i = 0; i < 8; i++) printf("%s%d", i ? "," : "", values[i]);
	printf("]}\n");
	eq(values[0], 7);
	eq(values[1], 28);
	eq(values[2], 52);
	eq(values[3], 54);
	eq(values[4], 73);
	eq(values[5], 25);
	eq(values[6], 86);
	eq(values[7], 13);
	ok;
}

int setup_tests(void **state)
{
	struct monster *monster = mem_zalloc(sizeof(*monster));
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->move_energy = 100;
	z_info->mon_blows_max = 2;
	z_info->pack_size = 23;
	z_info->quiver_size = 10;
	z_info->quest_max = 1;
	z_info->level_monster_max = 32;
	z_info->max_range = 20;
	quests = &test_quest;
	textui_input_init();
	projections = test_projections;
	l_list = &test_lore;
	monster->race = &test_r_human;
	r_info = &test_r_human;
	*state = monster;
	return 0;
}

int teardown_tests(void *state)
{
	mem_free(state);
	mem_free(z_info);
	z_info = NULL;
	return 0;
}

static int test_attack_fixture(void *state)
{
	struct monster *monster = state;
	struct player *player = &test_player;
	int before, after;

	cave = &test_cave;
	player->upkeep = &test_player_upkeep;
	monster->race->blow[0].effect = &test_blow_effect_hurt;
	monster->race->blow[0].method = &test_blow_method;
	Rand_quick = false;
	rand_fix(100);
	player->chp = player->mhp;
	before = player->chp;
	make_attack_normal(monster, player);
	after = player->chp;
	printf("NATIVE_PARITY_ATTACK {\"damage\":%d}\n", before - after);
	eq(before - after, monster->race->blow[0].dice.dice);
	ok;
}

static int test_drop_count_fixture(void *state)
{
	struct monster *monster = state;
	int specific = -1;
	int count;
	(void)monster;

	/* RF_DROP_1 is the native guaranteed one-item branch. */
	rf_on(test_r_human.flags, RF_DROP_1);
	Rand_quick = false;
	count = mon_create_drop_count(&test_r_human, false, false, &specific);
	rf_off(test_r_human.flags, RF_DROP_1);
	printf("NATIVE_PARITY_DROP {\"count\":%d,\"specific\":%d}\n",
	       count, specific);
	eq(count, 1);
	eq(specific, 0);
	ok;
}

static int test_energy_fixture(void *state)
{
	(void)state;
	printf("NATIVE_PARITY_ENERGY {\"speed_90\":%d,\"speed_110\":%d,\"speed_120\":%d}\n",
	       turn_energy(90), turn_energy(110), turn_energy(120));
	eq(turn_energy(90), 3);
	eq(turn_energy(110), 10);
	eq(turn_energy(120), 20);
	ok;
}

static int test_compaction_reference_fixture(void *state)
{
	struct chunk *c = cave_new(5, 5);
	struct feature *saved_f_info = f_info;
	struct player *p = &test_player;
	struct monster_group *group = monster_group_new();
	struct mon_group_list_entry *member = mem_zalloc(sizeof(*member));
	struct monster *mon = &c->monsters[2];
	bool tracked_same;
	bool target_same;
	bool target_cleared;
	int tracked_index;
	(void)state;

	cave = c;
	player = p;
	f_info = mem_zalloc(FEAT_MAX * sizeof(*f_info));
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_PROJECT);
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_PASSABLE);
	p->upkeep = &test_player_upkeep;
	p->grid = loc(1, 2);
	square_set_feat(c, loc(1, 2), FEAT_FLOOR);
	square_set_feat(c, loc(2, 2), FEAT_FLOOR);
	c->mon_max = 3;
	c->monster_groups[1] = group;
	group->index = 1;
	group->leader = 2;
	group->member_list = member;
	member->midx = 2;
	mon->race = &test_r_human;
	mon->midx = 2;
	mon->grid = loc(2, 2);
	mflag_on(mon->mflag, MFLAG_VISIBLE);
	mon->group_info[PRIMARY_GROUP].index = 1;
	mon->group_info[PRIMARY_GROUP].role = MON_GROUP_LEADER;
	p->upkeep->health_who = mon;
	if (!target_set_monster(mon)) return 1;

	monster_index_move(c, 2, 1);
	tracked_same = p->upkeep->health_who == &c->monsters[1];
	target_same = target_get_monster() == &c->monsters[1];
	target_cleared = target_get_monster() == NULL && !target_okay();
	tracked_index = p->upkeep->health_who ? p->upkeep->health_who->midx : 0;
	printf("NATIVE_PARITY_COMPACTION {\"health_tracks_same_monster\":%s,\"target_tracks_same_monster\":%s,\"target_cleared\":%s,\"new_index\":%d}\n",
	       tracked_same ? "true" : "false", target_same ? "true" : "false",
	       target_cleared ? "true" : "false", tracked_index);

	target_set_monster(NULL);
	p->upkeep->health_who = NULL;
	mem_free(f_info);
	f_info = saved_f_info;
	mem_free(group->member_list);
	mem_free(group);
	c->monster_groups[1] = NULL;
	cave = NULL;
	cave_free(c);
	require(tracked_same);
	require(!target_same);
	require(target_cleared);
	eq(tracked_index, 1);
	ok;
}

static int test_persistent_world_map_fixture(void *state)
{
	struct chunk *c = cave_new(5, 5);
	int32_t saved_turn = turn;
	(void)state;

	cave = c;
	c->turn = 100;
	turn = 250;
	c->scent.grids[2][2] = 100;
	c->scent.grids[1][1] = 65530;
	c->scent.grids[0][0] = 88;
	c->noise.grids[2][2] = 55;
	c->noise.grids[0][0] = 77;
	age_scent();
	forget_noise();
	printf("NATIVE_PARITY_WORLD_MAP {\"scent_center\":%u,\"scent_overflow\":%u,\"scent_border\":%u,\"noise_center\":%u,\"noise_border\":%u,\"age_increment\":%d}\n",
	       c->scent.grids[2][2], c->scent.grids[1][1], c->scent.grids[0][0],
	       c->noise.grids[2][2], c->noise.grids[0][0],
	       turn / 10 - c->turn / 10);
	eq(c->scent.grids[2][2], 115);
	eq(c->scent.grids[1][1], 0);
	eq(c->scent.grids[0][0], 88);
	eq(c->noise.grids[2][2], 0);
	eq(c->noise.grids[0][0], 77);
	turn = saved_turn;
	cave = NULL;
	cave_free(c);
	ok;
}

static int test_rubble_allocation_fixture(void *state)
{
	struct feature *saved_f_info = f_info;
	struct trap_kind *saved_trap_info = trap_info;
	struct trap_kind fixture_traps[4] = { 0 };
	struct chunk *c;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int requested = 3, failed, placed = 0;
	struct loc changed[3];
	int changed_n = 0;
	int x, y;
	(void)state;

	for (x = 0; x < RAND_DEG; ++x) saved_rng[x] = STATE[x];
	f_info = mem_zalloc(FEAT_MAX * sizeof(*f_info));
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_FLOOR);
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_PASSABLE);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_ROCK);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_WALL);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_PERMANENT);
	flag_on(f_info[FEAT_RUBBLE].flags, TF_SIZE, TF_WALL);
	flag_on(f_info[FEAT_PASS_RUBBLE].flags, TF_SIZE, TF_PASSABLE);
	fixture_traps[3].name = "web";
	fixture_traps[3].desc = "web";
	fixture_traps[3].tidx = 3;
	trap_info = fixture_traps;
	z_info->trap_max = 4;
	c = cave_new(7, 9);
	for (y = 0; y < c->height; ++y) {
		for (x = 0; x < c->width; ++x) {
			square_set_feat(c, loc(x, y),
				(x == 0 || y == 0 || x == c->width - 1 ||
				 y == c->height - 1) ? FEAT_PERM : FEAT_FLOOR);
		}
	}
	Rand_quick = false;
	Rand_state_init(7);
	failed = alloc_objects(c, SET_CORR, TYP_RUBBLE, requested, 1, 0);
	for (y = 1; y < c->height - 1; ++y) {
		for (x = 1; x < c->width - 1; ++x) {
			int feat = square(c, loc(x, y))->feat;
			if (feat == FEAT_RUBBLE || feat == FEAT_PASS_RUBBLE) {
				changed[changed_n++] = loc(x, y);
				++placed;
			}
		}
	}
	printf("NATIVE_PARITY_RUBBLE {\"seed\":7,\"depth\":1,\"requested\":%d,\"failed\":%d,\"placed\":%d,\"rng_state_i\":%u,\"changed\":[",
		requested, failed, placed, state_i);
	for (x = 0; x < changed_n; ++x) {
		int feat = square(c, changed[x])->feat;
		printf("%s{\"x\":%d,\"y\":%d,\"feat\":%d}",
			x ? "," : "", changed[x].x, changed[x].y, feat);
	}
	printf("]}\n");
	eq(failed + placed, requested);
	eq(placed, requested);
	cave_free(c);
	mem_free(f_info);
	f_info = saved_f_info;
	trap_info = saved_trap_info;
	for (x = 0; x < RAND_DEG; ++x) STATE[x] = saved_rng[x];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

/* trap allocation fixture deferred: native catalog/effect legality is not isolated */
#if 0
static int test_trap_allocation_fixture(void *state)
{
	struct feature *saved_f_info = f_info;
	struct trap_kind *saved_trap_info = trap_info;
	struct trap_kind fixture_traps[3] = { 0 };
	struct chunk *c;
	uint32_t saved_rng[RAND_DEG];
	uint32_t saved_state_i = state_i, saved_rand_value = Rand_value;
	bool saved_rand_quick = Rand_quick;
	int x, y, failed, placed = 0;
	(void)state;

	for (x = 0; x < RAND_DEG; ++x) saved_rng[x] = STATE[x];
	f_info = mem_zalloc(FEAT_MAX * sizeof(*f_info));
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_FLOOR);
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_PASSABLE);
	flag_on(f_info[FEAT_FLOOR].flags, TF_SIZE, TF_TRAP);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_ROCK);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_WALL);
	flag_on(f_info[FEAT_PERM].flags, TF_SIZE, TF_PERMANENT);
	fixture_traps[1].name = "web";
	fixture_traps[1].desc = "web";
	fixture_traps[1].tidx = 1;
	fixture_traps[2].name = "fixture trap";
	fixture_traps[2].desc = "fixture trap";
	fixture_traps[2].tidx = 2;
	fixture_traps[2].rarity = 1;
	fixture_traps[2].min_depth = 0;
	fixture_traps[2].power.base = 1;
	trf_on(fixture_traps[2].flags, TRF_TRAP);
	trf_on(fixture_traps[2].flags, TRF_FLOOR);
	trap_info = fixture_traps;
	z_info->trap_max = 3;
	c = cave_new(7, 9);
	for (y = 0; y < c->height; ++y) {
		for (x = 0; x < c->width; ++x) {
			square_set_feat(c, loc(x, y),
				(x == 0 || y == 0 || x == c->width - 1 ||
				 y == c->height - 1) ? FEAT_PERM : FEAT_FLOOR);
		}
	}
	Rand_quick = false;
	Rand_state_init(7);
	failed = alloc_objects(c, SET_CORR, TYP_TRAP, 1, 1, 0);
	for (y = 1; y < c->height - 1; ++y) {
		for (x = 1; x < c->width - 1; ++x) {
			struct trap *trap = square_trap(c, loc(x, y));
			if (trap) {
				++placed;
				printf("NATIVE_PARITY_TRAP {\"seed\":7,\"depth\":1,\"requested\":1,\"failed\":%d,\"placed\":%d,\"rng_state_i\":%u,\"changed\":[{\"x\":%d,\"y\":%d,\"trap_id\":%d,\"power\":%d}]}\n",
					failed, placed, state_i, x, y, trap->t_idx, trap->power);
			}
		}
	}
	eq(failed + placed, 1);
	eq(placed, 1);
	cave_free(c);
	mem_free(f_info);
	f_info = saved_f_info;
	trap_info = saved_trap_info;
	for (x = 0; x < RAND_DEG; ++x) STATE[x] = saved_rng[x];
	state_i = saved_state_i;
	Rand_value = saved_rand_value;
	Rand_quick = saved_rand_quick;
	ok;
}

#endif

static int test_persistent_chunk_pair_fixture(void *state)
{
	struct chunk **saved_list = chunk_list;
	uint16_t saved_count = chunk_list_max;
	struct chunk *primary = cave_new(3, 3);
	struct chunk *known = cave_new(3, 3);
	struct chunk *other = cave_new(3, 3);
	bool found_primary, found_known, removed_primary, removed_known, removed_twice;
	bool order_after_primary, remaining_other;
	(void)state;

	chunk_list = NULL;
	chunk_list_max = 0;
	primary->name = string_make("Angband 1");
	known->name = string_make("Angband 1 known");
	other->name = string_make("Angband 2");
	chunk_list_add(primary);
	chunk_list_add(known);
	chunk_list_add(other);
	found_primary = chunk_find_name("Angband 1") == primary;
	found_known = chunk_find_name("Angband 1 known") == known;
	removed_primary = chunk_list_remove("Angband 1");
	order_after_primary = chunk_list_max == 2 && chunk_list[0] == known &&
		chunk_list[1] == other;
	removed_known = chunk_list_remove("Angband 1 known");
	remaining_other = chunk_list_max == 1 && chunk_list[0] == other;
	removed_twice = chunk_list_remove("Angband 1 known");
	printf("NATIVE_PARITY_CHUNK_PAIR {\"found_primary\":%s,\"found_known\":%s,\"removed_primary\":%s,\"order_after_primary\":%s,\"removed_known\":%s,\"remaining_other\":%s,\"removed_twice\":%s}\n",
	       found_primary ? "true" : "false", found_known ? "true" : "false",
	       removed_primary ? "true" : "false", order_after_primary ? "true" : "false",
	       removed_known ? "true" : "false", remaining_other ? "true" : "false",
	       removed_twice ? "true" : "false");

	mem_free(chunk_list);
	chunk_list = saved_list;
	chunk_list_max = saved_count;
	cave_free(primary);
	cave_free(known);
	cave_free(other);
	require(found_primary);
	require(found_known);
	require(removed_primary);
	require(order_after_primary);
	require(removed_known);
	require(remaining_other);
	require(!removed_twice);
	ok;
}

const char *suite_name = "parity/native";
struct test tests[] = {
	{ "rng-fixture", test_rng_fixture },
	{ "energy-fixture", test_energy_fixture },
	{ "attack-fixture", test_attack_fixture },
	{ "drop-fixture", test_drop_count_fixture },
	{ "compaction-reference-fixture", test_compaction_reference_fixture },
	{ "persistent-world-map-fixture", test_persistent_world_map_fixture },
	{ "persistent-chunk-pair-fixture", test_persistent_chunk_pair_fixture },
	{ "rubble-allocation-fixture", test_rubble_allocation_fixture },
	{ NULL, NULL },
};
