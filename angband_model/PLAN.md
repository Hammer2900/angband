# План точной модели Angband 4.2.6 в SandboxCore2026

## Partial commit failure trace/provenance — 2026-10-18

Добавлен внешний `PartialCommitTrace` и `inspect_partial_commit_trace`: partial
failure теперь привязывается к тому же transfer ID, терминальной failed boundary и
after-manifest digest. Для каждого commit-компонента сохраняется явное состояние;
неизвестные или отсутствующие компоненты остаются pending. Provenance и readiness
показывают отдельный `partial_commit_trace` binding и блокируют partial-after без
него. Retry, rollback и выполнение не добавлены; trace является только внешним
свидетельством наблюдаемого состояния.

Проверка: focused transfer commit/provenance/readiness tests — 30 OK; compile — OK.
Следующий этап каркаса: связать partial-after trace с disposition/cleanup evidence,
не вычисляя retention, rollback или фактическое владение.

## Partial outcome/disposition/cleanup consistency — 2026-10-18

Добавлен read-only `inspect_partial_commit_outcome_consistency`: partial-after
trace, terminal disposition и cleanup evidence сопоставляются по transfer ID и
boundary index. Для retained disposition сверяется state manifest digest с
after-manifest digest partial trace; discarded disposition требует только
декларацию `discard_scratch`. Никакая запись не доказывает фактическое удержание,
удаление, rollback или выполнение cleanup; неполные записи остаются pending.
Обычный completed transfer получает `partial_outcome=not_applicable`.

Проверка: focused transfer tests — 32 OK; compile и git diff --check — OK.
Следующий этап каркаса: проверить cleanup-attempt transitions относительно partial
failure, не вызывая cleanup adapter и не подтверждая фактическое удаление.

## Partial failure cleanup-attempt binding — 2026-10-18

Добавлен read-only `inspect_partial_cleanup_consistency`: cleanup evidence
сопоставляется с partial trace по transfer ID и failed boundary, проверяется action
`discard_scratch` и наличие attempt records. Pending/failed попытки не становятся
успешными из-за одной записи; даже structurally consistent completed attempt не
ставит `cleanup_verified=True` и не доказывает фактическое удаление. Readiness
публикует отдельную секцию `partial_cleanup`; обычный completed transfer получает
`not_applicable`.

Проверка: focused commit/cleanup/disposition/readiness tests — 62 OK; compile и
git diff --check — OK.
Следующий этап каркаса: связать idempotency key/request digest cleanup attempts с
partial trace provenance, не выполнять cleanup и не выводить ownership.

## Partial cleanup idempotency/request provenance — 2026-10-18

`PartialCommitTrace` получил caller-declared `cleanup_idempotency_key` и
`cleanup_request_digest`. `inspect_partial_cleanup_consistency` сверяет их с
cleanup evidence и каждой attempt-записью, вместе с transfer ID и failed boundary.
Несовпадение даёт failed, отсутствие декларации остаётся pending. Это не
проверяет уникальность в runtime, выполнение adapter или ownership; cleanup
verification остаётся false.

Проверка: focused commit/cleanup/disposition/readiness tests — 62 OK; compile и
git diff --check — OK.
Следующий этап каркаса: связать cleanup provenance с after-state/disposition digest
для retained/discarded partial outcomes, не выводя фактический результат cleanup.

## Partial cleanup outcome/digest reconciliation — 2026-10-18

`inspect_partial_commit_outcome_consistency` теперь сверяет partial after-manifest
digest с retained disposition и сопоставляет disposition только с завершёнными
cleanup attempts. Pending/failed/unknown попытки остаются unresolved; конфликт
`retained`/`discarded` даёт failed. Согласованная внешняя запись не включает
`runtime_state_verified` или `cleanup_verified`: фактическое сохранение/удаление
по-прежнему не выводится.

Проверка: focused commit/cleanup/disposition/readiness tests — 64 OK; compile и
git diff --check — OK.
Следующий этап каркаса: связать этот reconciliation с transfer debug summary и
строгой валидацией отчёта, не добавляя execution semantics.

## Partial reconciliation in transfer debug — 2026-10-18

`transfer_debug` теперь публикует `partial_cleanup` и `partial_outcome` в sections,
а summary содержит их статусы. `validate_transfer_debug_report` сверяет summary
соответствие sections и отвергает drift. Новые поля остаются диагностическими;
execution, cleanup verification и native fidelity не меняются.

Проверка: focused debug/commit/cleanup/disposition/readiness tests — 106 OK;
compile и git diff --check — OK.
Следующий этап каркаса: проверить strict JSON и immutable snapshot semantics для
новых partial reconciliation sections.

## Partial reconciliation strict JSON/snapshot contract — 2026-10-18

`transfer_debug` возвращает partial cleanup/outcome sections после strict JSON
round-trip. Добавлены проверки `allow_nan=False` и detached nested collections:
мутация полученного отчёта не меняет следующий отчёт или исходные evidence.
Validator продолжает сверять summary со sections; execution и native fidelity
остаются запрещены.

Проверка: focused debug/commit/cleanup/disposition/readiness tests — 107 OK;
compile и git diff --check — OK.
Следующий этап каркаса: связать partial reconciliation с lifecycle trace health,
проверяя cursor/terminal state без replay или исполнения transfer.

## Partial reconciliation lifecycle-health binding — 2026-10-18

Добавлен read-only `inspect_partial_lifecycle_consistency`: partial trace сверяется
с detached lifecycle health по transfer ID/cursor и проверяется `failed`, terminal,
commit-started state. Readiness строит health только из существующего immutable trace
telemetry; debug summary/validator публикуют и проверяют новый статус. Несовпадение
даёт failed, отсутствие health — pending; replay, retry и execution не вызываются.

Проверка: focused debug/commit/cleanup/disposition/readiness tests — 109 OK;
compile и git diff --check — OK.
Следующий этап каркаса: связать lifecycle health с provenance boundary digest, не
подменяя digest доказательством исполнения.

## Provenance boundary/lifecycle digest binding — 2026-10-18

Добавлен `inspect_provenance_lifecycle_boundary`: binding сверяется с исходным
immutable trace по transfer ID и вычисленным prefix digest, а с lifecycle health —
по transfer ID и cursor. Несовпадение boundary или редактирование digest даёт
failed; отсутствие health остаётся pending. Digest фиксирует структурную связь и
не является доказательством исполнения или native fidelity.

Проверка: focused provenance/commit/readiness/debug tests — 82 OK; compile и
git diff --check — OK.
Следующий этап каркаса: опубликовать provenance/lifecycle binding в debug summary,
не добавляя новых execution claims.

## Provenance/lifecycle binding in debug — 2026-10-18

Readiness публикует отдельный `partial_provenance_lifecycle`, а transfer debug
выводит его в sections и summary. Validator сверяет статус summary с section и
отвергает drift. Секция остаётся структурной диагностикой: digest/cursor не
становятся execution proof, native validation и transfer execution не добавлены.

Проверка: focused debug/provenance/readiness/commit tests — 82 OK; compile и
git diff --check — OK.
Следующий этап каркаса: добавить immutable snapshot/strict JSON проверки для
provenance/lifecycle section и затем зафиксировать общий transfer contract.

## Provenance/lifecycle strict JSON snapshot — 2026-10-18

`partial_provenance_lifecycle` включён в strict JSON/detached snapshot проверки
transfer debug. Мутация его nested collections не влияет на следующий отчёт;
validator принимает неизменённый payload. Это остаётся structural evidence без
execution или native fidelity claims.

Проверка: focused debug/provenance/readiness/commit tests — 82 OK; compile и
git diff --check — OK.
Следующий этап каркаса: зафиксировать единый read-only transfer contract для
lifecycle, provenance, partial cleanup и outcome sections.

## Unified read-only transfer observation contract — 2026-10-18

Добавлен `describe_transfer_observation_contract`: единый контракт перечисляет
lifecycle/provenance/partial sections, допустимые статусы и запрещённые capability
flags (`execution`, retry, rollback, cleanup verification). Readiness публикует
его, transfer debug возвращает его отдельным полем, а validator отвергает drift и
включённые execution flags. Контракт остаётся metadata-only и не запускает adapters.

Проверка: focused debug/provenance/readiness/commit/cleanup/disposition tests —
114 OK; compile — OK.
Следующий этап каркаса: добавить отдельную валидацию общего contract payload и
immutable strict-JSON snapshot, затем перейти к следующей pending области плана.

## Unified transfer contract validation — 2026-10-18

Добавлен `validate_transfer_observation_contract`: проверяются точные sections,
statuses, counts, strict JSON и все запрещённые capability flags. Debug validator
вызывает contract validator и отвергает drift. Контракт остаётся side-effect-free,
execution/retry/rollback/cleanup verification не разрешены.

Проверка: focused debug/provenance/readiness/commit/cleanup/disposition tests —
115 OK; compile и git diff --check — OK.
Следующий этап каркаса: перейти к следующей pending области плана после transfer
provenance — native generation tail или другой явно отмеченный adapter boundary.

## Native generation-tail boundary audit — 2026-10-18

Проверено, что native generation tail уже имеет metadata-only контракт в
`generation_pipeline`: шесть фаз в точном порядке `classic_gen` — rubble, traps,
player placement, monsters, objects, gold — с dependencies, source refs, native
calls и `generation_complete=False`. Existing validators и generation-tail probe
проверяют порядок, refs, strict JSON и non-executable flags. Scenario callbacks не
заменяют native callbacks.

Новый Python placeholder не добавлялся: следующий реальный шаг требует
engine-owned adapters/callbacks, native chunk state, candidate predicates, RNG
handoff и differential evidence для каждой фазы. Без них нельзя объявлять tail
реализованным или угадывать counts/placement/RNG.

Проверка audit: generation-tail/pipeline/progress suite — 748 OK; 71.261 с.
Это был широкий запуск, не повторять для routine checks; дальше использовать
только focused generation-tail tests и `one_room`.

## Rubble helper/boundary checkpoint — 2026-10-18

Rubble-specific helpers и bridge проверены без расширения native claims: count/RNG
order, candidate search, allocation accounting, terrain installation, result health
и pending continuation уже покрыты. Bound native-shaped bridge reports
`bindings_complete`, но execution остаётся disabled. Не закрыты engine-owned
candidate set, corridor predicates, allocation ownership и native chunk/RNG handoff;
portable helper success не переводит `generation_complete` в true.

Проверка: 8 focused rubble backend tests — OK за 0.102 с; compile и
git diff --check — OK.
Следующий шаг требует C-side rubble callback/native probe boundary; Python-only
stub или guessed placement/RNG не добавлять.

## Rubble C allocation boundary audit — 2026-10-18

Проверен native call path: `classic_gen` вызывает публичный
`alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), depth, 0)`; `alloc_objects`
вызывает `alloc_object`, который сканирует interior через `cave_find`, проверяет
`square_isempty`/`SET_CORR` и вызывает внутренний `place_rubble`. Python API сейчас
экспортирует только tail metadata, не rubble execution callback.

Для корректного native probe нужны `cave_new`, `f_info` terrain flags, `z_info`,
RNG initialization и native chunk/object state. Probe должен изолированно вызвать
только rubble allocation, записать requested/placed/failed, changed cells и RNG
boundary, затем освободить fixture; обычный `classic_gen` и Python execution
трогать нельзя. Без этих prerequisites probe может дать ложную parity, поэтому
C fixture пока не добавлен.

Проверка audit: 8 focused rubble backend tests — OK за 0.102 с; C source path
проверен по `generate.h`, `gen-util.c` и `gen-cave.c`.
Следующий шаг: подготовить prerequisites и добавить отдельный native rubble probe.

## Rubble native-probe prerequisites — 2026-10-18

Уточнено, что `alloc_objects` публичен через `generate.h`, но его rubble path
зависит от `cave_new`, `f_info` feature flags, `z_info`, `Rand_state_init`,
`cave_find` interior scan и native terrain/object state. Existing parity tests
имеют разные минимальные fixtures и не предоставляют готовый rubble fixture.

Probe contract: создать отдельный synthetic corridor chunk, инициализировать
только подтверждённые terrain flags/constants, сохранить RNG до/после, вызвать
`alloc_objects(SET_CORR, TYP_RUBBLE, requested, depth, 0)`, собрать changed cells
и failed count, затем освободить chunk и восстановить globals. Probe не должен
вызывать `classic_gen`, Python callbacks или менять обычную генерацию.

Пока добавлен только boundary audit; C/Python execution не изменён, потому что
неполная fixture initialization дала бы ложные native parity claims.

Проверка: source audit `generate.h`/`gen-util.c`/`gen-cave.c`; существующие
8 focused rubble backend tests — OK за 0.102 с.
Следующий шаг: минимальная C fixture initialization и отдельный native probe test.

## Native rubble allocation probe — 2026-10-18

Добавлен изолированный `parity/native` fixture: временные `f_info`, trap catalog,
synthetic 7x9 chunk и RNG seed 7; вызывается только публичный
`alloc_objects(SET_CORR, TYP_RUBBLE, 3, 1, 0)`. Probe печатает detached JSON и
восстанавливает RNG/feature/trap globals. Получено: requested=3, failed=0,
placed=3, `rng_state_i=24`; `parity/native` — 8/8 passed.

Добавлен Python differential regression test с явным probe path override. Он
проверяет JSON allocation boundary и `placed + failed == requested`; это не
подтверждает full generation-tail parity и не подключает callback к `classic_gen`.

Проверка: native target build/pass; targeted differential test — OK без skip;
Python compile и git diff --check — OK.
Следующий шаг: сравнить native rubble changed-cell/RNG evidence с portable helper
на том же fixture, не объявляя rubble adapter готовым.

## Rubble native/portable comparison boundary — 2026-10-18

Native probe расширен changed-cell records: seed 7, requested=3, failed=0,
placed=3, `rng_state_i=24`, cells `(1,1)`, `(2,1)`, `(7,5)`, all
`FEAT_RUBBLE=24`. Прямое сравнение с текущим portable helper невалидно:
native `alloc_objects` владеет interior `cave_find` scan и вызывает ровно заданный
request count, а portable `execute_rubble_placement_native` сначала выполняет
полный `roll_classic_corridor_counts` и получает другой count/RNG sequence; его
candidate list также передаётся снаружи. Parity claim не добавлен.

Проверка: native parity target — 8/8; native probe build/pass; targeted
Python differential allocation test — OK; portable smoke показал отдельную RNG/
candidate boundary. Следующий шаг: explicit native candidate-scan/RNG handoff
contract, затем сравнение только после выравнивания boundary.

## Rubble native candidate-scan/RNG ownership contract — 2026-10-18

Добавлены `describe_rubble_native_boundary` и
`validate_rubble_native_boundary`. Контракт фиксирует native entry
`alloc_objects`, engine-owned interior scan/predicates/RNG/install, caller inputs
(chunk/depth/requested/RNG-before) и detached result fields (counts, changed cells,
RNG before/after). Execution, callbacks и parity остаются pending; Python не
получает право выбирать native candidates или RNG.

Проверка: 4 focused generation pipeline/backend tests — OK за 0.092 с; compile и
git diff --check — OK.
Следующий шаг: связать native probe JSON с этим boundary contract и валидировать
его shape/ownership без изменения `classic_gen`.

## Rubble native probe contract validation — 2026-10-18

Добавлен `validate_rubble_native_probe`: detached JSON проверяет обязательные
seed/depth/count/RNG fields, changed-cell shape и count reconciliation без
повторного запуска native allocation. Native probe output теперь содержит
changed cells; Python differential fixture сверяет `(1,1)`, `(2,1)`, `(7,5)` и
`FEAT_RUBBLE=24` при seed 7. Boundary contract по-прежнему фиксирует engine
ownership; probe evidence не превращает portable helper или `classic_gen` в
native-parity-ready adapter.

Проверка: native parity target 8/8; targeted differential probe test — OK без
skip; boundary unit test — OK; compile и git diff --check — OK.
Следующий шаг: добавить immutable/detached snapshot validation probe result и
явно сохранить native execution/parity pending.

## Rubble native probe detached snapshot — 2026-10-18

Добавлен `detach_rubble_native_probe`: native JSON проходит strict
`allow_nan=False` round-trip, а nested changed-cell records отделяются от
исходного payload. Unit-тест проверяет mutation isolation и rejection non-JSON
values. Probe остаётся внешним evidence; native execution/parity и `classic_gen`
integration не включены.

Проверка: targeted boundary/snapshot test — 1 OK; compile и git diff --check — OK.
Следующий шаг: добавить probe result health validator, связывающий counts,
changed cells и RNG boundary без повторного запуска native allocation.

## Rubble native probe result health — 2026-10-18

Добавлен `health_rubble_native_probe`: detached evidence проверяет count
reconciliation, changed-cell count, уникальность координат и interior bounds.
Health validator не перезапускает native allocation, не подтверждает ownership и
сохраняет `native_validation_pending=True`/`execution_supported=False`.

Проверка: targeted boundary/health test — 1 OK; compile и git diff --check — OK.
Следующий шаг: добавить RNG-boundary consistency check между probe result и
portable/native observation schema, не сравнивая неподтверждённые full streams.

## Rubble probe RNG-boundary health — 2026-10-18

`health_rubble_native_probe` проверяет `rng_state_i` и optional before/after
indices в native range `0..31`, сохраняя их semantics как opaque state index.
Разница индексов не используется для вывода draw count, full RNG streams не
сравниваются, execution/parity не объявляются. Counts и changed-cell health
остаются отдельными structural checks.

Проверка: targeted health/RNG test — 1 OK; compile и git diff --check — OK.
Следующий шаг: связать probe health с differential report metadata, явно разделяя
structural evidence и native parity status.

## Rubble differential evidence status — 2026-10-18

Добавлены `build_rubble_differential_report` и
`validate_rubble_differential_report`. Отчёт разделяет `structural_status`,
`native_evidence_status=observed` и `parity_status=pending`; проверяет probe health
и запрещает преждевременный parity completion. Это detached metadata-only report:
execution/callbacks disabled, native validation pending.

Проверка: targeted differential-status test — 1 OK; compile и git diff --check — OK.
Следующий шаг: связать этот differential report с generation-tail readiness, не
переводя `generation_complete` или rubble adapter в ready.

## Rubble differential evidence in tail readiness — 2026-10-18

`GenerationTailBridge.readiness()` принимает optional `rubble_differential` и
публикует его как диагностическую секцию. Valid structural/native evidence не
делает tail ready: при переданном отчёте readiness требует
`parity_status=complete`, который сейчас остаётся pending. Без evidence сохраняется
прежняя callback-binding semantics; `generation_complete` и execution flags не
меняются.

Проверка: targeted readiness/differential test — 1 OK; compile и git diff --check —
OK.
Следующий шаг: добавить strict validator для readiness+differential связки и
проверить detached snapshot, не поднимая parity status.

## Rubble differential readiness validation — 2026-10-18

Добавлен `validate_rubble_readiness_binding`: differential report в tail readiness
проверяется как strict JSON, parity обязан оставаться pending, а `ready=True` при
pending parity отвергается. `GenerationTailBridge.readiness()` теперь публикует
rubble evidence через detached JSON round-trip, поэтому nested probe cells не
делят ссылки с caller payload. Generation completion и execution не меняются.

Проверка: targeted readiness/detached snapshot test — 1 OK; compile и
git diff --check — OK.
Следующий шаг: добавить этот readiness binding в общий generation progress/debug
report, сохраняя native tail pending.

## Rubble differential in generation progress — 2026-10-18

`generation_bridge_progress_report` принимает optional rubble differential и
публикует detached status/validation. Structural/native evidence не уменьшает
`remaining_bridges`, не меняет callback counters и не включает
`native_execution_eligible`; validator отвергает `parity_status=complete` до
полного boundary comparison.

Проверка: targeted adapter-progress linkage test — 1 OK; compile и
git diff --check — OK.
Следующий шаг: strict JSON/detached snapshot validation для progress report с
rubble evidence, затем зафиксировать rubble audit как pending engine boundary.

## Rubble progress strict JSON snapshot — 2026-10-18

`generation_bridge_progress_report` публикует rubble differential после strict
JSON round-trip. Nested probe/cell mutation не затрагивает исходное evidence;
validator сохраняет `parity_status=pending` и `native_execution_eligible=False`.
Generation bridge counters не меняются.

Проверка: targeted progress snapshot test — 1 OK; compile и git diff --check — OK.
Следующий шаг: завершить rubble audit как pending engine boundary и перейти к
следующей native tail phase только после явной engine-owned точки расширения.

## Random-trap native boundary audit — 2026-10-18

Rubble audit остаётся pending engine boundary: probe/evidence не подключены к
`classic_gen`. Для следующей фазы добавлены `describe_trap_native_boundary` и
`validate_trap_native_boundary`. Контракт фиксирует
`alloc_objects(SET_CORR, TYP_TRAP, requested, depth, 0)`, engine-owned interior
scan/predicates, weighted `pick_trap` catalog, `randcalc` power, RNG и
`place_trap` install. Trap context остаётся explicit; Python не выбирает trap,
power или native RNG, execution/parity pending.

Проверка: targeted pipeline trap/rubble boundary test — 1 OK; compile и
git diff --check — OK.
Следующий шаг: native trap probe prerequisites и detached result contract, по
аналогии с rubble, без запуска полного `classic_gen`.

## Random-trap probe prerequisites audit — 2026-10-18

Trap probe требует не только synthetic chunk/RNG: `lookup_trap` зависит от
`trap_info`/`z_info->trap_max`, а `place_trap` — от weighted trap kind flags,
effect/power `random_value`, `randcalc`, arena/depth gates и square trap state.
Без подтверждённого catalog/effect fixture C probe дал бы ложную parity, поэтому
trap execution probe пока не добавлен.

Safe boundary уже зафиксирован в `TrapPlacementBridge` и новом
`describe_trap_native_boundary`: engine владеет selection/power/catalog/RNG/install,
Python callback не синтезирует trap result.

Проверка audit: C `trap.h`/`trap.c`/`gen-util.c`/`gen-cave.c` и существующие
focused trap bridge tests; execution/parity остаются pending.
Следующий шаг: detached trap-result contract/validator, затем C probe только после
полной fixture initialization.

## Random-trap detached result contract — 2026-10-18

Добавлен `validate_trap_native_result`: detached evidence проверяет seed/depth,
requested/failed/placed reconciliation, changed-cell fields `(x,y,trap_id,power)`
и strict JSON. Validator явно оставляет `parity_status=pending`, execution disabled
и callbacks не вызываются. Trap catalog/effect/power fixture и native C probe пока
не добавлялись, поскольку их неполная инициализация дала бы ложную parity.

Проверка: targeted trap/rubble pipeline test — 1 OK; compile и git diff --check —
OK.
Следующий шаг: detached snapshot/health для trap result и затем минимальный C trap
fixture с подтверждённым catalog/effect state.

## Random-trap C fixture blocker — 2026-10-18

Попытка synthetic C fixture для `alloc_objects(SET_CORR, TYP_TRAP, ...)` показала
дополнительную native dependency: `square_isempty` вызывает `lookup_trap("web")`
через `square_iswebbed`, поэтому минимального fixture с одной weighted trap entry
недостаточно. После добавления web/catalog entries allocation не дал trap result
(8/9 harness tests), поэтому fixture отключён и parity claim не сделан.
Rubble fixture остаётся валидным: native parity снова **8/8**.

Следующий trap шаг: использовать подтверждённый штатный catalog/effect init либо
отдельно пробировать `place_trap` с явным trap index, не угадывая weighted
`pick_trap` prerequisites. Обычный `classic_gen` не менять.

## Explicit native trap install probe — 2026-10-18

Добавлен отдельный `parity/trap.c` с штатным `set_file_paths/init_angband/
player_make_simple` harness. Synthetic chunk вызывает только `place_trap` с явным
`lookup_trap("web")` index, не weighted `pick_trap`; detached JSON сообщает cell,
trap_id=4, power=0, counts и opaque RNG index. Python differential test запускает
probe из `GAME_ROOT`, проверяет стабильные install facts и только диапазон RNG
index `0..31`.

Проверка: `unittest-parity-trap` build и `parity/trap` 1/1; targeted differential
trap test — 1 OK; compile и git diff --check — OK. Full random-trap allocation
parity и `classic_gen` integration остаются pending.

Следующий шаг: связать explicit trap probe с detached trap-result health/report,
не выдавая его за weighted allocation parity.

## Explicit trap install differential report — 2026-10-18

Добавлены `build_trap_install_report` и `validate_trap_install_report` для
explicit-index native evidence. Отчёт отделяет `selection_mode=explicit_index`,
`native_evidence_status=observed` и `weighted_selection_parity=pending`; result
health/strict JSON проверяются без повторного запуска `place_trap`. Weighted
`pick_trap`, random allocation и `classic_gen` parity остаются pending.

Проверка: targeted trap install/report test — 1 OK; compile и git diff --check —
OK.
Следующий шаг: подключить explicit trap report к generation readiness/progress
только как diagnostic evidence, не уменьшая pending random-trap work.

## Explicit trap report in generation progress — 2026-10-18

`generation_bridge_progress_report` принимает optional `trap_install`, публикует
его detached status/validation и сохраняет `weighted_selection_parity=pending`.
Explicit install evidence не уменьшает `remaining_bridges`, callback counters или
`native_execution_eligible`; validator отвергает completion drift.

Проверка: targeted adapter-progress explicit-trap linkage test — 1 OK; compile и
git diff --check — OK. Native explicit probe остаётся отдельным evidence path;
random allocation/full `classic_gen` parity pending.

Следующий шаг: strict snapshot validation для progress `trap_install`, затем
зафиксировать random-trap boundary и перейти к следующей tail phase.

## Explicit trap progress detached snapshot — 2026-10-18

Проверено, что `generation_bridge_progress_report(trap_install=...)` делает strict
JSON/detached snapshot: nested `result.changed` не мутирует исходный trap result.
Validator сохраняет `weighted_selection_parity=pending`, counters не меняются,
`native_execution_eligible=False`.

Проверка: targeted progress snapshot test — 1 OK; native explicit trap probe —
`parity/trap 1/1`; compile и git diff --check — OK.

Random-trap boundary зафиксирован как engine-owned: explicit install evidence
получен, но weighted allocation/catalog parity и `classic_gen` integration pending.
Следующая tail phase: player placement/start.

## Player-start native boundary contract — 2026-10-18

Добавлены `describe_player_start_native_boundary`,
`validate_player_start_native_boundary` и `validate_player_start_native_result`.
Контракт фиксирует engine ownership persistent stair reuse, ordered `find_start`,
incoming stair flags, `player_place`/occupancy и failed-generation cleanup.
Detached result проверяет status, position, stair evidence и install flag; execution
и parity остаются pending.

Player-start evidence подключён в `GenerationTailBridge.readiness()` и
`generation_bridge_progress_report` через strict JSON detached snapshots. Valid
structural evidence не уменьшает pending bridge counters, не включает native
execution и не делает `generation_complete=True`.

Проверка: focused player boundary/progress tests — 2 OK; `one_room` — won, 8/8
фаз; compile и git diff --check — OK.
Следующий батч: native player-start probe с engine-owned `new_player_spot`, либо
фиксация prerequisites/blocker без Python-only замены.

## Native player-start probe — 2026-10-18

Добавлен `parity/player.c` со штатным `set_file_paths/init_angband/
player_make_simple` harness. Synthetic 7x9 chunk вызывает публичный
`new_player_spot`: engine сам выполняет ordered start search, incoming stair
creation и `player_place`. Detached JSON сообщает completed position, `FEAT_MORE`
(6), installation и opaque RNG index; RNG/global cleanup исправлены после первого
double-free fixture bug.

Добавлен Python differential test с `GAME_ROOT` cwd: проверяет interior position,
feature, installed flag и RNG index range без фиксации draw count. `classic_gen`
не запускается, player adapter readiness не меняется.

Проверка: `unittest-parity-player` build, `parity/player` 1/1; targeted
 differential player test — 1 OK; one_room — won, 8/8 фаз; compile и
git diff --check — OK.
Следующий шаг: подключить player-start result report к readiness/progress strict
snapshot, затем перейти к monster population boundary.

## Monster population native boundary contract — 2026-10-18

Добавлены `describe_monster_native_boundary`,
`validate_monster_native_boundary` и `validate_monster_native_result`. Контракт
фиксирует engine ownership attempt budget, 10000-attempt distant search,
empty/restriction/distance predicates, depth-weighted `get_mon_num`, placement и
friends/escort group expansion. Detached result проверяет attempts/placed/failed,
created IDs и opaque RNG index; spawning/race selection/group RNG не выполняются.

Monster evidence подключён к tail readiness и generation progress через strict JSON
detached snapshots. Valid evidence не уменьшает bridge counters, не включает native
execution и оставляет parity pending.

Проверка: focused monster boundary/progress tests — 2 OK; one_room — won, 8/8
фаз; compile и git diff --check — OK.
Следующий батч: native monster probe prerequisites или следующий objects/gold
boundary, не создавая Python-only monster semantics.

## Objects/gold native boundary contract — 2026-10-18

Добавлены `describe_object_gold_native_boundary`,
`validate_object_gold_native_boundary` и `validate_object_gold_native_result`.
Контракт фиксирует source order room objects → both objects → both gold, engine-owned
`Rand_normal` counts, interior allocation scan, object/ego/artifact construction,
gold type/value/no-selling context и floor carry. Detached result проверяет counts,
created IDs, floor positions и opaque RNG index; content creation не выполняется.

Objects/gold evidence подключён в tail readiness и generation progress через strict
JSON detached snapshots. Valid evidence не уменьшает bridge counters, не включает
native execution и оставляет parity pending.

Проверка: focused object/monster/player boundary/progress tests — 4 OK;
one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: native monster probe prerequisites или C object/gold probe audit.

## Objects/gold and monster boundary completion — 2026-10-18

Objects/gold boundary/result contracts добавлены и подключены к readiness/progress;
source order и engine ownership сохранены, content creation не выполняется.
Monster boundary/result contracts также добавлены: engine owns attempt budget,
distant search, restrictions/distance, depth-weighted race selection, placement и
friends/escort expansion. Evidence публикуется detached и остаётся parity pending.

Native monster probe пока отложен: корректный C fixture требует штатного `r_info`
allocation table, monster race data, unique/group/friends state, player/chunk
context и cleanup. Python-only race/group semantics запрещены.

Проверка: focused object/monster/player boundary/progress tests — 4 OK;
one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: native monster init/probe prerequisites или C object/gold probe
audit с полным штатным data initialization.

## Native object/gold install probe — 2026-10-18

Добавлен `parity/object.c` со штатным init harness и synthetic arena. Probe явно
вызывает `place_object` и `place_gold` на соседних клетках, возвращает object/gold
IDs, positions, placement flags и opaque RNG index; allocation count/candidate scan
и full `classic_gen` остаются отдельным pending boundary.

Python differential test сверяет object_id=1, gold_id=2 и позиции `(2,2)/(3,2)`;
RNG проверяется только как opaque range. Object/gold evidence contracts уже
подключены к readiness/progress и не включают execution.

Проверка: `unittest-parity-object` 1/1; targeted differential object test — 1 OK;
object/monster/player boundary/progress tests — 4 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: native `alloc_objects` object/gold allocation probe или
завершение monster random-search/group boundary.

## Native object/gold allocation probe — 2026-10-18

Расширен `parity/object.c`: второй fixture вызывает публичные
`alloc_objects(SET_BOTH, TYP_OBJECT, 1, ...)` и `alloc_objects(SET_BOTH, TYP_GOLD, 1,
...)` после штатного init. Probe считает все floor objects по native interior scan и
разделяет object/gold counts; fixed-cell assumptions удалены. Получено object=1,
gold=1, `parity/object` — 2/2.

Python differential test проверяет counts, depth/seed и opaque RNG range; explicit
install test обновлён под новый harness count. Это не закрывает `Rand_normal` count
roll, full candidate parity или `classic_gen` integration.

Проверка: native object target 2/2; two targeted differential tests — 2 OK;
compile и git diff --check — OK.
Следующий batch: full allocation count/candidate differential или monster random
search/group boundary.

## Monster random-search/group boundary contract — 2026-10-18

Добавлены отдельные `describe_monster_search_boundary`/
`validate_monster_search_boundary` и `describe_monster_group_boundary`/
`validate_monster_group_boundary`. Search contract фиксирует engine-owned
10000-attempt coordinate RNG и empty/restriction/distance predicates; group
contract фиксирует friends/friends_base, scatter/open-cell, group limits/unique и
RNG ownership. Explicit monster probe остаётся отдельным install evidence path и
не подтверждает random race/group parity.

Проверка: 4 focused monster/object/player boundary tests — OK; native explicit
monster probe — 1/1; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: native random monster search/group probe prerequisites или
завершение full object/gold allocation count contract.

## Native random monster search probe — 2026-10-18

Расширен `parity/monster.c` штатным random fixture: `pick_and_place_distant_monster`
на synthetic arena после `init_angband/player_make_simple`. Probe возвращает
`placed=true`, `monster_count>0` и opaque RNG index; explicit race/install fixture
остаётся отдельным. При текущем seed `monster_count` зависит data/RNG context, поэтому
фиксируется только как положительный observed result.

Python differential test проверяет seed/depth/placed/count>0/RNG range, без вывода
race-selection или group draw semantics. `validate_monster_search_result` проверяет
boolean placement, positive count и opaque RNG range без выбора race/spawn.
Search/group contracts отделены и остаются parity pending; `classic_gen` не запускается.

Проверка: `parity/monster` 2/2; random+explicit differential tests — 2 OK;
boundary/search/group tests — 3 OK; one_room — won, 8/8 фаз; compile и
git diff --check — OK.
Следующий batch: native group expansion evidence или full object/gold
`Rand_normal` allocation boundary.

## Native monster group expansion probe — 2026-10-18

Расширен `parity/monster.c` третьим штатным fixture: выбирается первая
non-unique race с `friends`/`friends_base` из реального catalog и вызывается
`place_new_monster(..., group_ok=true)`. Probe сообщает `placed=true`, positive
monster_count, race_id и `group_expansion=true`; конкретный count/RNG не
фиксируется из-за data-dependent friends/escort rolls.

Добавлен `validate_monster_group_result`; differential test проверяет observed
placement/group evidence и opaque RNG range. Explicit install, random search и
group expansion остаются отдельными evidence paths; full `classic_gen` parity
не объявляется.

Проверка: `parity/monster` 3/3; group/random/explicit differential tests — 3 OK;
search/group/result tests — OK; compile и git diff --check — OK.
Следующий batch: full object/gold `Rand_normal` allocation boundary.

## Full object/gold Rand_normal allocation probe — 2026-10-18

`parity/object.c` allocation fixture теперь воспроизводит native count order:
`Rand_normal(room_item_av, 3)`, `Rand_normal(both_item_av, 3)`,
`Rand_normal(both_gold_av, 3)`, затем room/both object и both gold allocations.
Synthetic interior помечен `SQUARE_ROOM` для SET_ROOM; probe считает все floor
objects и разделяет object/gold counts.

Python differential test фиксирует source-backed accounting relations:
`object_count = room_requested + both_object_requested`,
`gold_count = both_gold_requested`; конкретные случайные counts/RNG cursor не
зашиваются, только opaque range. Full candidate/item semantics и classic_gen
integration остаются pending.

Проверка: native object target 2/2; allocation/explicit differential tests —
2 OK; compile и git diff --check — OK.
Следующий batch: monster group completion review или native full classic-tail
allocation integration audit.

## Unified native-tail evidence aggregate — 2026-10-18

Добавлены `build_generation_tail_evidence_report` и
`validate_generation_tail_evidence_report`. Aggregate фиксирует ровно шесть фаз в
classic order, observed/not_observed per phase, `parity_status=pending`,
`generation_complete=False` и execution disabled. Explicit probe evidence теперь
может быть сведено в одну metadata-only проекцию без certification full tail.

Progress summary публикует aggregate как `native_generation_tail_evidence` и
validator запрещает completion drift. Existing bridge counters и one-room behavior
не изменились.

Проверка: aggregate/progress tests — 2 OK; one_room — won, 8/8 фаз; compile и
git diff --check — OK.
Следующий batch: full classic-tail allocation integration audit либо native
player/monster/object combined fixture.

## Full classic-tail integration prerequisites audit — 2026-10-18

Добавлены `describe_classic_tail_integration_prerequisites` и validator. Контракт
фиксирует required runtime globals/catalogs (`player`, `chunk`, `dun`, profile,
z_info/f_info/r_info/k_info/trap_info, RNG, events), exact six-phase order,
ownership domains и native failure-cleanup policy. Probe mode явно isolated; обычный
`classic_gen` dispatch не объявляется безопасным без всех prerequisites.

Aggregate evidence report уже публикуется как metadata-only; `generation_complete`
остаётся false, execution disabled, parity pending. Full classic-tail probe отложен
до единого штатного harness, чтобы не скрыть phase-specific failures.

Проверка: prerequisites/aggregate tests — 2 OK; one_room — won, 8/8 фаз; compile и
git diff --check — OK.
Следующий batch: native combined six-phase harness prerequisites или отдельный
full `classic_gen` probe с проверяемым cleanup/error path.

## Combined six-phase evidence manifest — 2026-10-18

Добавлены `build_generation_tail_manifest` и
`validate_generation_tail_manifest`. Manifest сводит шесть phases с exact classic
order, adapter/source refs, `observed/not_observed/invalid` evidence status и
per-phase pending/execution flags. Nested evidence detached через strict JSON;
manifest запрещает parity/generation completion drift.

Это metadata-only combined report, не запуск `classic_gen`: failure cleanup,
shared RNG ordering и actual cross-phase ownership всё ещё требуют единого native
harness.

Проверка: manifest/prerequisite/aggregate tests — 3 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: native combined six-phase harness prerequisites или отдельный
full `classic_gen` probe с проверяемым cleanup/error path.

## Native combined full-tail outer pipeline probe — 2026-10-18

Добавлен `parity/full_tail.c` со штатным
`set_file_paths/init_angband/player_make_simple/prepare_next_level/on_new_level`
harness. Probe задаёт depth=1 и проходит реальный outer generation pipeline,
возвращая detached depth/player position/monster count/object capacity/RNG index.
Получено completed depth=1 с player/monster/object state; cleanup проходит.

Это strongest combined evidence без прямого `classic_gen` вызова: shared RNG и
phase ordering наблюдаются через штатный outer path, но full phase-by-phase parity,
changed-cell records и native ownership certification ещё не объявлены.

Проверка: `unittest-parity-full_tail` 1/1; targeted full-tail differential test —
1 OK; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: добавить full-tail differential manifest linkage или зафиксировать
remaining missing phase-level evidence.

## Full-tail outer pipeline in evidence manifest — 2026-10-18

`build_generation_tail_manifest` принимает optional `outer_pipeline` и публикует
его detached как `outer_pipeline_status`. Outer result normalizes к
`generation_complete=False`, `parity_status=pending`, `execution_supported=False`;
он не подменяет phase-level evidence и не сертифицирует full tail.

Validator проверяет outer section и completion/parity drift. Full-tail outer probe
через штатный `prepare_next_level` остаётся combined evidence, но phase-specific
changed cells, exact RNG order, ownership и cleanup semantics остаются pending.

Проверка: manifest/prerequisite tests — 2 OK; full-tail differential — 1 OK;
one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: connect actual full-tail probe JSON to manifest, затем audit
missing phase-level evidence и cleanup semantics.

## Actual outer probe linked to phase manifest — 2026-10-18

Python differential test теперь передаёт реальный `NATIVE_PARITY_FULL_TAIL` JSON в
`build_generation_tail_manifest(outer_pipeline=...)`. Manifest публикует
`outer_pipeline_status=observed`, но `observed_count=0` для phase-level evidence:
outer completion не подменяет отдельные rubble/trap/player/monster/object/gold
records. Completion/parity drift по-прежнему отвергается.

Проверка: full-tail manifest differential — 1 OK; manifest/prerequisite tests — 2
OK; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: native phase-level evidence comparison/cleanup audit.

## Phase-level evidence and cleanup audit — 2026-10-18

Combined manifest теперь сохраняет per-phase `cleanup_status` и общий
`cleanup_verified=False`; outer pipeline evidence не подменяет phase evidence и не
подтверждает cleanup. Validator отвергает cleanup/completion/parity drift.
Phase-level probes остаются отдельными: rubble/trap/player/monster/object
records имеют разные ownership boundaries и не объединяются в ложный full parity.

Проверка: manifest/prerequisite/cleanup audit tests — 3 OK; full-tail differential
— 1 OK; compile и git diff --check — OK.
Следующий batch: native phase-level changed-cell/RNG comparison или remaining
cleanup failure-path evidence.

## Phase evidence normalization and RNG/cleanup audit — 2026-10-18

Добавлен `normalize_generation_phase_evidence`: каждая phase record проходит
strict JSON detachment, получает explicit `evidence_status` и default
`cleanup_status=not_observed`, а RNG index проверяется только как opaque range
`0..31`. Draw count, cross-phase RNG order и parity не выводятся.

`build_generation_tail_manifest` использует normalizer для всех шести phases;
invalid/non-JSON evidence не маскируется outer result. Cleanup remains unverified.

Проверка: manifest/prerequisite/normalizer tests — 3 OK; one_room — won, 8/8
фаз; compile и git diff --check — OK.
Следующий batch: native phase-level changed-cell/RNG comparison или remaining
cleanup failure-path evidence.

## Phase failure/cleanup evidence contract — 2026-10-18

Добавлен `validate_generation_phase_failure_evidence`: failed phase обязан иметь
explicit `cleanup_status` (`completed/pending/not_observed`), не может объявлять
generation complete и не может заявлять retry/rollback support. Strict JSON и
side-effect-free semantics проверяются отдельно; cleanup execution не вызывается.

Phase stable-fact comparator и manifest сохраняют parity pending, поэтому outer
failure/cleanup evidence не маскирует отсутствующие phase records.

Проверка: failure/comparator/manifest tests — 3 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: aggregate native probe report with per-phase status или
remaining cleanup/error-path fixtures.

## Native phase stable-fact comparator — 2026-10-18

Добавлен `compare_generation_phase_observation`: сравнивает phase-specific stable
facts без вывода полного native parity. RNG `state_i` проверяется только как
opaque range `0..31`; draw count, cross-phase stream order и cleanup не выводятся.
Comparator покрывает rubble/trap/player/monster/object stable shapes и mismatch
status, всегда оставляя parity pending и execution disabled.

Проверка: comparator test — 1 OK; all native probe targets built; one_room — won,
8/8 фаз; compile и git diff --check — OK.
Следующий batch: remaining cleanup failure-path evidence или полный native phase
stream capture, если можно изолировать его без gameplay mutation.

## Aggregate native probe manifest from actual binaries — 2026-10-18

Python differential helper `_native_tail_probe_manifest` запускает существующие
native probe binaries (rubble, player, monster, object) из `GAME_ROOT`, извлекает
JSON prefixes и строит six-phase manifest. Observed phases не скрывают отсутствие
random_traps; outer/full-tail evidence остаётся отдельным layer. Native RNG indices
проверяются как opaque range, phase facts — через per-probe assertions.

Проверка: manifest differential test — 1 OK; all required native probe targets
built; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: remaining cleanup failure-path fixtures или native phase stream
capture with explicit ordering evidence.

## Native generation event ordering envelope — 2026-10-18

`parity/full_tail.c` слушает существующие native
`EVENT_GEN_LEVEL_START`/`EVENT_GEN_LEVEL_END` events и добавляет их в detached
outer JSON. Differential test проверяет start/end presence и successful end, не
создавая новые gameplay events. Это подтверждает outer lifecycle envelope, но не
выводит порядок внутренних rubble/trap/player/monster/object/gold calls.

Проверка: full-tail event differential — 1 OK; manifest/failure tests — 2 OK;
one_room — won, 8/8 фаз; all native probe targets built; compile и git diff --check —
OK.
Следующий batch: remaining cleanup/error-path fixtures или source-backed internal
phase telemetry, если доступна без изменения gameplay.

## Phase failure/cleanup path verification — 2026-10-18

Failure evidence contract проверен в manifest: failed phase с
`cleanup_status=pending` остаётся observed, но не может объявлять generation
complete, retry или rollback. Это документирует частичное native state без
вызова cleanup и без inference фактического удаления/rollback.

Проверка: failure/comparator/manifest tests — 2 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK. Native full-tail success path остаётся отдельным;
real failure injection пока не добавляется, чтобы не менять gameplay/engine state.

Следующий batch: aggregate native probe report с per-phase status или source-backed
internal phase telemetry без изменения gameplay.

## Aggregate native probe status report — 2026-10-18

Добавлены `build_generation_tail_status_report` и validator. Они публикуют
per-phase `observed/invalid/not_observed` и derived failed count, отдельно от
outer pipeline status. Report strict-JSON, side-effect-free, parity pending и
содержит `pending_counters_unchanged=True`; progress summary публикует его как
`native_generation_tail_status` и проверяет counter immutability.

Проверка: status/manifest/progress tests — 3 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: remaining cleanup/error-path fixtures или source-backed internal
phase telemetry без изменения gameplay.

## Native internal generation telemetry audit — 2026-10-18

`parity/full_tail.c` расширен read-only listeners для существующих
`EVENT_GEN_ROOM_START`, `EVENT_GEN_ROOM_END` и `EVENT_GEN_TUNNEL_FINISHED`. Detached
outer JSON публикует room start/end counts, tunnel event count и aggregate steps;
обычный depth-1 outer path наблюдал `0/0/0`, что фиксируется как отсутствие event
telemetry, а не как доказательство отсутствия rooms/tunnels.

Differential test проверяет только nonnegative accounting и
`room_end_count <= room_start_count`; внутренний six-phase order/parity остаётся
pending.

Проверка: full-tail telemetry differential — 1 OK; status/manifest/progress tests —
3 OK; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: remaining cleanup/error-path fixtures или native source-backed
phase instrumentation boundary.

## Outer generation telemetry validator — 2026-10-18

Добавлен `validate_generation_outer_telemetry` для detached full-tail lifecycle
JSON: boolean level start/end/success envelope, nonnegative room/tunnel counters и
`room_end <= room_start`. Validator не выводит внутренний phase order, не меняет
pending counters и сохраняет parity pending.

Actual full-tail differential теперь использует validator; observed depth-1 path
с `room_start/end/tunnel=0/0/0` принимается как valid telemetry absence, не как
semantic room/tunnel result.

Проверка: full-tail telemetry differential — 1 OK; telemetry/status/progress tests
— 3 OK; compile и git diff --check — OK.
Следующий batch: remaining cleanup/error-path fixtures или native source-backed
phase instrumentation boundary.

## Native source-backed phase instrumentation boundary — 2026-10-18

Source audit подтверждает: existing engine events покрывают lifecycle и room
size/subtype/tunnel internals, но не emit отдельные six-tail calls
(rubble/trap/player/monster/object/gold). Поэтому новые gameplay hooks не
добавлялись. `validate_generation_outer_telemetry` публикует explicit
`telemetry_coverage`: lifecycle observed, room/tunnel observed-or-absent,
tail_phase_calls unavailable.

Это фиксирует instrumentation gap как evidence, а не маскирует его inferred
phase order/parity.

Проверка: full-tail telemetry differential — 1 OK; telemetry/status/progress tests
— 3 OK; one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: remaining cleanup/error-path fixtures или отдельные native
phase hooks только при source-backed безопасной границе.

## Native generation cleanup boundary audit — 2026-10-18

Добавлены `describe_generation_cleanup_boundary` и validator по native
`classic_gen` failure branches. Source-backed actions: `uncreate_artifacts`,
`wipe_mon_list`, `cave_free`; result: null chunk + error text (`less than two
rooms created` или `could not place player`). `rollback_verified=False` сохраняется:
структурный cleanup call sequence не доказывает фактический rollback state.

Cleanup boundary опубликован в progress summary как diagnostic-only metadata.

Проверка: cleanup/telemetry/progress tests — 3 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: remaining native failure injection only if isolated, иначе
finalize native parity pending ledger.

## Native parity pending ledger — 2026-10-18

Добавлены `build_generation_tail_parity_ledger` и validator. Ledger публикует по
каждой из шести tail phases evidence/structural status, cleanup status,
`parity_status=pending` и execution disabled. Даже при observed evidence для
rubble/objects/gold `pending_parity_count` остаётся ровно 6; validator отвергает
counter decrement, completion или phase parity drift.

Ledger подключён к progress summary как immutable diagnostic section.

Проверка: ledger/cleanup/progress tests — 3 OK; `pending_parity_count=6`;
one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: finalize native parity pending ledger и remaining source-backed
blocker inventory.

## Native parity blocker inventory — 2026-10-18

Добавлены `describe_native_parity_blockers` и validator с четырьмя unresolved
source-backed blockers:

1. `phase_semantics` — six tail calls не имеют phase-level telemetry;
2. `rng_order` — shared native RNG stream/draw order не captured;
3. `catalog_ownership` — engine candidate scans/catalog ownership остаются native;
4. `cleanup_verification` — cleanup call sequence есть структурно, rollback state
   не verified.

Каждый blocker содержит category/status/source ref/reason. Inventory подключён к
progress summary и не позволяет объявить generation complete или parity complete.

Проверка: blocker/ledger/progress tests — 3 OK; `pending_parity_count=6`;
one_room — won, 8/8 фаз; compile и git diff --check — OK.
Следующий batch: финальный pending ledger/report audit без искусственного закрытия
native blockers.

## Final native parity pending report — 2026-10-18

Добавлены `build_native_generation_parity_report` и validator. Итоговый immutable
report объединяет six-phase parity ledger, four source-backed blockers и cleanup
boundary. Инварианты: `pending_parity_count=6`, `blocker_count=4`,
`parity_status=pending`, `generation_complete=False`, execution disabled и
pending counters unchanged. Under-reporting, completion drift и parity-complete
claims отвергаются.

Progress summary публикует final report отдельной diagnostic-only секцией.

Проверка: final report/blocker/ledger/progress tests — 4 OK;
`pending_parity_count=6`, blockers=4; one_room — won, 8/8 фаз; compile и
git diff --check — OK.
Следующий batch: maintenance-only audit или новые native probes только при
появлении source-backed evidence; parity не закрыта.

## Final parity report consistency audit — 2026-10-18

Повторно проверены immutable final report/ledger/blocker invariants без изменения
claims: `pending_parity_count=6`, blockers=4, parity pending, execution disabled.
Representative native probes прошли: full-tail 1/1, monster 3/3, object 2/2,
native RNG 8/8, player 1/1, trap 1/1. Actual differential manifest и outer
telemetry валидны; phase-level missing evidence не замаскирована.

`one_room` прошёл 8/8; compile, py_compile и git diff --check — OK. Native parity
остаётся pending.

Следующий batch: maintenance-only audit или новые native probes только при
появлении source-backed evidence; parity не закрыта.

## Progress summary final-report integrity guard — 2026-10-18

Добавлен focused progress test, который требует наличия final native parity report
в aggregate summary и отвергает tampering `pending_parity_count`, `blocker_count`
или `parity_status`. Guard не меняет counters и не расширяет native claims.

Проверка: progress/final-report tests — 3 OK; one_room — won, 8/8 фаз; compile,
py_compile и git diff --check — OK. Native parity остаётся pending.

Следующий batch: maintenance-only audit или новые native probes только при
появлении source-backed evidence; parity не закрыта.

## Release-readiness final consistency audit — 2026-10-18

Tracked model/native files чисты относительно последнего logical state. Final
report guards — 5 OK; native probes: full-tail 1/1, monster 3/3, object 2/2,
native RNG 8/8, player 1/1, trap 1/1. Differential manifest/outer telemetry
valid; `one_room` — won, 8/8 фаз; compile/py_compile/git diff --check — OK.

Инварианты не изменены: pending parity=6, blockers=4, execution disabled,
`generation_complete=False`. Pyright unavailable — diagnostics не являются
runtime/build failure.

Следующий batch: maintenance-only audit или новые native probes только при
появлении source-backed evidence; parity не закрыта.

## Native shared RNG stream boundary audit — 2026-10-18

Source audit `z-rand.c` подтверждает native WELLRNG1024a и `state_i` как opaque
state index `0..31`; это не draw counter. `classic_gen` вызывает множество native
RNG consumers между six tail calls, но public telemetry не экспортирует phase
boundaries или draw sequence. Добавлены `describe_native_rng_stream_boundary` и
validator, запрещающие подменять cursor draw count и сохраняющие parity pending.

Проверка: RNG boundary/final/progress tests — 3 OK; one_room — won, 8/8 фаз;
compile и git diff --check — OK.
Следующий batch: maintenance-only audit или source-backed native stream hook;
искусственная реконструкция RNG order запрещена.

## Native catalog/candidate ownership boundary — 2026-10-18

Source audit подтверждает engine-owned catalogs и scans: objects через `k_info` и
`alloc_objects` (`src/gen-cave.c:1321`), monsters через `r_info`/`lookup_monster`
(`src/mon-util.c:119`), traps через `trap_info`/`lookup_trap` (`src/trap.c:41`).
Добавлены `describe_native_catalog_ownership_boundary` и validator; Python catalog
substitution запрещена, parity остаётся pending.

Boundary включён в final native parity report как отдельная ownership evidence,
без выбора candidates и без execution.

Проверка: catalog/final/RNG/progress tests — 4 OK; py_compile и git diff --check —
OK.
Следующий batch: maintenance-only audit или source-backed native stream hook;
искусственная реконструкция RNG order запрещена.

## Native differential probe path normalization — 2026-10-18

`test_differential.py` получил `native_parity_probe()` с env override и fallback
`build-headless/unittests/parity` → legacy `build/unittests/parity`. Это устраняет
ложные skips при штатной headless CMake сборке для object/monster/player/trap/
native/full-tail probes; native semantics и parity claims не изменяются.

Проверка: native differential manifest/object/monster/full-tail tests — 4/4 OK без
env overrides; catalog/final/RNG/progress tests — 3 OK; one_room — won, 8/8 фаз;
py_compile и git diff --check — OK.
Следующий batch: maintenance-only audit или source-backed native stream hook;
искусственная реконструкция RNG order запрещена.

## Normalized full native differential matrix — 2026-10-18

`test_differential.py` теперь автоматически находит `build-headless` probes и
поддерживает env overrides. Исправлен object/gold accounting assertion: native
`Rand_normal` signed result может быть `-1`, а `alloc_objects` не выполняет
отрицательное количество; differential сравнивает effective `max(requested, 0)`.

Проверка: normalized native matrix — **10/10 OK**, без skips; catalog/final/RNG/
progress tests — OK; one_room — won, 8/8 фаз; py_compile и git diff --check — OK.
Native parity claims не расширены.

Следующий batch: maintenance-only audit или source-backed native stream hook;
искусственная реконструкция RNG order запрещена.

## Native outer failure cleanup/retry semantics — 2026-10-18

Source audit `generate.c:cave_generate` добавил подтверждённые outer failure
semantics к cleanup boundary: builder failure вызывает `cleanup_dun_data`,
`EVENT_GEN_LEVEL_END=false` и retry; post-build overflow вызывает
`uncreate_artifacts`, `cave_clear`, `cleanup_dun_data`, false end event и retry.
Retry loop limit равен 100. Это структурное C evidence, не доказательство
rollback state или успешной failure injection.

Проверка: cleanup/final/progress tests — 3 OK; one_room — won, 8/8 фаз;
py_compile и git diff --check — OK.
Следующий batch: maintenance-only audit или source-backed native stream hook;
искусственная реконструкция RNG order запрещена.

## Explicit native monster install probe — 2026-10-18

Добавлен `parity/monster.c` со штатным init harness, synthetic arena и explicit
`lookup_monster("giant white mouse")` race. Probe вызывает `place_new_monster` без
random `get_mon_num` и без group expansion; detached JSON сообщает race_id=23,
monster_id=1, position `(8,7)`, placed=true и opaque RNG index.

Python differential test проверяет explicit selection mode, race/id/position и RNG
range. Monster boundary/result evidence уже подключены в readiness/progress и не
уменьшают pending callbacks или native execution eligibility.

Проверка: `unittest-parity-monster` 1/1; targeted differential monster test — 1
OK; boundary/progress/object tests — 3 OK; compile и diff-check — OK.
Random race selection, 10000-attempt search, groups/friends и `classic_gen` parity
остаются pending.

Следующий batch: native object/gold probe audit с полным штатным data initialization
или завершение monster random-selection boundary contract.

## Random-trap detached snapshot/health — 2026-10-18

Добавлены `detach_trap_native_result` и `health_trap_native_result`: strict JSON
round-trip отделяет changed-cell evidence, health проверяет counts, уникальность,
interior bounds и opaque RNG state index `0..31`. Draw count не выводится,
execution disabled, parity pending.

Проверка: targeted trap snapshot/health test — 1 OK; compile и git diff --check —
OK.
Следующий шаг: минимальная C trap fixture initialization с catalog/effect/power
state; при неполной fixture остановиться без parity claim.

## Commit ownership consistency with isolation — 2026-10-18

`transfer_commit.inspect_commit_ownership_consistency` сопоставляет source/scratch
owner IDs до и после commit с соответствующими before/after isolation-декларациями
для chunk, player, progress, carried objects, RNG, commands, scheduler и events.
Дополнительно сверяется метка `unchanged`/`transferred` с изменением owner-пары.
Readiness/debug показывают mismatch по компонентам; неполные декларации остаются
unresolved. Согласованность означает только совпадение переданных записей, не
проверку реальных владельцев, mutation или исполнения.

Проверка: 410 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: определить trace/provenance представление частичного commit
failure, чтобы сохранять наблюдаемое состояние после неуспешной передачи.

## External commit/ownership observations — 2026-10-18

Добавлены `transfer_commit.TransferCommitObservation/CommitResourceObservation`
и `inspect_transfer_commit`: фиксируются transfer ID, trace boundary, before/after
manifest digests, ownership result для chunk/player/progress/carried objects/RNG/
commands/scheduler/events, события и возможность частичного изменения состояния.
Pending требует явного unchanged/no-event; failed сохраняет причину и partial state
без retry. After provenance связывает commit record с after manifest; readiness
сверяет before digest с before manifest. Shared ownership — явный конфликт. Commit,
rollback и callbacks не выполняются; status records являются внешним свидетельством.

Проверка: 410 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: сверить владельцев компонент commit с source/scratch
изоляционными декларациями и обозначить неизвестные переходы без их вычисления.

## Isolation manifest provenance — 2026-10-18

`transfer_provenance.bind_transfer_manifest/inspect_transfer_provenance` теперь
принимают role-specific `isolation` и включают digest записи в соответствующий
before/after binding. Проверяются transfer ID и совпадение trace boundary с
manifest boundary; `transfer_readiness` принимает
`isolation={"before": ..., "after": ...}` и показывает результат каждой роли.
Переставленные записи отвергаются. Digest подтверждает лишь структурную связь
данных: object aliases, фактическое владение, runtime creation и выполнение не
проверяются.

Проверка: 406 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: описать внешние результаты commit/ownership handoff и
неопределённое состояние при частичном сбое, без реализации commit или rollback.

## Scratch runtime isolation declarations — 2026-10-18

Добавлен `transfer_isolation.TransferIsolationEvidence/MutableResourceOwnership` и
`inspect_transfer_isolation`: source/scratch runtime IDs, владельцы и alias status
для chunk, RNG, commands, scheduler, events, framework delivery и controller
callbacks привязываются к transfer trace boundary. Общие mutable IDs, один runtime
ID и несовпадение заявленного owner дают конфликты; пропущенные поля остаются
unresolved. Lifecycle descriptor и transfer readiness/debug показывают контракт
и блокеры. Реальные aliases не исследуются, runtime не создаётся, isolation не
подтверждена.

Проверка: 405 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: связывать isolation evidence отдельно с before/after
manifest provenance, чтобы обнаруживать смешение записей между границами.

## RNG handoff manifest provenance — 2026-10-18

`transfer_provenance.bind_transfer_manifest/inspect_transfer_provenance` теперь
принимают role-specific `rng_handoff`. В before/after binding включается digest
точного внешнего свидетельства; перед связыванием проверяются `transfer_id` и
совпадение boundary index с manifest. `transfer_readiness` принимает
`rng_handoff={"before": ..., "after": ...}` и отдельно сообщает статус каждой
связки. Подмена записей между границами даёт failed provenance. Digest и
структурная связность не подтверждают выполнение, корректность policy, изоляцию
или restore; RNG не меняется.

Проверка: 400 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: уточнить lifecycle evidence для изоляции scratch runtime
и ownership transfer, не добавляя executor или неподтверждённые правила.

## Transfer RNG handoff evidence — 2026-10-18

Добавлен `transfer_rng.RngTransferEvidence/inspect_transfer_rng`: внешнее
свидетельство source/scratch RNG state, заявленной handoff policy и lifecycle
boundary index. Проверяются stage/status, ссылки на `RngStateDeclaration`,
`transfer_id` и границы существующего `ChunkTransferTrace`. Отчёт лишь показывает,
совпадают ли opaque states; соответствие policy, изоляция, ownership и restore
не подтверждаются. `transfer_readiness` и `transfer_debug` теперь включают это
свидетельство и блокируют отсутствие/ошибки, но execution остаётся запрещён.
Состояния не копируются, не сбрасываются и не восстанавливаются.

Проверка: 399 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: связать RNG handoff evidence с before/after manifest
provenance, сохраняя отдельные границы данных и не применяя handoff policy.

## Unified RNG debug/readiness — 2026-10-18

Добавлен `rng_debug.build_rng_debug_report`: объединяет каталог интерфейсов,
декларацию, внешнее operation observation и callable binding в JSON-отчёт с
blockers и последовательностью будущей интеграции. Missing evidence остаётся
unresolved; pending/failed trace показывается явно. Даже структурно полный
trace не подтверждает владение, полный call order, state/result semantics или
reproducibility. Ни один handler не вызывается; generator, draws и restore не
выполняются. Отчёт включён в scenario preflight.

Проверка: 395 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: определить external RNG trace/handoff boundary для
transfer debug и связать уже существующие RNG свидетельства без применения policy.

## RNG operation observations — 2026-10-18

Добавлен `rng_observation.RngOperationObservation/inspect_rng_observation`: external
pending/completed/failed records, explicit operation/generator/owner/boundary,
arguments, opaque before/after/result/events и possible mutation. Проверяются known
operation/argument names, identity и before-state agreement с декларацией.
Missing declaration/reference/result evidence остаётся unresolved; explicit zero
и empty records сохраняются. Pending запрещает изменения/события; failed сохраняет
uncertain partial state без retry. Результаты и state_i не используются для
вывода draw count, native result semantics или reproducibility. Descriptor в
preflight. Никаких RNG construction/draw/restore или handoff changes.

Проверка: 391 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.

## RNG state/ownership declarations — 2026-10-18

Добавлен `rng_declaration.RngStateDeclaration/inspect_rng_declaration`: explicit
generator/owner/boundary IDs, seed, opaque state, call-order references и handoff
policy с caller source references. Unknown значения не заменяются defaults;
explicit zero/empty state/empty order сохраняются, duplicate call references
допускаются без draw-count inference. Проверяются basic shapes/known refs/strict
JSON, не native state shape/restore validity или seed-state correspondence.
Ownership, sources, ordering и reproducibility не подтверждаются; handoff text
не применяется. Descriptor в preflight. RNG не создаётся, draws/restore отсутствуют.

Проверка: 386 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: внешние RNG operation observations и declaration/owner/
boundary consistency без draws, restore или вывода draw count из state_i.

## RNG/state API skeleton — 2026-10-18

Добавлен `rng_contract`: восемь existing AngbandRng APIs initialize, snapshot,
restore и random helpers. Параметры сверены с Python signatures; алгоритм,
коэффициенты и defaults не копируются. Initialize/observe/restore/draw layers
разделены. Runtime ownership указан как reference; source/scratch handoff требует
явной policy. state_i — индекс состояния, не monotonic draw counter; nested helper
calls нельзя повторять при reporting. Seed/state не доказывают callback order,
input parity или resumable session. Binding inspection не создаёт RNG, не выбирает
значения и не восстанавливает состояние. Descriptor включён в preflight.

Проверка: 381 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: explicit RNG state/ownership declarations с unknown
seed/state/owner/order evidence, без draws, restore или новой handoff policy.

## Unified summon debug/readiness — 2026-10-18

Добавлен `summon_debug.build_summon_debug_report`: detached strict JSON API/binding,
input evidence и external result. Missing evidence, invalid declarations и external
pending/failed observations различаются. API/layer берутся только из input context,
result не служит fallback. Explicit empty creation и unknown output сохраняются
отдельно. Постоянные blockers фиксируют sources, allocation/placement/unique/RNG
semantics, request/spawn/wave child-call ownership, fallback/order и native execution.
Полные записи не включают выбор расы/позиции, создание entities или retry.
Preflight включает отчёт; extension points используют existing runtime APIs.

Проверка: 377 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: инвентаризация existing RNG/state interfaces и явных
ownership/reproducibility boundaries без изменения генератора или порядка draws.

## Summon result observations — 2026-10-18

Добавлен `summon_result.SummonResultObservation/inspect_summon_result`: external
pending/completed/failed records с plan/selection/spawn/wave layer, отдельными
caster_id и created_ids, opaque before/after/events и possible mutation.
Проверяются API/layer agreement, caster spell-path identity и single-spawn output
ID против summoned_id/request monster_id. Empty created_ids является explicit
no-creation observation, None остаётся unknown; wave IDs не выводятся из prefix.
Missing input evidence сохраняется unresolved. Pending запрещает creation/change/
events; failed сохраняет uncertain partial state без retry. Descriptor в preflight.
Никаких allocation/placement/spawn execution или native-success inference.

Проверка: 372 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified summon debug/readiness с explicit blockers
и extension points без allocation, placement или entity creation.

## Summon input context — 2026-10-18

Добавлен `summon_inputs.SummonInputContext/inspect_summon_inputs`: explicit
arguments, opaque caster/request/type/allocation/placement/unique/RNG/adapter
observations и source references. Проверяются known names, strict JSON, spell-path
caster ID, explicit summon-name/request/event subtype agreement и placement origin.
request_spawn.monster_id — output entity ID, не caster; роли не смешиваются.
Position selection scope не требует вымышленного request/caster. Missing evidence
остаётся unresolved; explicit None/0/empty collections сохраняются без defaults.
JSON request/event/predicate declarations не становятся runtime objects/bindings.
Dependencies являются inspection scope, не доказательством native readiness.
Descriptor в preflight; нет race/position selection или entity creation.

Проверка: 367 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external summon-result observations и API/layer/caster/
created-entity consistency без allocation, placement или spawn execution.

## Summon planning/selection/spawn API skeleton — 2026-10-18

Добавлен `summon_contract`: восемь existing runtime APIs spell/curse planning,
position/race/request-race selection, request spawn, spell spawn и spell wave.
Параметры сверены с Python signatures; defaults/формулы не копируются. Plan,
selection, spawn и wave layers явно разделены. Wave/spawn wrappers владеют child
calls; reporting не повторяет allocation/creation. Map predicates, allocation
observations и unique tracking остаются explicit boundaries. Binding inspection
ничего не вызывает. Каталог частичный, native placement/allocation/fallback parity
не подтверждается. Descriptor подключён к preflight.

Проверка: 362 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: summon-input context с explicit caster/request/type/
allocation/placement/unique/RNG observations без выбора расы или создания сущностей.

## Unified monster spell debug/readiness — 2026-10-18

Добавлен `monster_spell_debug.build_monster_spell_debug_report`: detached strict
JSON API/binding, input evidence и external result. Missing evidence, structural
errors и pending/failed observations различаются. Entry point/layer берутся только
из input context; result не служит fallback. Постоянные blockers сохраняют source/
identity, filters/spell availability/RNG/effect semantics, gate/selection/roll/apply
ownership, native order и cast execution verification. Player-effect application
не получает вымышленного caster. Полные записи не включают selection/rolls/effects
или retry. Preflight integration; extension points используют existing APIs.

Проверка: 358 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: каталог existing summon planning/selection/spawn APIs
и границ allocation/placement ownership без новых правил призыва.

## Monster spell result observations — 2026-10-18

Добавлен `monster_spell_result.MonsterSpellResultObservation/inspect_monster_spell_result`:
external pending/completed/failed records, gate/selection/roll/apply layer,
caster/target/spell identity, opaque before/after/events и possible mutation.
Проверяется agreement с input API/layer и явно указанными identities. Selection
spell_name является output choice, не приравнивается к input spell-state name;
поддержка/доступность выбранного spell не подтверждается. Player-effect application
не требует вымышленного caster. Missing inputs/records остаются unresolved.
Pending запрещает изменения/события; failed сохраняет unknown partial state без
retry. Descriptor в preflight. Нет selection/RNG/effect execution или cast-success inference.

Проверка: 353 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified monster-spell debug/readiness с explicit blockers
и extension points, без выбора, бросков, применения эффектов или новой cast pipeline.

## Monster spell input context — 2026-10-18

Добавлен `monster_spell_inputs.MonsterSpellInputContext/inspect_monster_spell_inputs`:
explicit arguments, opaque caster/target/spell/filter/RNG/adapter observations
и source references. Проверяются known fields, strict JSON и явно указанные
caster ID/target ID/spell-name agreement. Missing evidence остаётся unresolved;
None/0/False сохраняются без defaults или native semantic validation. JSON effect
records/callback declarations не становятся runtime objects/bindings. Dependencies
описывают inspection groups, не обязательные native prerequisites каждой ветки.
Player-effect application scope не придумывает caster или новый RNG roll.
Descriptor в preflight. Нет selection, rolls, casting или effect application.

Проверка: 348 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external monster-spell result observations и entry-point/
layer/caster/target/spell consistency без исполнения или повторного RNG.

## Monster spell API skeleton — 2026-10-18

Добавлен `monster_spell_contract`: шесть existing APIs attempt gate, selection,
failure gate, hit roll, effect-event rolls и application supplied player-effect
records. Required/optional параметры сверены с Python signatures; defaults и
численные правила не копируются. Gate/selection/roll/apply layers разделены,
автоматическая cast pipeline не выводится. Reporting не повторяет RNG/effects.
Blast dispatcher остаётся отдельным reference. Каталог частичный: self spells,
summons и все effect families не покрыты полностью. Binding inspection не вызывает
handlers, не подтверждает native parity. Descriptor подключён к preflight.

Проверка: 343 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: monster-spell input context с explicit caster/target/spell/
filter/RNG/adapter observations без выбора, бросков или исполнения заклинаний.

## Blast result integration into readiness — 2026-10-18

`build_blast_dispatch_debug_report` теперь принимает keyword-only observation
и включает existing result inspection в sections.result. Input context получается
из result inspector без повторной input inspection; binding errors дедуплицируются.
Missing result/state/identity показаны отдельным result_evidence_unresolved blocker,
внешние pending/failed — отдельными external_result blockers. Invalid identity
остаётся structural error; partial failed state сохраняется unknown. Result не
подменяет missing inputs. Completed records остаются external evidence, не native
execution proof. Существующий preflight получает result-aware report без fake result.

Проверка: 339 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: каталог existing monster spell selection/cast APIs и
границ target/roll/dispatch ownership без изменения магии или численных правил.

## Blast dispatcher result observations — 2026-10-18

Добавлен `blast_dispatch_result.BlastDispatchResultObservation/inspect_blast_dispatch_result`:
external pending/completed/failed records для фиксированного existing dispatcher.
Проверяются source ID, supplied target ID и caller-selected target position против
blast input context. Position не вычисляется; target cell не требует вымышленного
actor. Opaque before/after/events сохраняются detached strict JSON. Missing input/
requirement evidence остаётся unresolved даже при полном result. Pending запрещает
изменения/события; failed сохраняет неизвестное частичное состояние без retry.
Descriptor в preflight. Никаких geometry/falloff/damage/effects или replay child calls.

Проверка: 335 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: подключить result observations к unified blast debug/readiness
с отдельными blockers внешнего failure, missing state и native execution evidence.

## Unified blast dispatcher input readiness — 2026-10-18

Добавлен `blast_dispatch_debug.build_blast_dispatch_debug_report`: общий detached
strict JSON input/requirement/binding inspection. Отдельные blockers для missing
arguments/state/references/reduction, unknown requirements и required missing hooks.
Binding errors дедуплицируются; optional absent hooks не становятся ошибкой.
Постоянные blockers сохраняют sources/conditions/input semantics, hook signatures,
child-effect ownership/native RNG order и execution verification. Полные декларации
не разрешают dispatcher execution. Preflight включает отчёт. Future points:
identity → inputs → bindings → ownership → verify через existing runtime.

Проверка: 330 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: внешние blast-dispatch result observations и согласованность
с source/target/input context, без geometry/damage/effect execution или retry.

## Blast dispatcher input context — 2026-10-18

Добавлен `blast_dispatch_inputs.BlastDispatchInputContext/inspect_blast_dispatch_inputs`:
explicit data arguments, отдельный player_reduction, opaque projection context,
references и conditional adapter requirement records. Callbacks передаются отдельно
в bindings, не в JSON arguments. Requirements unknown/required/not_required являются
caller declarations; explicit states требуют reference, native conditions не
выводятся. Required missing binding остаётся unresolved; bound hook не подтверждает
условие и не исполняется. Проверяются source identity/target position и strict JSON.
Missing evidence/None/0/False сохраняются. Descriptor включён в preflight; никакой
geometry/damage/effect execution или reduction semantics validation.

Проверка: 325 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified blast-dispatcher readiness/debug с input/requirements/
binding blockers, без расширения численных правил и без вызова dispatcher.

## Blast dispatcher signature/binding skeleton — 2026-10-18

Добавлен `blast_dispatch_contract`: полный static signature существующего
apply_monster_projection_blast, positional/keyword-only параметры, **player_reduction
и девять optional callback hooks. Значения defaults не копируются. Binding inspector
проверяет имена/callable presence без вызовов; missing optional hook не считается
отсутствием built-in dispatch. Python signature/annotations сверены тестами.
Preflight публикует descriptor; projection debug включает отдельную dispatcher
section и binding errors. Старый blocker incomplete signature заменён на
coverage_unverified: conditional adapters и actor-effect execution не подтверждены.
Общий six-entry projection catalog остаётся отдельным partial catalog.

Проверка: 320 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: explicit blast-dispatcher input context и conditional
adapter observations без geometry/damage/effect execution.

## Unified projection debug/readiness — 2026-10-18

Добавлен `projection_debug.build_projection_debug_report`: detached strict JSON
API catalog, binding, inputs и external result. Missing evidence, invalid contracts
и pending/failed observations различаются. API/layer выбираются только из input
context. Постоянные blockers сохраняют source/input semantics, geometry/occupancy,
dispatch/child-effect ownership, ordering и execution verification. Отдельный
blocker фиксирует reference-only blast dispatcher: full signature/actor-effect
coverage не каталогизированы. Geometry/LOS не получают вымышленного actor.
Полные записи не включают paths/falloff/damage/effects или retry. Preflight integration.
Future points: identity → inputs → ownership → apply → verify через existing APIs.

Проверка: 316 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: полный signature/binding контракт existing blast dispatcher
и его adapter boundaries без переноса или изменения численных алгоритмов.

## Projection result observations — 2026-10-18

Добавлен `projection_result.ProjectionResultObservation/inspect_projection_result`:
внешние pending/completed/failed записи, explicit query/geometry/effect layer,
source/target identity, target/effect-cell/center position, opaque before/after
и events/mutation observations. Проверяются API/layer и явно указанные identities/
positions против input context. Position не является вычисленным path endpoint.
Geometry не получает вымышленного actor; LOS target set не требует одиночного ID
или позиции. Missing input evidence остаётся unresolved. Pending запрещает
изменения/события; failed сохраняет unknown partial state без retry. Descriptor
в preflight. Нет path/falloff/damage/dispatch/effect execution или native parity claim.

Проверка: 311 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified projection debug/readiness с явными blockers
и точками расширения существующих APIs, без включения исполнения.

## Projection input context — 2026-10-18

Добавлен `projection_inputs.ProjectionInputContext/inspect_projection_inputs`:
explicit arguments, opaque source/target/geometry/occupancy/effect/adapter evidence
и caller references. Проверяются известные поля, strict JSON, source monster ID,
явные start/target/position/center и projection-code agreement. Missing evidence
остаётся unresolved, None/0/False сохраняются без defaults и проверки native value
semantics. JSON events/targets/callback declarations не становятся runtime objects.
Inspection dependencies не подтверждают conditional runtime prerequisites. Blast
geometry scope не требует вымышленного actor. Descriptor подключён к preflight;
нет lookup, path/falloff calculations, dispatch или effects.

Проверка: 306 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external projection-result observations и API/layer/source/
target consistency без разрешения геометрии или повторного применения эффектов.

## Projection API skeleton — 2026-10-18

Добавлен `projection_contract`: шесть existing APIs projectability, blast geometry,
monster path, floor-object effects, terrain effects и supported LOS effect.
Параметры сверены с Python signatures; defaults/формулы не копируются. Query,
geometry и effect layers разделены. Existing apply_monster_projection_blast указан
как dispatch reference, без заявления о полном signature/actor-effect coverage.
Dispatcher владеет дочерними geometry/effect calls, повторный replay не допускается.
Target selection и engine/map hooks остаются явными boundaries. Binding inspection
ничего не вызывает; partial catalog не подтверждает native parity. Preflight integration.

Проверка: 301 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: projection-input context с explicit source/target/geometry/
occupancy/effect/adapter observations, без вычисления путей, falloff или эффектов.

## Unified trap debug/readiness — 2026-10-18

Добавлен `trap_debug.build_trap_debug_report`: detached strict JSON API catalog,
binding, input evidence и external result. Структурные ошибки, missing evidence
и pending/failed наблюдения различаются. Entry point берётся только из input
context; result не служит fallback. Постоянные blockers сохраняют непроверенные
identity/sources, effect/timing/RNG/adapter semantics, activation/effect/post-move/
delayed ownership, timeout cadence и native execution. Timeout scope не получает
вымышленного grid. Полные записи не включают activation/dice/effects/timeouts/retry.
Preflight включает отчёт. Future steps: identity → inputs → ownership → apply → verify
через existing APIs с сохранением built-in dispatch.

Проверка: 297 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: каталог existing projection geometry/dispatch/effect APIs
и границ ответственности без новых численных правил проекций.

## Trap result observations — 2026-10-18

Добавлен `trap_result.TrapResultObservation/inspect_trap_result`: внешние
pending/completed/failed записи, explicit grid/chest/trap identity, opaque
before/after, события и possible mutation. Проверяется agreement с input entry
point и явно указанными position/object_id/trap id. Для individual effect trap_id
остаётся caller diagnostic identity, а не новым требованием native handler.
Timeout collection scope не требует вымышленного grid/ID. Missing input evidence
и состояния остаются unresolved. Pending запрещает изменения/события; failed
сохраняет unknown partial state без retry. Descriptor в preflight. Нет activation,
dice, effects или timeout mutation; structure_valid не доказывает native execution.

Проверка: 292 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified trap debug/readiness с blockers и точками
расширения существующих APIs без включения исполнения.

## Trap input context — 2026-10-18

Добавлен `trap_inputs.TrapInputContext/inspect_trap_inputs`: explicit arguments,
opaque grid/chest/trap/effect/player/timing/RNG/adapter observations и source
references. Проверяются известные поля, strict JSON и согласованность явно
указанных grid position/chest ID/trap ID. Missing evidence остаётся unresolved;
None/False/0 сохраняются без подстановки defaults. JSON effects/callback declarations
не становятся runtime objects/bindings. Timeout scope не требует вымышленного
chest или RNG. Dependency groups являются inspection scope, не подтверждением
native conditional prerequisites. Descriptor в preflight. Нет activation, dice,
effects, timeout mutation или source verification.

Проверка: 287 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external trap-result observations и entry-point/identity
consistency без повторной активации, бросков, эффектов или изменения timeout.

## Trap activation/effect API skeleton — 2026-10-18

Добавлен `trap_contract`: четыре существующих runtime APIs grid activation,
individual effect application, chest activation и timeout processing. Параметры
сверены с Python signatures, defaults не копируются. Activation владеет dispatch;
individual effect не заменяет gates/order. Chest activation не подменяется batch
materialization/replay. Delayed/post-move вызовы и world timeout cadence отделены.
Existing built-in handlers не объявлены отсутствующими: optional adapters остаются
опциональными hooks. Callable inspection ничего не вызывает. Каталог частичный,
не подтверждает native parity. Descriptor подключён к preflight.

Проверка: 282 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: trap-input context с явными grid/chest/effect, timing,
RNG и adapter observations без бросков, эффектов или изменения timeout.

## Unified movement/terrain debug/readiness — 2026-10-18

Добавлен `movement_debug.build_movement_debug_report`: detached strict JSON
каталога API, binding, input context и external result. Структурные ошибки,
missing evidence и pending/failed результаты различаются. Entry point/layer
берутся только из input context; внешний result не служит fallback. Постоянные
blockers сохраняют непроверенные identity/sources, geometry/occupancy/action
semantics, movement/combat/post-move ownership, scheduler/energy order и native
execution. Query не получает вымышленного actor. Полные записи не включают
movement, terrain effects, callbacks или retry. Отчёт подключён к preflight.
Точки расширения: identity → inputs → ownership → apply → verify через existing APIs.

Проверка: 278 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: каталог существующих trap activation/effect APIs и
границ post-move/delayed обработки без новых правил ловушек или численных эффектов.

## Movement/terrain result observations — 2026-10-18

Добавлен `movement_result.MovementResultObservation/inspect_movement_result`:
внешние pending/completed/failed результаты, actor identity, explicit layer,
opaque before/after и events/mutation observations. Проверяются API/layer agreement
и actor consistency с input state, monster_id или proposal monster_id. Queries
не требуют вымышленного actor. Missing inputs/layer/state остаются unresolved.
Pending требует отсутствия изменений/событий; failed сохраняет неизвестное
частичное состояние без retry. Descriptor в preflight. Никаких вычислений позиции,
passability, LOS, terrain effects, combat или energy consumption. Structure_valid
подтверждает только форму/согласованность, не native execution.

Проверка: 273 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified movement/terrain debug/readiness с blockers
и точками расширения существующих API без включения исполнения.

## Movement input context — 2026-10-18

Добавлен `movement_inputs.MovementInputContext/inspect_movement_inputs`: explicit
arguments, opaque actor/geometry/occupancy/adapter observations и source references.
Проверяются имена, strict JSON, command-name соответствие move/open, наличие
объявленных dx/dy или proposal action/monster_id и согласованность monster identity.
Missing evidence остаётся unresolved; None/0/False сохраняются без семантического
подтверждения. JSON command/proposal/callback declarations не становятся runtime
objects или bindings. Queries требуют geometry evidence без вымышленного actor.
Dependency groups описывают inspection scope, не native conditional prerequisites.
Descriptor подключён к preflight. Нет lookup, pathfinding, movement или energy use.

Проверка: 268 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external movement/terrain result observations и
entry-point/layer/actor consistency без вычисления позиций или применения эффектов.

## Movement/terrain API skeleton — 2026-10-18

Добавлен `movement_contract`: шесть существующих Python runtime entry points:
player move/open, monster proposal/candidate и terrain predicates/LOS queries.
Required/optional параметры и **options сверены с signatures; defaults не
копируются. Command, proposal, candidate и query layers явно разделены. Existing
occupancy/terrain/post-move adapters остаются точками интеграции без вызовов;
инспектор не перемещает actors, не исполняет combat/traps и не списывает энергию.
Каталог частичный: query LOS не равен full perception, candidate не равен full
scheduler turn. Native parity/ownership не подтверждены. Descriptor в preflight.

Проверка: 263 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: movement-input context и явные actor/geometry/occupancy/
adapter observations без реализации новых правил движения.

## Unified inventory-transfer debug/readiness — 2026-10-18

Добавлен `inventory_debug.build_inventory_debug_report`: общий detached strict JSON
каталога API, binding, input context и external result. Структурные ошибки, missing
evidence и pending/failed результаты различаются. Entry point/layer берутся только
из input context; result не служит fallback. Постоянные blockers сохраняют
непроверенные sources/identity, input semantics, ownership/order, transfer/stacking/
placement и dispatcher energy consumption. Полные наблюдения не включают execution
или retry. Gold не получает вымышленного object ID. Preflight включает отчёт.
Точки расширения: identity → inputs → ownership → apply → verify, через existing APIs.

Проверка: 259 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: инвентаризация существующих movement/terrain-interaction
API и их границ ответственности без новых правил перемещения или энергии.

## Inventory-transfer result observations — 2026-10-18

Добавлен `inventory_result.InventoryResultObservation/inspect_inventory_result`:
внешние pending/completed/failed результаты, source-object identity, direct/command
layer, opaque before/after, события и возможная мутация. Проверяются entry point,
layer и source-object agreement с direct arguments или JSON command payload.
Output stack IDs не выводятся из source ID; gold не требует вымышленного object ID.
Missing input evidence/layer/state остаются unresolved. Pending требует отсутствия
изменений/событий; failed сохраняет неизвестное частичное состояние без retry.
Descriptor подключён к preflight. Transfer, stacking, ownership и energy semantics
не вычисляются; structure_valid не является подтверждением native execution.

Проверка: 254 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified inventory-transfer debug/readiness с blockers
и точками расширения существующих API, без включения исполнения.

## Inventory-transfer input context — 2026-10-18

Добавлен `inventory_inputs.InventoryInputContext/inspect_inventory_inputs`:
явные arguments, opaque object/player/ownership/slots/placement observations
и caller references. Проверяются известные поля, strict JSON, object identity
и соответствие JSON command-name выбранному command entry point. JSON command
не становится runtime PlayerCommand. Missing evidence остаётся unresolved,
явные None/0 сохраняются без семантического подтверждения. Location=None не
подтверждает ownership; gold-path не требует вымышленного object id. Dependencies
являются inspection groups, условные runtime prerequisites пока не проверяются.
Preflight публикует static descriptor. Никаких transfer/energy/stacking вычислений.

Проверка: 249 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external inventory-transfer result observations и
identity/layer consistency без перемещения предметов или повторного energy charge.

## Pickup/inventory-transfer API skeleton — 2026-10-18

Добавлен `inventory_contract`: девять существующих runtime APIs для прямых
pickup/gold/drop/takeoff/wield и четырёх command wrappers. Каталог содержит
required/optional параметры, layer и ответственность. Command wrappers владеют
своими direct calls; повторный перенос не составляется автоматически. Energy
override declaration отделён от dispatcher consumption. Location=None сама по
себе не доказывает player ownership. Callable inspection не вызывает callbacks,
не перемещает предметы и не списывает энергию. Каталог частичный, native parity
не подтверждается. Static descriptor подключён к preflight.

Проверка: 244 contract tests, включая Python signature compatibility; one_room
won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: inventory-transfer input context и явные ownership/slot/
placement observations без новых stacking, allocation или energy rules.

## Unified progression debug/readiness — 2026-10-18

Добавлен `progression_debug.build_progression_debug_report`: detached strict JSON
каталога API, callable binding, input context и внешнего result. Ошибки структуры,
missing evidence и pending/failed наблюдения различаются. Result не используется
как fallback для выбора entry point. Постоянные blockers сохраняют непроверенные
identity/sources, input semantics, ownership/cadence/level-refresh order и native
execution. Даже полные записи не разрешают XP/level/drain processing или retry.
Preflight включает отчёт. Будущие точки расширения: identity → inputs → ownership
→ apply → verify, через существующие runtime handlers.

Проверка: 240 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: каталог существующих pickup/inventory-transfer APIs
и границ владения предметами без новых правил allocation, stacking или энергии.

## Progression result observations — 2026-10-18

Добавлен `progression_result.ProgressionResultObservation/inspect_progression_result`:
внешние pending/completed/failed записи, player identity, отдельный monster-source
identity, opaque before/after, события и признак возможной мутации. Проверяется
согласованность с input entry point и явно указанными identities. Для monster_drain
отсутствующий source остаётся unresolved. Pending требует отсутствия изменений
и событий, failed сохраняет неизвестное частичное состояние без retry.
Недостающие input evidence сохраняются даже при полном result. Descriptor в
preflight; XP, thresholds, level refresh и drain не вычисляются. Structure_valid
не подтверждает native execution или корректность игровых правил.

Проверка: 235 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: unified progression debug/readiness с явными blockers
и точками расширения существующих API без включения исполнения.

## Progression input context — 2026-10-18

Добавлен `progression_inputs.ProgressionInputContext/inspect_progression_inputs`:
явные direct arguments, opaque state observations и caller source references.
Inspection groups описаны для gain/loss, monster drain и equipment drain; они
не подтверждают необходимость всех входов на каждой условной runtime ветке.
Проверяются известные имена, strict JSON и согласованность monster_id с указанным
monster-state id. Missing map/key остаются unknown, явные 0/False/None сохраняются.
JSON roll evidence не является runtime MonsterAttackRoll и не передаётся handler.
Источники/значения/ownership не подтверждаются, XP/threshold/level/drain не
вычисляются. Static descriptor включён в preflight.

Проверка: 230 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: external progression-result observations и согласованность
с entry point/input context без повторного начисления или снятия XP.

## Experience/level progression API skeleton — 2026-10-18

Добавлен `progression_contract`: четыре существующих runtime API gain/loss,
monster drain и equipment drain, их required/optional параметры и ответственность.
PlayerProgress и state helpers указаны отдельно: helpers не заменяют runtime
refresh/events. Kill/drain pipelines владеют собственными XP updates; observational
progression reporting не должно начислять/снимать XP повторно. Dungeon depth
transitions не входят в этот каталог. Callable binding проверяется без вызова;
ownership, cadence и native parity остаются unverified. Каталог явно частичный,
значения по умолчанию и численные правила не копируются. Интеграция в preflight.

Проверка: 225 contract tests, включая сравнение Python signatures; one_room won,
8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: explicit progression-input context и зависимости выбранного
API, без XP/threshold/level calculations и выполнения drain.

## Unified damage/death debug/readiness — 2026-10-18

Добавлен `damage_debug.build_damage_debug_report`: общий detached strict JSON
каталога API, callable binding, input context и external result. Ошибки структуры,
missing evidence и pending/failed внешние результаты показаны отдельно. Entry point
берётся только из input context; external result не служит fallback. Постоянные
blockers сохраняют неизвестные identity/sources, input semantics, ownership/order
и native execution. Даже полные записи не включают callbacks, damage/death,
XP, drops, cleanup или retry. Отчёт включён в preflight.
Точки будущего расширения: identity → inputs → ownership → apply → verify.

Проверка: 221 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: инвентаризация существующих experience/level/progression
API и их границ ответственности, без новых численных правил.

## Damage/death result observations — 2026-10-18

Добавлен `damage_result.DamageResultObservation/inspect_damage_result`: внешние
pending/completed/failed записи с target identity, opaque before/after, событиями
и неизвестным либо явным признаком возможного изменения состояния. Проверяются
entry-point/target consistency с DamageInputContext и pending-инварианты без
изменений/событий. Missing input evidence остаётся unresolved даже при полном
result. Failed сохраняет неопределённость частичного состояния, без retry.
Strict JSON descriptor опубликован в preflight. HP/death/XP/drop/cleanup semantics
не вычисляются; structure_valid не подтверждает native execution.

Проверка: 216 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: единый damage/death debug/readiness, явные blockers
и точки расширения существующих API без включения исполнения.

## Damage input context — 2026-10-18

Добавлен `damage_inputs.DamageInputContext/inspect_damage_inputs`: явные direct
arguments, opaque observations и caller references. Для каждого entry point
описаны группы target/race/progress/RNG/drop context; это inspection dependencies,
а не подтверждение обязательности всех данных на каждой условной runtime ветке.
Проверяются известные имена, strict JSON и согласованность monster_id с указанным
id target-state. Missing map/key остаются unknown; явные 0/None сохраняются.
Наличие ключа не подтверждает допустимость значения. Источники не верифицируются,
lookup, defaults, damage и death execution отсутствуют. Static descriptor в preflight.

Проверка: 211 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: внешние damage/death result observations и consistency
с выбранным entry point, без вычисления HP, XP, drops или cleanup.

## Damage/death API skeleton — 2026-10-18

Добавлен `damage_contract`: каталог шести существующих Python runtime entry points
для player HP, monster damage, ranged damage, kill/drop pipelines и carried drops.
Указаны обязательные/опциональные параметры, наличие **options и ответственность;
значения по умолчанию не копируются. `inspect_damage_interface` проверяет выбор
интерфейса и callable binding, ничего не вызывает и не подтверждает ownership.
Каталог явно неполный; не является доказательством native parity. Preflight
публикует каталог. Перекрывающиеся kill/reward entry points нельзя автоматически
составлять в повторную цепочку обработки смерти.

Проверка: 206 contract tests, включая соответствие реальным Python signatures;
one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: явный damage-input context и зависимости entry points,
без расчёта урона, lookup сущностей и исполнения смерти.

## Unified timed debug/readiness — 2026-10-18

Добавлен `timed_debug.build_timed_debug_report`: общий JSON отчёт API boundaries,
handler binding, declaration и external transition observation. Отдельно показаны
ошибки структуры, отсутствующие входы, pending/failed внешние наблюдения и постоянные
блокировки identity/owner, input semantics, cadence и execution verification.
Даже полные записи не разрешают исполнение; callbacks, декремент, эффекты и retry
не запускаются. Preflight включает отчёт без создания вымышленных наблюдений.
Будущие точки расширения: identity → inputs → cadence → transition → verify.

Проверка: 202 contract tests; one_room won, 8/8 фаз; оба git diff --check чистые.
Следующий этап каркаса: инвентаризация существующих damage/death интерфейсов
и описание границ владения без расширения численных правил.

## Timed-transition observation contracts — 2026-10-18

Добавлен `timed_observation.py`: внешние pending/completed/failed observations,
opaque состояния before/after, события и признак возможного изменения состояния.
Проверяются scope, actor, boundary, turn, единственный объявленный owner и
совпадение before с объявленным состоянием. Pending требует отсутствия изменений
и событий. Структурное согласование не подтверждает исполнение или подлинность
owner; декремент, cadence и эффекты не вычисляются, retry не запускается.
Descriptor подключён к существующему timed-разделу preflight.

Проверка: 197 contract tests; one_room won, 8/8 фаз; diff --check обеих репозиториев.
Следующий этап каркаса: unified timed debug/readiness без включения исполнения.

## Status/timer boundary contracts — 2026-10-18

Добавлен timed_contract.py: descriptor трёх existing APIs (monster boundary,
player timed decrement, equipped curse tick) и TimedBoundaryDeclaration.
Caller задаёт scope, actor/boundary IDs, optional turn, единственного owner,
timer_state и external_inputs. Missing state/inputs unknown, explicit empty/zero
сохраняются. Два owners, invalid names/shape/non-JSON data invalid. State dicts
opaque: timer values/limits/decay/resistance/cadence не вычисляются.

Optional handler presence не запускает callback и не доказывает ownership.
Preflight включает static timed_boundaries без runtime/actor lookup. Конструкция
не назначает владельца/частоту автоматически и не выполняет timer transitions.
SUBSYSTEM_INTERFACES.md обновлён с API/wiring/gaps.

5 новых tests: signature compatibility, missing state, explicit zero/empty inputs/
detached data без calls, multiple owners/invalid inputs, unresolved ownership.
Единая проверка: 192 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock/inspect, Bash launcher, Git. MCP,
Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Native timed rules, values и gameplay processing не менялись.

Следующий шаг каркаса: external timed-transition observations и identity/owner
consistency, без выполнения переходов или подстановки частоты/значений.

## Unified item-effect debug/readiness — 2026-10-18

Добавлен item_effect_debug.py: build/format APIs объединяют command route,
ObjectEffect/ObjectEffectRoll records, adapter bindings и choice/aim context.
Versioned detached strict JSON содержит summary, errors, concrete blockers и
5 future steps identity→inputs→roll→apply→verify. Steps ссылаются на existing
handlers/data, не включают execution и не подставляют effect/target/choice.

Preflight включает item_effect_debug без source lookup, command creation,
rolling или callback calls. Direct-handler и dispatcher layers сохраняют разные
binding diagnostics. Полные declarations pending; semantics/provenance/application
остаются unverified. SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: missing evidence/ordered dependencies, full declarations без
choice/effect calls, layer distinction, invalid/detached formatter и preflight
без fake command. Единая проверка: 187 tests прошли, one_room won, 8/8 фаз;
оба diff-check чистые. Использованы rg/чтение, Python/unittest/mock, Bash launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались. Новых effect/numerical implementations не добавлялось.

Следующий шаг каркаса: status/timer contracts через существующие timed APIs,
с явным owner/input context и без выполнения или расширения игровых эффектов.

## Item-effect choice/aim context — 2026-10-18

Добавлен item_effect_context.py: ItemEffectContext описывает optional object/effect
identity, aim/choice requirements unknown/required/not_required, source references,
direction/target observations и choice count/index. Явные requirements требуют
source/reason reference. Missing required direction/count/index — unresolved;
неверные формы/индекс за пределами declared count — invalid. Native direction
semantics и реальная необходимость aim/choice не вычисляются.

Item adapter inspection принимает context, сверяет object ID с command и effect
index с supplied roll. Descriptor включён в route effect metadata/preflight.
Полностью declared context pending/execution_allowed=False; choice/aim/effect
callbacks не вызываются. SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: missing requirements, explicit zero inputs, unresolved/invalid
references/index, object/roll consistency и complete context без choice calls/
shared mutation. Единая проверка: 182 tests прошли, one_room won, 8/8 фаз;
оба diff-check чистые. Использованы rg/чтение, Python/unittest/mock, Bash launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались. Новых игровых алгоритмов или default параметров нет.

Следующий шаг каркаса: unified item-effect debug/readiness и existing handler
extension points, без target resolution/choice execution/effect application.

## Item-effect adapter contracts — 2026-10-18

Добавлен item_effect_contract.py: ItemEffectAdapters описывает use-item roll handler,
activation text/roll handlers и choice handler. Inspector переиспользует существующие
ObjectEffect/ObjectEffectRoll, проверяет basic shape и name agreement, копирует
records через strict JSON. Dice/expressions не вычисляются, effect names не
объявляются supported по наличию строки, object lookup не выполняется.

Direct execute_use_item_command требует explicit effect handler; existing runtime
process_player_command path описан отдельно, без вызова/установки defaults.
Activation handler selection и choice/aim остаются unresolved. Metadata прикреплена
к use_item/activate routes/preflight. Existing runtime и callbacks unchanged;
SUBSYSTEM_INTERFACES.md обновлён с API/wiring/gaps.

5 новых tests: direct vs dispatcher, detached existing records/no dice,
full bindings без execution/object lookup, invalid records/routes/hooks и metadata.
Единая проверка: 177 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Новые игровые эффекты, формулы или зависимости не добавлялись.

Следующий шаг каркаса: explicit choice/aim input context для item-effect adapters,
без автоматического выбора/нацеливания или исполнения effects.

## Combined player-command debug/readiness — 2026-10-18

Добавлен player_command_debug.py: catalog-only режим и opt-in external command
evidence report. Sections объединяют routes, payload/options dependencies и
result observations; concrete blockers различают missing inputs/hooks/result,
unconnected scenario route, unverified semantics/dispatch/energy. Invalid,
unmapped, external failed и pending разделены. Полный evidence не включает dispatch.

Preflight включает player_command_debug без создания команд. Четыре future
extension points command→inputs→dispatch→verify используют existing builders,
runtime.process_player_command и TurnAdapterBundle.player_action. В карту добавлен
существующий dispatcher; direct handler requirements отделены от его adapters.
Callbacks/queue/runtime energy ownership не менялись. Карта обновлена.

5 новых tests: no invented catalog command/result, complete evidence без callback/
dispatch, missing vs external failure, invalid/unmapped/deep-copy и preflight mode.
Единая проверка: 172 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Новых gameplay/energy algorithms и dependencies нет.

Следующий шаг каркаса: effect-handler adapter contracts для use_item/activate
с переиспользованием существующих effect records и runtime handlers.

## Player-command result/energy observation contract — 2026-10-18

Добавлен player_command_result.py: PlayerCommandResult фиксирует external
pending/completed/failed, reason, mutation/turn/events, override state
unknown/absent/provided, optional energy_override и energy_charged. Unknown None
не заменяется requested energy; explicit zero override сохраняется. Inspector
сверяет command name и форму observations, не вычисляет стоимость, не читает
runtime state и не извлекает _energy_use_override из исходного payload.

Pending требует explicit unchanged-state/no-event/no-known-charge observation;
failed сохраняет unknown partial mutation без retry. Completed остаётся внешней
декларацией, execution/energy semantics unvalidated. Descriptor подключён к route
metadata/preflight. Callback/handler/scheduler code unchanged. Карта обновлена.

5 новых tests: unknown/absent/zero override и unchanged payload, external completion/
detached events, pending/failed state consistency, invalid/mismatched observations,
route metadata/missing result. Единая проверка: 167 tests прошли, one_room won,
8/8 фаз; diff-check чистый. Использованы rg/чтение, Python/unittest, Bash launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались. Численные механики/формулы не расширялись.

Следующий шаг каркаса: combined player-command debug/readiness report и будущие
точки dispatch integration, не исполняя команды и не меняя energy ownership.

## Player-command payload/options dependencies — 2026-10-18

Добавлен player_command_inputs.py: selected direct payload keys и required
callable options для 11 routes, плюс conditional inputs. Примеры: move dx/dy,
object commands object_id, ranged projectile/target/range/pierce/attack_kwargs,
throw без pierce input, use_item explicit effect_handler. Это описание существующих
Python handlers, не новые native правила/формулы и не полный semantic validator.

Inspector показывает missing keys/hooks и provided option names без вызовов и
подстановки defaults. Key presence (даже с value=None) отделена от value validity;
полностью заполненный контракт pending/execution_allowed=False. Условные options
и energy остаются в existing runtime handlers. Route catalog/preflight включают
dependency_contract; реальные callbacks/dispatch не менялись. Карта обновлена.

5 новых tests: selected-route coverage/required effect signature, missing inputs,
presence vs semantics/no callback calls, invalid hooks/unmapped routes, detached
descriptors. Единая проверка: 162 tests прошли, one_room won, 8/8 фаз; diff-check
чистый. Использованы rg/чтение, Python/unittest/mock/inspect, Bash launcher, Git.
MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Игровые механики и numerical параметры не расширялись.

Следующий шаг каркаса: external command-result и energy-override observations,
без расчёта энергии и без изменения существующего handler/scheduler поведения.

## Player-command routing contract — 2026-10-18

Добавлен player_command_contract.py: describe/inspect APIs описывают 11 selected
existing handlers (move, wait, pickup, drop, takeoff, wield, ranged_attack, throw,
use_item, activate, open). Metadata различает runtime API availability и current
scenario wiring: encounter move/wait/pickup, campaign move. Registry не является
полным перечнем Angband commands и не выполняет dispatch.

Inspector принимает существующий PlayerCommand, проверяет basic name/energy/
JSON payload и optional callable declarations. Explicit energy/payload сохраняются,
стоимость не вычисляется; existing runtime energy overrides не меняются. Handler
signatures/payload semantics не валидируются. Unknown command unmapped без fallback.
Preflight включает player_command_routes. Existing MVP routing callbacks unchanged.
SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: handler references/active route map, no invocation/deep copy/explicit
energy, known-but-unconnected routes, unmapped/bad bindings, static preflight.
Единая проверка: 157 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Игровые алгоритмы/атаки/энергетические формулы не добавлялись и не менялись.

Следующий шаг каркаса: handler payload/options dependency contracts для selected
routes, без подстановки значений и без выполнения player commands.

## Profiled-monster factory binding inspection — 2026-10-18

Добавлен profiled_monster_contract.py: inspector проверяет declarations
существующей make_profiled_monster_runtime_action — required mind_for/observe,
optional options/executors, explicit finite dt и timed owner. Missing dt остаётся
unknown, factory default не подставляется. Callable presence не подтверждает
signatures/semantics/coverage. Factory не конструируется, hooks не вызываются.

Inner timed_options и outer_timed_boundary нельзя объявить одновременно; owner
conflicts invalid. Owner без соответствующего binding остаётся unresolved.
Modeled monster policy/debug включают report и blockers; scenario callbacks
не принимают factory overrides. Runtime/scheduler/timed algorithms не менялись.
SUBSYSTEM_INTERFACES.md обновлён с API/wiring/gaps.

5 новых tests: compatibility с existing factory signature, missing inputs/default
avoidance, complete hooks без construction/calls, timed/invalid binding/dt errors,
policy/debug integration. Единая проверка: 152 tests прошли, one_room won, 8/8;
оба diff-check чистые. Использованы rg/чтение, Python/unittest/mock/inspect,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Численные правила не расширялись.

Следующий шаг каркаса: player-command routing contract для существующих runtime
handlers, без добавления новых атак/эффектов и без исполнения modeled policies.

## Combined monster debug/readiness — 2026-10-18

Добавлен monster_debug.py: versioned strict-JSON report объединяет policy,
action context/dependencies, proposal/result, errors и конкретные blockers.
Missing actor/turn/fields/hooks/proposal/result видны отдельно. Recorded failed
result даёт debug failed с recorded_action_failure; invalid declarations остаются
invalid. External completed и full bindings по-прежнему pending, modeled AI disabled.

Preflight включает monster_debug рядом с прежним monster_policy. Пять future
extension points перечисляют context→perception/target→proposal→composition→verify.
Composition ссылается на существующий runtime.make_profiled_monster_runtime_action;
в карте также отмечен execute_monster_movement_proposal. Никакие factory callbacks
не создаются/не устанавливаются. SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: scenario report/preflight, missing blockers/dependency order,
complete external observations без execution, external failure vs invalid input,
detached formatter/schema. Единая проверка: 147 tests прошли, one_room won,
8/8 фаз; diff-check чистый. Использованы rg/чтение, Python/unittest/mock,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Игровые алгоритмы не менялись.

Следующий шаг каркаса: inspection контракта/bindings существующей profiled-monster
factory (mind_for/observe/options/timed ownership), без construction/execution.

## Monster proposal/result contract — 2026-10-18

Добавлен monster_action_contract.py: inspector использует существующий
monster_ai.MonsterActionProposal, проверяет nonempty action/IDs, optional target,
integer destination pair и actor/context agreement. Список допустимых native
actions не придумывается: shape-valid не означает semantics/execution valid.
Новый MonsterActionResult описывает внешние pending/completed/failed observations,
reason, mutation/turn/events и исходный proposal. Payload detached strict JSON.

Pending требует explicit state_may_be_modified=False, без events/наблюдаемого
turn advance. Completed требует proposal, но не доказывает исполнение; failed
допускает unknown mutation и не допускает automatic retry. Actor/result/proposal
mismatch diagnostic invalid. Modeled policy включает action_contract и остаётся
pending; scenario callbacks не принимают silent proposal/result overrides.
SUBSYSTEM_INTERFACES.md обновлён. Runtime/executor/AI алгоритмы не менялись.

5 новых tests: reuse/shape без semantic claim, invalid coordinates/actor,
pending consistency, external results/deep-copy/no execution proof, result/policy
integration. Единая проверка: 142 tests прошли, one_room won, 8/8 фаз; diff-check
чистый. Использованы rg/чтение, Python/unittest, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.

Следующий шаг каркаса: combined monster debug/readiness и future implementation
extension points, сохраняя modeled_ai execution disabled.

## Monster-action context/dependency contract — 2026-10-18

Добавлен monster_context.py: MonsterActionContext содержит explicit monster_id,
turn (или unknown) и MonsterContextField groups actor/scheduler/player state,
terrain/occupancy/perception/target/timed/RNG. Missing fields unknown; provided
values strict JSON с source reference; not_applicable требует reason/reference.
Семантика values и реальный actor lookup не проверяются. Inspector не выдумывает
perception/targets и не вызывает runtime.

MonsterActionAdapters задаёт future context/perception/target providers,
action_selector и action_executor. Полные callable bindings только декларация,
execution_allowed=False. inspect_monster_policy включает context/dependencies
для modeled_ai; прежние scenario_callbacks не принимают silent override context.
Per-action inspector opt-in, не новые ScenarioDefinition gameplay inputs и не
adapter execution. SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: unspecified inputs, detached false/zero observations, full data/hooks
без execution, invalid actor/fields/references/JSON и unused scenario override.
Единая проверка: 137 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Игровые AI/perception/target algorithms и numerical rules не менялись.

Следующий шаг каркаса: proposal/result inspection через существующий
monster_ai.MonsterActionProposal, без новой AI реализации или исполнения actions.

## Explicit monster-action policy contract — 2026-10-18

Добавлен monster_policy.py: scenario_callbacks available и modeled_ai pending;
unknown selection/mode/noncallable binding diagnostic invalid. Inspector показывает
encounter/campaign existing callback references и TurnAdapterBundle hook signature,
не вызывает callable и не заявляет native fidelity. Даже full binding не включает
modeled AI. Missing inputs/composition/timed/RNG/AI-spell coverage остаются явными.

ScenarioDefinition.monster_action_policy (default scenario_callbacks) подключён
к validator, configured/raw preflight, scenario report и turn bundle metadata.
MVP monster closures не переписывались, callback order и RNG сохранены.
SUBSYSTEM_INTERFACES.md обновлён: row, wiring, unresolved work и next step.

5 новых tests: mode-specific interfaces, callable presence without invocation,
pending/unknown before startup, invalid mode/binding и one_room policy report.
Единая проверка: 132 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Native numerical rules и новые AI/spell algorithms не добавлялись.

Следующий шаг каркаса: monster-action input context и adapter dependency contract,
без исполнения modeled AI и без угадывания perception/target parameters.

## Единый world readiness/debug report — 2026-10-18

Добавлен world_debug.py: build_world_debug_report и summarize_world_policy
объединяют policy, input states, sources, scheduler boundary и concrete blockers.
Versioned strict JSON содержит summary, sections, errors и 4 future implementation
steps: resolve_inputs→validate_inputs→compose_callback→validate_execution.
Dependencies/evidence явны, все execution_enabled=False. Report detached от inputs.

Preflight включает world_debug и сохраняет прежний world_policy report.
Scenario_trails available означает только поддержанный выбор прежнего callback;
modeled_world_execution_allowed=False и native_fidelity_validated=False. Для
modeled_native неизвестные inputs/sources, resolver, speed, semantic/composition/
numerical gaps перечислены отдельно. Полный набор declarations не запускает effects.
SUBSYSTEM_INTERFACES.md обновлён вместе с кодом.

5 новых tests: trails semantics/JSON, missing blockers и dependency order,
complete declarations без resolver invocation, invalid/detached report и static
preflight integration. Единая проверка: 127 tests прошли, one_room won, 8/8 фаз;
оба diff-check чистые. Использованы rg/чтение, Python/unittest/mock, Bash launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались. Новых numerical/world algorithms и dependencies нет.

Следующий шаг каркаса: explicit monster-action policy contract для существующих
scenario callbacks и pending broader modeled AI/spell integration.

## World source/adapter и scheduler-speed ownership — 2026-10-18

Добавлен world_sources.py: WorldInputSource задаёт name/kind/reference,
WorldInputAdapter — будущий resolve(runtime, requested_names). Inspection не
вызывает resolver и не подтверждает источник/значение. Provided fields без source
видны как missing_source_names. Пустые/duplicate/unknown declarations invalid.
player_speed требует source kind scheduler; supplied integer world speed должен
совпадать с явно переданным scheduler_speed. Forwarding policy остаётся unresolved:
не дублировать scheduler argument в world_options автоматически.

ScenarioDefinition.world_sources подключён к config/preflight/raw runner checks.
Trails не принимает unused sources; modeled_native pending независимо от bindings.
Scope — declarations/input agreement, не native rule validation. Value semantics,
resolver mutation/RNG contract и modeled callback composition ещё не реализованы.
SUBSYSTEM_INTERFACES.md обновлён: API, wiring, unresolved work и next step.

5 новых tests: bound resolver without invocation, missing source, scheduler
ownership/conflicts, malformed/unused sources, configured preflight без runtime.
Единая проверка: 122 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Игровые правила, алгоритмы и RNG execution не менялись.

Следующий шаг каркаса: combined world readiness/debug report и точки будущей
реализации input adapter/composition, не включая modeled world effects.

## World-options input-state contract — 2026-10-18

Добавлен world_inputs.WorldInputState/inspect_world_inputs. Missing inputs явно
unknown; provided сохраняет strict-JSON value (включая None/False/0), not_applicable
требует reason и не допускает value. Unknown тоже не маскирует supplied value.
Duplicate/unknown names, invalid records и nonfinite/non-JSON values диагностируются.
Все values копируются; defaults не применяются. Declared не означает semantic
validation или подтверждённую applicability.

ScenarioDefinition.world_inputs подключён к validator, preflight и raw/configured
runner checks. Неиспользуемые trails inputs отклоняются до startup. modeled_native
остаётся pending даже при полностью declared input list. Runtime numerical
processing не включён. SUBSYSTEM_INTERFACES.md обновлён вместе с интерфейсом.

5 новых tests: missing vs explicit None/False/0, reasons/applicability, detached
values и malformed input, declared без startup/native selection, unused trails
rejection. Единая проверка: 117 tests прошли, one_room won, 8/8 фаз; diff-check
чистый. Использованы rg/чтение, Python/unittest/mock, Bash launcher, Git. MCP,
Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Новые механики/параметры по умолчанию не добавлялись.

Следующий шаг каркаса: world external-input adapter/source contract и
scheduler-speed ownership; values и modeled callbacks пока не исполнять.

## World policy/input inspection contract — 2026-10-18

Добавлен world_policy.py: scenario_trails available, modeled_native pending,
unknown policy invalid. Static input-name inspection показывает explicit names
существующего process_native_world_turn, но не подставляет значения и не
определяет defaults/requiredness или полный набор effect/reduction options.
Input values и callback binding not validated; наличие всех names не включает
native processing. Scheduler-speed ownership/composition остаются явными gaps.

ScenarioDefinition.world_policy (default scenario_trails) подключён к validator,
preflight, configured/raw runners и turn_adapters metadata. Unsupported selection
отклоняется до runtime. Успешные runner по-прежнему передают refresh_player_trails;
численные правила, scheduler order и RNG не менялись. SUBSYSTEM_INTERFACES.md
обновлён: world row, active wiring, pending inputs и следующий этап.

5 новых tests: selection without fidelity claims, pending native со всеми names,
invalid/unused names, raw/configured pre-bootstrap rejection, one_room config/
preflight/bundle reports. Единая проверка: 112 tests прошли, one_room won, 8/8;
оба diff-check чистые. Использованы rg/чтение, Python/unittest/mock, Bash launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались: native rule behavior не менялось.

Следующий шаг каркаса: world-options input-state contract с явными unknown/
provided значениями и ограничениями адаптера; modeled_native не включать.

## Карта интерфейсов игровых подсистем — 2026-10-18

Добавлен SUBSYSTEM_INTERFACES.md: source-model interface map для configuration,
player/state, commands/scheduler, melee/ranged/throw, damage/death, inventory,
pickups/drops, experience, monster placement/AI/spells, timed/effects, terrain,
world, generation/transition, RNG/events, snapshots и transfer/debug/export.
Для каждого указаны существующие modules/APIs, scenario wiring и future input/gap.
Методы сверены по AST/call sites, 27 module paths проверены на существование.
Это evidence текущего Python model, не повторная numerical/native source audit.

Выявлена важная граница: MVP world_phase=refresh_player_trails, тогда как
finish_native_world_cycle отдельно подключает process_native_world_turn.
Connected scheduler boundary не означает, что весь modeled world-effects pipeline
подключён к primary MVP. В таблице API available и connected разделены явно.
Будущую реализацию направлять в существующие модули; не добавлять generic
waves/perks/co-op по шаблону без подтверждённого соответствия игре.

В этом этапе изменены только документы. One_room smoke won, 8/8 фаз; оба
Git diff-check чистые. Последний contract suite — 107 tests — ранее прошёл,
здесь не перезапускался. Использованы rg/чтение, Python AST/path checks,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Native rules и numerical code не менялись.

Следующий шаг каркаса: explicit world-phase policy/input inspection contract,
с сохранением текущего trails callback и без включения additional world mechanics.

## Transfer debug-report и карта будущей реализации — 2026-10-18

Добавлен transfer_debug.py: build_transfer_debug_report и formatter существующего
readiness report. Versioned strict-JSON отчёт содержит summary, input/bridge/
lifecycle/manifest/comparison/provenance sections, errors/blockers и 6 ordered
implementation steps с interfaces/dependencies/required evidence. Отчёт не
принимает executable/ready claims, данные отделены от исходных mutable objects.
Restricted preflight включает transfer_debug; другие backend — None.

TRANSFER_IMPLEMENTATION_GUIDE.md описывает, куда позже подключать profile
resolution, capture, isolation, generation, commit и verification. Зафиксированы
failure semantics, порядок evidence/lifecycle, source MCP перед native rules и
явные неизвестные handoff policies. Самого transfer executor нет: callbacks,
chunk/RNG, game algorithms и numerical параметры не менялись. Всё ещё каркас.

4 новых tests: JSON/section ordering, failed input с complete bindings без вызова,
detached report/schema rejection и preflight integration. Единая проверка:
107 tests прошли, one_room won, 8/8 фаз; diff-check чистый. Использованы rg/чтение,
Python/unittest/mock, Bash launcher, Git. MCP, Arbor, C probe/сборка, web,
субагенты и многоэтажные сценарии не использовались.

Следующий шаг каркаса: итоговая карта интерфейсов игровых подсистем и сверка
точек расширения; implementation остаётся последующим отдельным этапом.

## Transfer evidence provenance — 2026-10-18

Добавлен transfer_provenance: bind_transfer_manifest связывает проверенный manifest
с explicit request, transfer_id, before/after role и существующим lifecycle prefix.
Before соответствует created boundary (index 0), after — committed/verifying/
completed. SHA-256 canonical JSON digests фиксируют request, manifest и trace
prefix. Это checksum внешних наблюдений, не аутентификация runtime execution.
Профиль не подбирается, adapters не вызываются, manifest/runtime не меняются.

inspect_transfer_provenance возвращает pending при missing binding,
structurally_bound при совпадении или failed при смешанных/изменённых данных.
Readiness принимает before_binding/after_binding, сохраняет отдельные provenance
reports/blockers и ошибки. Даже обе valid bindings не снимают execution/isolation/
catalog/native-tail blockers. execution_allowed и execution_validated False.

В suite добавлены 4 tests: detached binding/prefix consistency, чужой transfer ID,
request/role/manifest edit, invalid after boundary, trace checksum edit и readiness
integration без разрешения transfer. Единая проверка: 103 tests прошли, one_room
won, 8/8 фаз; diff-check чистый. Использованы rg/чтение, Python/unittest,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Численные механики не расширялись.

Следующий шаг каркаса: единый debug-report для transfer evidence/statuses и
remaining blockers, без создания/commit chunk и без выбора неизвестных политик.

## Единый transfer readiness report — 2026-10-18

Добавлен transfer_readiness.inspect_transfer_readiness: объединяет restricted
input/profile status, bridge bindings, observational lifecycle, до/после manifest
и comparison под caller invariants. Blockers имеют code/message; invalid input,
manifest, recorded failure и violated comparison отражаются отдельно в errors.
Missing/unresolved evidence даёт pending, явные ошибки — failed. After/before
manifest валидируются без capture/runtime; callbacks и transfer не выполняются.

Даже полный набор внешних данных и satisfied comparison не разрешает исполнение:
execution_allowed/execution_supported/native_fidelity_validated=False. Постоянные
блокировки честно фиксируют profile catalog, isolation, связь request/trace/
manifest с реальным переносом, executable dispatch и native tail. Restricted
scenario preflight включает отчёт; обычные backend получают transfer_readiness=None.

5 новых tests: missing blockers, complete evidence без hooks/runtime mutation,
detached report, invalid manifest/recorded failure, violations vs unresolved,
static preflight integration. Единая проверка: 99 tests прошли, one_room won,
8/8 фаз; diff-check чистый. Использованы rg/чтение, Python/unittest/mock,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Численные механики не расширялись.

Следующий шаг каркаса: provenance-контракт привязки evidence к transfer_id,
стороне before/after и lifecycle boundary; не выдавать это за execution proof.

## Manifest comparison и preservation invariants — 2026-10-18

Добавлен transfer_comparison.compare_transfer_manifests: оба входа проходят
manifest inspection, затем сравниваются ownership/carried_objects/player/progress/
RNG/commands/events/scheduler/source. Invariants задаёт caller: equal либо observe;
для carried_objects дополнительно retain_existing (исходные IDs/values сохраняются,
новые допускаются). Без explicit invariant раздел pending даже при равенстве.
Observe отмечает изменение, но не подтверждает допустимость. Violations дают
failed, unresolved sections — pending, все заданные условия выполнены — satisfied.

Report содержит changed flags, removed/added/changed object IDs, violated/pending
sections и scope=caller_supplied_manifest_invariants. Satisfied не подтверждает
transfer execution/native parity; execution_supported/native_fidelity_validated
остаются False. Ничего не исправляется и не переносится. Политики reset/replay
RNG/commands/events не придумываются. Restricted lifecycle включает comparison
contract; runtime и preflight callbacks не запускаются.

Единая проверка: 94 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
6 новых tests покрывают unresolved defaults, explicit equality, detached inputs,
object mutation/ownership changes, added objects, observed RNG/commands/events,
invalid references/modes без repair. Использованы rg/чтение, Python/unittest,
Bash launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные
сценарии не использовались. Numerical механики не расширялись.

Следующий шаг каркаса: единый transfer readiness report, объединяющий input,
bridge bindings, lifecycle, manifest и comparison evidence с явными блокировками;
исполнение transfer пока не включать.

## Transfer manifest: source state и ownership — 2026-10-18

Добавлен transfer_manifest.py: capture_transfer_manifest читает runtime getters
и snapshot, сохраняет независимые JSON данные. Manifest содержит carried objects
без location, упорядоченные inventory/equipment/pack/quiver views, player/progress,
RNG, FIFO commands, events, scheduler и source chunk metadata. Geometry, floor
objects, monsters, persistent chunks и framework/controller state не переносятся.
Unknown views остаются None; явно пустые views не заменяются отсутствующими.
Объекты без location вне inventory отмечены unclassified, без догадки о владельце.

inspect_transfer_manifest проверяет schema/capabilities, контейнеры и ссылки,
согласованность player data/ownership и unclassified IDs. Это не runtime restore
validator и не проверка совместимости destination. Strict JSON и отдельная копия
исключают shared mutable payload с source. Capture не исполняет callbacks/commands,
не двигает RNG, не создаёт chunk и не делает commit. Copy/reset/replay handoff
policy явно unresolved. Restricted lifecycle report включает manifest contract.

Единая проверка: 88 tests прошли, one_room won, 8/8 фаз; diff-check чистый.
Добавлены read-only/deep-copy/RNG/commands checks, slot order/empty positions,
unknown-vs-explicit-empty inventory, dangling references и false execution claim.
В тесте quiver использована capacity из существующего runtime config без
подстановки чисел. Использованы rg/чтение, Python/unittest, Bash launcher, Git.
MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не использованы.
Игровые механики и native численные правила не менялись.

Следующий шаг каркаса: comparison report двух manifest и явные preservation
инварианты до/после transfer, без исполнения переноса.

## Fresh-chunk transfer lifecycle — 2026-10-18

Добавлен chunk_transfer.py: immutable ChunkTransferTrace/TransferObservation
фиксируют внешние наблюдения prepare→generation→commit→verification. Переходы
проверяются; commit до generation/repeated commit, resume terminal state и abort
после начала commit запрещены. Pending допускает только abort/failure; причины
pending/abort/failure обязательны. Failure после commit_started помечает
partial_commit_possible без обещания rollback. Constructor проверяет историю,
report возвращает отдельные JSON данные.

Это observational contract: callbacks не вызываются, chunk/RNG не изменяются,
completed означает лишь записанную последовательность. execution_supported и
adapter_execution_validated всегда False. describe_chunk_transfer перечисляет
недостающие проверки изоляции, carried objects, inventory/equipment/pack/quiver,
RNG, scheduler/commands/events и post-commit ownership. Restricted preflight
включает lifecycle description, execution по-прежнему pending.

В suite добавлены 5 tests: ordered observations/immutability, pending/abort,
commit ordering/partial failure/no retry, invalid history/reason и preflight
integration. Единая проверка: 83 tests прошли, one_room won, 8/8 фаз; diff-check
чистый. Использованы rg/чтение, Python/unittest, Bash launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Численные механики и native runtime не менялись.

Следующий шаг каркаса: manifest переносимых данных и ownership references
(inventory/equipment/pack/quiver, RNG, commands/events), без исполнения commit.

## Restricted preset input и fresh-chunk bridge contracts — 2026-10-18

Добавлен restricted_generation.py: RestrictedPresetInput требует явные
profile_name/depth/player_depth/quest_level/persistent/arena; профиль не выбирается
и не создаётся автоматически. FreshChunkBridge задаёт capture_carried,
create_generation_runtime, commit_generated_chunk и verify_carried. Inspection
проверяет наличие callable hooks, но не вызывает их, не копирует и не меняет chunk.
bindings_complete отдельно от isolation_validated/execution_supported=False.

LevelGenerationPolicy.restricted_preset подключён к backend/preflight report.
Переданный профиль убирает missing-input diagnostic, но catalog остаётся
not_inspected, bridge hooks и scenario dispatch/native-tail integration pending.
Unused request с другим backend отклоняется. Даже полный bridge registry не
включает execution: нет скрытого commit, переноса inventory или clearing chunk.
Никакие native численные правила не переносились и не менялись.

В contract suite включены 5 новых тестов: explicit input/JSON, missing hooks,
полные bindings без вызовов/claims, invalid/unsupported inputs, configured
pending до runtime и запрет unused input. Единая проверка: 78 tests прошли,
one_room won, 8/8 фаз; оба diff-check чистые. Использованы rg/чтение,
Python/unittest/mock, Bash launcher, Git. MCP, Arbor, C probe/сборка, web,
субагенты и многоэтажные сценарии не использовались.

Следующий каркасный шаг: план lifecycle fresh-chunk transfer (prepare/commit/
abort, переносимые ссылки и failure states), без реализации native generation.

## Единая проверка и checklist каркаса — 2026-10-18

Добавлены check_skeleton.sh и angband_model.skeleton_check: явный contract-suite
список плюс отдельный SCENARIOS['one_room'] smoke. Launcher разрешает пути от
своего расположения, без CLI inputs и без зависимости от cwd/ACTIVE_SCENARIO.
Ошибки suite/smoke дают nonzero exit. Smoke проверяет outcome, порядок/статусы
8 lifecycle phases, strict JSON, fixture backend, preflight, unsupported resume
и generation_complete=False. Новые numerical rules и dependencies не добавлялись.

SKELETON_CHECKLIST.md фиксирует connected interfaces отдельно от pending native
adapters/validation. Suite не выдаётся за все numerical/parity tests. Generated
и многоэтажные сценарии не обнаруживаются и не запускаются; synthetic wiring
fixtures в выбранных tests сохранены. Проверка не экспортирует файлы.

Единая команда выполнена из /tmp через absolute path: 73 tests прошли,
one_room won, 8/8 фаз. Оба diff-check чистые. Использованы rg/чтение,
Python/unittest, Bash launcher и Git. MCP, Arbor, C probe/сборка, web, субагенты
и многоэтажные сценарии не использовались.

Следующий шаг каркаса: explicit restricted-preset profile input и контракт
fresh-chunk carried-state adapter; выполнение оставить pending до безопасного
подключения. Native-tail численные алгоритмы пока не расширять.

## Public preflight и readiness обзор — 2026-10-18

Добавлен preflight.inspect_scenario_preflight: static config/backend validation,
список обязательных turn hooks, runtime checks, native generation tail specs с
источниками/недостающими контрактами, session blockers и framework clock mode.
Не создаёт runtime, не читает игровые каталоги и не вызывает callbacks/RNG.
startup_allowed означает только допустимость static configuration, не успешный
startup/gameplay и не numerical fidelity. Hooks/catalogs/state остаются явно
not_inspected, native_generation pending, generation_complete=False.

require_scenario_preflight подключён к run_configured_scenario; result содержит
preflight рядом с generation_backend. Main проверяет сценарий до чтения полей
для логов. Persistent/arena campaign transitions без adapter отклоняются до
runtime; restricted backend остаётся pending. Low-level runners сохраняют
собственные checks и не обещают полного ScenarioDefinition preflight.

Проверки: 73 tests прошли. Static inspection всех четырёх config без runtime,
отсутствие mutation, JSON report, unsupported backend/transitions, invalid main
config до logging field access и one_room configured report. Генерируемые и
многоэтажные сценарии не запускались. One_room smoke won, 8/8 фаз; diff-check
чистый. Использованы rg/чтение, Python/unittest/mock, launcher, Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Новых численных правил и зависимостей нет.

Следующий шаг каркаса: единая команда проверки contract suite + one_room и
итоговый checklist интерфейсов; pending native adapters не включать автоматически.

## Bootstrap failure contract — 2026-10-18

Добавлен bootstrap.bootstrap_failure_result. Encounter/campaign различают
build_model, composition_start и runtime_lookup exceptions до получения usable
runtime. Результат использует общий envelope: incomplete/bootstrap_failed,
stopped_phase=runtime, bootstrap_error с исходной exception и явными
runtime_acquired=False/partial_startup_possible=True/retry_performed=False.
Runtime phase adapter, gameplay phases skipped, outcome incomplete, snapshot
phase skipped. turns=None и runtime_snapshot=None: runtime observations не
выдумываются. Controller counters до начала gameplay остаются 0.

Seed/options/navigation validation остаётся до bootstrap boundary; catalog
race/class/monster validation после загрузки остаётся отдельной configuration
проверкой. Нет повторного factory/start/lookup. Configured path добавляет свою
scenario metadata к failure result. Main экспортирует bootstrap report обычным
output helper и завершает exit 1 даже при успешном export. Partial factory state
не восстанавливается и не объявляется безопасным checkpoint.

Проверки: 69 tests прошли. Проверены три startup failure stages в обоих runner,
ровно один вызов startup dependencies, отсутствие birth, null runtime fields,
правильный campaign lifecycle, отдельные configuration errors и main export +
nonzero exit. One_room smoke won, 8/8 фаз; оба diff-check чистые. Использованы
rg/чтение, Python/unittest/mock, launcher и Git. MCP, Arbor, C probe/сборка, web,
субагенты и многоэтажные сценарии не использовались. Игровые правила не менялись.

Следующий этап каркаса: public preflight guarantees и единый readiness обзор
без запуска callbacks, без процента точности и без включения pending adapters.

## Output/export failure contract — 2026-10-18

Добавлен report_output.export_run_result для уже полученного result. Сериализация
строгого JSON выполняется один раз; callbacks/runtime не доступны helper. Отдельные
serialization/stdout/file statuses и error_type/message отражают output failures.
При serialization failure sinks skipped; при stdout failure файл всё равно
пытается сохраниться. Исходный gameplay result не меняется, simulation/rewards
не повторяются. Запись файла идёт через temp в той же папке и os.replace после
закрытия полной записи; temporary очищается. Fsync/durability не обещаются.

mvp.main использует этот контракт, сохраняет привычный stdout title + JSON и
report_path. Output failure логируется отдельно как report_output_failed и даёт
exit code 1; scenario_done по-прежнему описывает simulation, не успешный export.
Диагностика экспорта возвращается отдельным helper report, не встраивается в
игровой payload и не меняет outcome. Повторной попытки/restart нет.

Проверки: 66 contract/controller/session/generation/output/one_room тестов прошли.
Проверены matching stdout/file JSON, unchanged payload, invalid/nonfinite JSON,
broken stdout с успешным file, replace failure с сохранением старого файла и
cleanup, main output failure с ровно одним scenario call. One_room smoke won,
8/8 фаз; diff-check чистый. Использованы rg/чтение, Python/unittest/mock,
launcher, Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии
не использовались. Внешних зависимостей и numerical правил не добавлялось.

Следующий этап каркаса: отдельный bootstrap failure result до создания runtime,
затем public preflight guarantees и единый readiness обзор.

## Special generation/turn-stop reporting — 2026-10-18

finish_generation_stop и finish_turn_stop теперь защищены от secondary snapshot,
summary и payload diagnostics failures. Общий _finish_stop_report_error закрывает
остаток trace через существующий finish_stage_stop без перезаписи фаз. В результате
сохраняются исходные outcome_reason/stopped_phase, primary_stop (kind/status/report),
turn_status/turn_report либо generation_status/generation_continuation. Вторичная
ошибка вынесена в reporting_error; недоступный snapshot остаётся явным None с
snapshot_error. Это fallback diagnostics, не повтор адаптера, rollback или resume.

Generation adapter failure больше не переименовывается во внешний population
failure при сбое stop summary. Partial runtime, ранее накопленные campaign
counters/floor_results и первичные adapter messages сохраняются. При уже закрытом
trace вторичный payload failure не добавляет дубли outcome/snapshot. Обычные
stop reports без secondary failure сохраняют прежние поля и причины.

Проверки: 61 controller/session/generation/one_room тест прошёл. Добавлены pending
и failed turn stop с summary/snapshot/inventory errors, generation failed с
исходным adapter message и secondary failure, pending generation continuation.
Подтверждены 8/8 trace, сохранение counters, отсутствие шага и расхода RNG в
изолированных turn-stop checks. One_room smoke won, 8/8 фаз; diff-check чистый.
Использованы rg/чтение, Python/unittest/mock, py_compile, launcher, Git. MCP,
Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не использовались.
Игровые алгоритмы и численные параметры не менялись.

Следующий этап каркаса: output/export failure contract без повторного запуска
simulation; затем bootstrap и общий readiness обзор.

## Повторный аудит каркаса — 2026-10-18

Перепроверены public entry points, backend/turn/session contracts, специальные
stop reports и внешний output. Актуальная сводка переписана в
`angband/plans/model_skeleton_audit.md` (путь от workspace root), без устаревших
указаний на уже закрытые campaign/framework boundaries.

Подтверждён пробел finish_turn_stop: исключение snapshot summary выходит наружу
при pending hooks. Изолированный API probe не запускал step (turn=0), trace
остался 7/8. Generation stop также требует сохранения первичной причины при
вторичном reporting failure. Следом остаются output/export и bootstrap boundaries,
гарантии public preflight, общий readiness обзор. Native adapters и resume
отдельно перечислены как намеренно pending, не как реализованные механики.

В этом этапе изменены только документы. Проверка: one_room smoke won, 8/8 фаз;
чтение кода и synthetic reporting probe. Предыдущие 58 tests повторно не гонялись.
Использованы rg, Python/mock, launcher и Git. MCP, Arbor, C probe/сборка, web,
субагенты и многоэтажные сценарии не использовались.

Следующий этап каркаса: special generation/turn-stop reporting с сохранением
исходной причины и secondary reporting error без retry/rollback.

## Step trace, progression и final reporting boundaries — 2026-10-18

Post-step trace в encounter/campaign подключён к finish_stage_stop. Выполненный
step учитывается до trace; сбой trace возвращает stage=step_trace, phase=turns,
сохраняя partial runtime и encounter_steps без повторного scheduler/input.
Turn summary и progression report также защищены; stage различает turn_report
и progression_report. Progression здесь только отчёт о существующем runtime:
новая логика наград/опыта не добавлялась.

Final outcome/snapshot/payload reporting использует final_report boundary.
finish_stage_stop теперь допускает outcome/snapshot и уже закрытый lifecycle:
записанные фазы не переписываются и не дублируются. При позднем сбое результат
incomplete, прежний outcome доступен в stage_error.recorded_outcome; stopped_phase
равен snapshot или report для payload после полного trace. Snapshot failure
сохраняется отдельно. Повторный диагностический snapshot — только чтение, не
повтор gameplay, rollback или resume.

Проверки: 58 controller/session/generation/one_room тестов прошли. Новые случаи:
trace failure ровно после одного step в обоих runner, progression report error,
snapshot summary failure, payload failure после 8 закрытых фаз и недоступный
snapshot. One_room smoke won, 8/8 фаз; оба diff-check чистые. Использованы rg,
Python/unittest, py_compile, launcher, Git. MCP, Arbor, C probe/сборка, web,
субагенты и многоэтажные сценарии не использовались; numerical rules неизменны.

Следующий шаг: сверка остаточных пробелов каркаса. Bootstrap до runtime,
внешний JSON/file output и специальные stop-report paths не считать полностью
защищёнными без отдельной проверки; numerical детализацию пока не расширять.

## Navigation и turn setup boundaries — 2026-10-18

Campaign navigation (walk/scenario_handoff, stair request и exit-navigation
report) подключена к finish_stage_stop. Partial state и уже накопленные
counters/floor results сохраняются; последующие level/population/turns skipped.
Неверный navigation_mode валидируется до runtime creation. Ошибка navigation
помечается stage=navigation, stopped_phase=exit_navigation.

В encounter/campaign защищены подготовка melee/flow, создание TurnAdapterBundle
и подготовка trace до loop. Ошибка возвращает stage=turn_setup, stopped_phase=turns
с diagnostic snapshot; scheduler не запускается после неудачной подготовки.
Существующий TurnAdapterBundle.attempt по-прежнему обрабатывает input/runtime
callback failures своим контрактом. Порядок успешных вызовов, игровые формулы,
RNG и native механики не расширялись. Rollback/retry/resume отсутствуют.

Проверки: 53 controller/session/generation/one_room теста прошли. Новые случаи:
partial flow setup в обоих runner, melee/bundle construction failures, invalid
navigation input, walk/handoff failures после синтетически закрытого encounter.
Navigation test использует fixture room и fake step: это проверка wiring,
не запуск two_floors или native generation. One_room smoke won, 8/8 фаз;
оба diff-check чистые. Использованы rg, Python/unittest, py_compile, launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии не
использовались.

Следующий этап каркаса: ошибки step trace/progression/final reporting и сверка
оставшихся границ. Полнота lifecycle не означает numerical/native fidelity.

## Birth/population controller boundaries — 2026-10-18

Birth в encounter/campaign подключён к finish_stage_stop: partial initialization
сохраняется в raw snapshot, level/population/turns/progression skipped, результат
incomplete. validate_player_identity проверяет race/class по загруженным
каталогам до birth boundary; неверный выбор остаётся configuration ValueError.

Population orchestration охватывает fixture objects/entry/population setup в
encounter и placement/report setup в campaign. Ошибка отражается как stage
population_setup и фактическая текущая lifecycle phase (level либо population).
Существующие pending/failed generation adapters по-прежнему проходят через
finish_generation_stop, без замены их контракта. Campaign считает текущий уровень
подготовленным только после успешного population report; при сбое остаётся
прежний progress. Порядок успешных игровых вызовов и RNG не менялся.

Проверки: 49 contract/session/generation/controller/one_room тестов прошли.
Добавлены partial birth, неверные race/class, partial orchestration, report error
в обоих runner и совместимость generation adapter failure. Campaign проверялся
только на stub geometry, без генерации этажей. One_room smoke won, 8/8 фаз;
оба diff-check чистые. Использованы rg, Python/unittest, py_compile, launcher,
Git. MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные прогоны не
использовались. Новых численных механик, rollback/retry/resume нет.

Следующий этап каркаса: navigation и подготовка turns вне callback attempt;
затем progression/final reporting boundaries и повторная сверка пробелов.

## Geometry/transition diagnostic boundary — 2026-10-18

Добавлен общий finish_stage_stop для controller failures. One_room geometry,
campaign first geometry/entry и destination transition подключены к нему:
ошибка возвращает incomplete со stage_error (stage, lifecycle phase, exception,
partial-state flag), stopped_phase и полным diagnostic runtime_snapshot.
Оставшиеся gameplay phases skipped, outcome/snapshot закрывают trace. Прежние
campaign counters/floor results сохраняются. Награды не вычисляются, rollback,
retry и resume отсутствуют. Explicit stopped_phase имеет приоритет перед
next_phase старого generation report, который может относиться к прежнему chunk.

Диагностика не использует inventory-report adapters: повреждённые ownership
references не мешают сохранить raw snapshot. Если runtime.snapshot тоже падает,
результат сохраняет snapshot_error и captured=False вместо второго exception.
validate_generation_options выделен отдельно и вызывается до создания runtime
в обоих runner; invalid options/configured scenario по-прежнему дают validation
error, а не incomplete. Birth/navigation/population orchestration и остальные
неподключённые стадии ещё не охвачены общей boundary.

Проверки: 44 теста прошли (42 contract/session/generation/error/one_room +
2 configured-fixture/stubbed-transition regression). Проверены partial geometry,
сломанный inventory, campaign first-floor stop, snapshot failure, сохранение
прежнего прогресса и config validation. One_room smoke won, 8/8 фаз; diff-check
чистый. Использованы rg, Python/unittest, py_compile, launcher и Git. MCP,
Arbor, C probe/сборка, web, субагенты и многоэтажные прогоны не использованы.
Численные правила не менялись.

Следующий этап: подключить оставшиеся controller boundaries, начиная с birth
и population orchestration, сохранив конфигурационные ошибки отдельно.

## Session/checkpoint schema — 2026-10-18

Добавлен session_checkpoint.py: capture_session_checkpoint принимает runtime,
SimulationRun, ScenarioDefinition, явные controller counters, boundary и
необязательный framework_tick. Диагностический JSON envelope сохраняет полную
конфигурацию, runtime snapshot, lifecycle prefix/cursor, counters и отдельные
runtime/framework clocks. Capture не вызывает step/flush и не меняет RNG.
inspect_session_checkpoint проверяет версию, identity, lifecycle/cursor,
clock agreement и ограничения capabilities; возвращает независимые JSON данные.

Resume явно не поддержан: resume_session_checkpoint всегда отказывает до
restore/мутации. Не сохраняются callbacks, external policy state и framework
queue/delivery cursors. Diagnostic_stop допускает частично изменённое состояние;
recorded lifecycle cursor не является безопасной точкой повторного исполнения.
Run result содержит session_capabilities, но не создаёт полный checkpoint
автоматически. Полного persistence/controller resume пока нет.

Проверки: 38 contract/session/generation/one_room тестов прошли. Проверены JSON
round-trip, detached copy, отсутствие RNG/state mutation, cursor/identity/clock
ошибки, partial diagnostic state и запрет resume. One_room smoke won, 8/8 фаз.
Использованы rg, Python/unittest, launcher и Git. MCP, Arbor, C probe/сборка,
web, субагенты и многоэтажные прогоны не использовались: численные правила
не менялись.

Следующий этап каркаса: общая diagnostic error boundary для geometry и других
неподключённых стадий; configuration errors сохранять отдельными.

## Generation backend и carried-state guard — 2026-10-18

Добавлен generation_backend: describe_generation_backend различает auto,
fixture, scenario_generator и restricted_preset. LevelGenerationPolicy.backend
по умолчанию auto сохраняет прежний выбор по room fixture. Несовместимый backend
и restricted preset отклоняются validation до создания runtime. Restricted
остаётся pending: нужны explicit profile, fresh-chunk carried-state adapter и
native-tail integration. Скрытого переключения на другой генератор нет.

CarriedStateGuard использует существующие inventory/equipment validators,
сохраняет копии объектов без location и проверяет их после scenario generation.
Проверяются данные объектов, порядок inventory_ids и equipment_slots. Новые
объекты генерации допускаются; исходные переносимые объекты нельзя потерять
или изменить. Это проверка существующего reset, не новый алгоритм переноса.
Unclassified объекты без координат сохраняются, но не объявляются собственностью
игрока. Ошибка guard не откатывает состояние; единая geometry error boundary
ещё не реализована. Configured result содержит generation_backend report.

Проверки: 32 contract/generation/one_room теста; synthetic reset/wrapper без
генерации этажей, сохранение RNG и ownership, обнаружение partial mutation.
One_room smoke: won, 8/8 фаз. Использованы rg, Python/unittest, launcher и Git.
MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные прогоны не использованы:
численные правила и native runtime не менялись.

Следующий этап каркаса: session/checkpoint schema с явным запретом
неподдержанного resume и описанием clock/callback dependencies.

## Campaign/framework: граница хода и остановки — 2026-10-18

Campaign подключён к TurnAdapterBundle с существующими callbacks.
`attempt()` проверяет зависимости, возвращает pending до вызовов или failed
при исключении input/runtime step. `finish_turn_stop` закрывает lifecycle как
incomplete, сохраняет diagnostic runtime_snapshot и достигнутый прогресс;
частичные изменения не откатываются, повтор и resume не выполняются.

`framework.step_angband_with_adapters` выполняет доменный шаг и доставляет
события через публичный flush. Общий composition tick не продвигается;
обычный simulation.step не подключает bundle автоматически.

Проверки: 27 contract/generation/one_room тестов прошли, включая synthetic
campaign wiring без запуска генерации этажей, partial-state error и доставку
framework events. One_room smoke: won, 8/8 фаз. Численные правила не менялись.
Использованы rg, Python/unittest, py_compile, launcher и Git. MCP, Arbor,
C probe/сборка, web, субагенты и многоэтажные прогоны не использовались.

Следующий этап: generation backend selection и контракт переноса player-owned
state при создании chunk. Реализацию численных механик пока не расширять.

## Turn-adapter contract и one_room — 2026-10-18

Добавлен `turn_pipeline.TurnAdapterBundle`: явные player_speed, policy_name,
command_policy, player_action, monster_action и world_phase; validate/report
проверяют все hooks до подготовки команды/runtime.step. Отсутствующая
зависимость останавливает вызов до побочных действий. Report сериализуемый,
без сохранения callbacks, с меткой scenario_adapter.

One_room использует прежние сценарные callbacks через bundle. Подготовка
команды выделена в prepare_input, runtime.step получает прежние arguments;
порядок работы scheduler/monster/world остаётся в runtime. Split prepare/step
сохраняет прежнюю границу step trace после постановки команды, без двойного
input callback. Turns lifecycle содержит turn_adapters report. AI, damage,
world formulas и numerical параметры не расширялись.

Проверки: 22 contract/generation/snapshot/one_room tests прошли; отсутствующий
world hook не допускает input и step, forwarding происходит один раз.
Проверены прежние victory/turn-limit/death исходы. one_room smoke won, 8/8 фаз;
оба diff-check прошли. Использованы rg, Python/unittest, launcher и Git.
Source MCP, Arbor, C probe/сборка, web, субагенты и многоэтажные сценарии
не использовались — изменялось структурное подключение существующих callbacks.

Следующий этап скелета: подключить campaign/framework boundary к общему
turn contract и определить структурированный outcome для отсутствующих либо
ошибочных turn adapters. Проверять wiring на stub runtime/phase plan,
не запускать многоэтажные сценарии и не углублять игровые механики.

## Сводный аудит каркаса — 2026-10-18

Аудит записан в `angband/plans/model_skeleton_audit.md` относительно workspace.
Основной one_room lifecycle замкнут; повторные generation/report оболочки
не нужны. Реальные пробелы подключения: общий turn-adapter bundle, выбор
restricted preset backend с сохранением inventory при fresh chunk, session
checkpoint contract, диагностика ошибок за пределами generation. Framework
integration event delivery — отдельная низкоприоритетная граница.

Следующий этап: контракт и preflight для player/monster/world callbacks,
затем подключение существующего one_room цикла без нового AI или numerical
правил. Последующие backend/session задачи описаны в аудите. Runtime restore
не объявляется автоматическим resume; native_validation остаётся pending.

Проверка этапа: документационный аудит и one_room smoke won, 8/8 фаз;
многоэтажные ветки только просмотрены по коду. Новых тестов не добавляли.
Использованы rg, Git, Arbor MCP через Python-клиент и launcher. Source MCP,
C probe/сборка, browser и субагенты не использовались. Неоднозначную связь
Arbor build_model с чужим проектом исключили из доказательств и сверили файлы.

## Общий результат запуска и preflight adapters — 2026-10-18

Продолжаем только каркас. run_result.finalize_run_result задаёт обязательные
общие поля и result_schema_version=1 для encounter, campaign и ранней остановки.
Везде присутствуют generation_status, generation_complete, stopped_phase,
runtime_snapshot (None при обычном завершении). Специфичные метрики сохраняются;
готовность adapter-цепочки не выдаётся за native numerical fidelity.

inspect_generation_adapters проверяет регистрации и возвращает полный список
missing_adapters без вызовов. require_all_adapters=True в сценарных путях
останавливает всю цепочку до первого callback, если отсутствует любой
обязательный hook. Отчёт содержит preflight и execution_started=False;
next_phase указывает первый отсутствующий hook, а предыдущие этапы blocked.
Проверка выполняется перед callbacks, после подготовки геометрии/fixture inputs.
Restricted native prefix сохраняет необязательный режим частичного продолжения.

Проверки: 19 contract/snapshot/scenario tests и 1 native preset parity test
прошли без skips. Проверены отсутствие побочных действий при пропущенном
последнем adapter, единые поля success/stop, JSON и прежние сценарные исходы.
one_room smoke won, 8/8 фаз; оба diff-check прошли.
Использованы rg, Python/unittest, готовый C probe, launcher и Git.
MCP, Arbor, новая C-сборка, web, субагенты и многоэтажные сценарии не использовались.

Этот участок скелета generation → lifecycle → report/snapshot замкнут.
Следующий шаг: сводный аудит готовности всего каркаса и перечень действительно
недостающих интерфейсов, прежде чем добавлять новые оболочки или численные правила.

## Остановка lifecycle при незавершённой генерации — 2026-10-18

Продолжаем каркас, без детализации игровых алгоритмов. Общий
finish_generation_stop заменяет RuntimeError на границе adapter-цепочки
в encounter и campaign wiring. Pending/failed generation возвращает
outcome=incomplete, generation_status, outcome_reason, stopped_phase,
полный generation_report и runtime_snapshot с частичным состоянием.
Population/turns/progression и оставшиеся этапы будущих уровней отмечаются
skipped; outcome и snapshot завершают lifecycle в его обычном порядке.
Complete run trace означает заполненный отчёт, а не завершённую генерацию.

Ранее завершённые этажи/метрики campaign сохраняются. Ошибка адаптера не
откатывается и не повторяется: snapshot нужен для диагностики, автоматический
resume не реализован. Ошибки входной конфигурации по-прежнему отклоняются
валидацией; этот контракт относится к результатам generation adapters.

Проверки: 17 contract/snapshot/scenario tests прошли. Pending и exception
проверены через one_room с запретом вызовов runtime.step; turns=0, XP=0,
диагностические изменения failed adapter сохранены. Закрытие будущих фаз
проверено на синтетическом campaign phase plan без запуска этажей.
one_room smoke won, 8/8 фаз; оба diff-check прошли.
Использованы rg, Python/unittest, launcher и Git. MCP, Arbor, C probe,
C-сборка, браузер, субагенты и многоэтажные сценарии не использовались.

Следующий шаг каркаса: единая схема результата для обычного завершения и
ранней остановки, плюс проверка доступности обязательных adapters до запуска.
Численные реализации подсистем остаются отложенными.

## Общая точка входа generation-каркаса — 2026-10-18

Добавлен runtime.continue_generation_skeleton: единая проверка готовности
префикса, запуск adapters и запись chunk/lifecycle отчёта. Её используют
restricted preset bridge (успех и insufficient_rooms), fixture и generated
scenario paths. Неизвестный префикс отклоняется; после failed prefix callbacks
не выполняются, cleanup и недостающие стадии остаются pending.

Scenario populate_scenario_level вынесен в общий helper: явные координаты,
проверка входных позиций/object IDs/повторного monster ID, существующий
place_monster_race, описание готовых предметов и явные omission policies.
Generated paths сохраняют upstream contents; omission означает отсутствие
дополнительного scenario content, а не выполнение native placement.
MVP encounter и campaign wiring используют этот helper. Префиксы отмечены
scenario_fixture_ready либо scenario_generated_level_ready; все callbacks
остаются scenario_adapter, generation_complete=False.

Проверка: 16 contract/snapshot/scenario/native bridge tests прошли без skips;
fixture/generated helper проверен на маленьких синтетических картах без запуска
one_floor/two_floors/twenty_floors. Native preset parity сохранён на 12 cases.
one_room smoke won, 8/8 фаз; оба diff-check прошли.
Использованы rg, Python/unittest, готовый native C probe, launcher и Git.
MCP, Arbor, новая C-сборка, web, субагенты и многоэтажные сценарии не использовались.

Следующий шаг скелета: единый результат остановки при pending/failed generation
в верхнем lifecycle — сохранить диагностику и не запускать turns, вместо
неструктурированного RuntimeError. Детальные native алгоритмы отложены.

## Сценарные adapters подключены к one_room — 2026-10-18

Добавлен scenario_generation.build_scenario_generation_adapters: явные actions
и omissions по именам фаз. Неизвестные/пересекающиеся политики отклоняются;
для omission нужна непустая причина. Незарегистрированный этап остаётся pending.
Каждый результат помечен scenario_adapter, события отличают scenario_action
от scenario_omission; native fidelity не подразумевается.

В one_room общий каркас вызывает существующее размещение противника и
устанавливает заданную позицию игрока. Rubble, random traps и gold имеют явные
omission policies fixture. Предметы созданы как scenario inputs до цепочки;
object adapter проверяет их IDs и описывает без повторной materialization.
Сохраняется порядок RNG-зависимых игровых операций. Prefix имеет статус
scenario_fixture_ready, а не rooms_and_tunnels_complete: native prefix и stairs
fixture не воспроизводит. Все 6 callback фаз могут быть completed, но
native_validation_pending=True и generation_complete=False.

Итоговый report теперь содержит полный generation_report вместе с краткой
сводкой в level/snapshot lifecycle. Проверены победа, turn limit и смерть игрока.
Сгенерированные уровни/campaign пока используют прежнюю точку размещения;
их адаптеры следующий шаг каркаса. Детальная numerical реализация отложена.

Проверки: 13 contract/snapshot/scenario tests прошли; one_room smoke won, 8/8 фаз.
Использованы rg, Python/unittest, launcher и Git. MCP, Arbor, C probe,
новая C-сборка, web, субагенты и многоэтажные сценарии не использовались.

Следующий этап: общий entry point каркаса для fixture/generated/preset paths
с одинаковым контрактом готовности и ошибок, без углубления игровых алгоритмов.

## Generation-каркас в lifecycle и snapshot — 2026-10-18

Отчёт continuation хранится в `ChunkState.generation_report` вместе с уровнем:
версия, depth, prefix_status, pending этапы и полный отчёт adapter-цепочки.
Restricted preset bridge записывает его и при успехе префикса, и при
insufficient_rooms; выдаётся diagnostic event generation_pipeline_reported.
RNG и игровая карта от записи отчёта не меняются.

Full snapshot автоматически сохраняет отчёт; restore проверяет версию/основные
поля, восстанавливает независимую копию без вызовов callbacks или автоповтора.
Legacy snapshot без поля сбрасывает старый отчёт в None. Состояние отчёта
привязано к chunk, поэтому новый ChunkState не наследует отчёт прошлого уровня.

MVP level/snapshot lifecycle и компактные итоговые snapshot включают summary:
failed/pending/adapters_complete/untracked, next_phase, failed_phases,
pending_generation_stages и native_validation_pending. Отсутствие отчёта
обозначается untracked; complete жизненного цикла не означает готовность
точной генерации. Полные диагностические события остаются в chunk snapshot.

Проверка: 10 contract/snapshot/native bridge tests прошли без skips;
добавленные bridge assertions подтвердили совпадение сохранённого отчёта
и результата на 12 fixtures. one_room won, 8/8 фаз; оба diff-check прошли.
Использованы rg, Python/unittest, готовый C probe, launcher и Git.
Source MCP, Arbor, новая C-сборка, web, субагенты и многоэтажные сценарии
не использовались: изменялся только reporting-каркас.

Следующий шаг скелета: подключить существующие scenario population/player
adapters через явный реестр и показать сквозной проход каркаса с их метками
точности. Детальные native алгоритмы и автоматический resume отложены.

## Исполнение adapter-каркаса — 2026-10-18

В generation_pipeline добавлены GenerationPhaseOutcome, GenerationPhaseAdapter
и run_generation_continuation. Адаптер получает runtime и спецификацию фазы,
возвращает status (completed/pending/failed), implementation
(skeleton/scenario_adapter/native), message и JSON events. Реестр проверяется
до вызовов; порядок берётся из существующего каркаса. Отсутствующий adapter,
pending или failed останавливает цепочку и оставляет последующие фазы blocked.
Exceptions отражаются как failed; отката и автоповтора нет, отчёт явно сообщает
о возможном частичном изменении состояния. Pending-контракт требует отсутствия
изменений карты/RNG со стороны адаптера.

Restricted preset bridge принимает необязательный continuation_adapters mapping,
ключи — имена adapters из спецификаций. По умолчанию возвращается прежний
описательный каркас. Pending стадии берутся из результата цепочки. Даже если
все adapters completed, generation_complete=False: adapter_chain_complete
обозначает только проход цепочки, native_validation_pending остаётся true.
Скелет не может объявить собственную фазу completed. Возобновление частично
исполненной цепочки пока не реализовано; повтор может повторить side effects.

Проверка: 5 contract tests и native preset bridge test прошли (6 тестов,
без skips); one_room won, 8/8 фаз; оба diff-check прошли. Использованы
rg, Python/unittest, готовый native probe, launcher и Git. Source MCP и Arbor
не требовались для структурного контракта; новая C-сборка, web, субагенты
и многоэтажные сценарии не использовались.

Следующий этап скелета: связать отчёт цепочки с общим lifecycle/snapshot и
описать подключение существующих scenario adapters. Детальные алгоритмы
rubble/traps/population отложены согласно приоритету пользователя.

## Приоритет скелета и контракты хвоста генерации — 2026-10-18

По уточнению пользователя переключились с подробного переноса очередных
алгоритмов на сквозной каркас; numerical fidelity будем дописывать позже.
Добавлен `angband_model/generation_pipeline.py`: единый реестр фаз rubble,
random_traps, player_placement, monsters, objects, gold. У каждой фазы есть
предшественник, имя будущего adapter, исходные вызовы, SourceRef и список
недостающих контрактов. Порядок подтверждён через angband-source MCP:
`src/gen-cave.c:1294–1330`; objects объединяет два последовательных вызова,
сначала SET_ROOM, затем SET_BOTH.

Успешный restricted preset bridge возвращает `continuation`: rubble — pending,
следующие фазы blocked до завершения предшественника. Это описание интерфейсов,
без вызовов adapters, изменения карты или RNG. `generation_complete=False` и
существующие pending стадии сохранены. Уже перенесённые фазы не переписывались.
Приоритет закреплён в PROJECT_GOAL.md и MVP.md.

Проверка: существующий native preset bridge test прошёл (12 fixtures, без skips),
контракт/JSON continuation проверены; one_room won, 8/8 фаз; diff-check чистый.
Использованы angband-source MCP, rg, Python/unittest, готовый C probe,
launcher и Git. Arbor, новая сборка CMake/Ninja/Zig, web, субагенты
и многоэтажные сценарии не использовались.

Следующий шаг: определить контракт результата adapters (успех, ошибка,
невыполненный этап, события) и связать каркас с существующим lifecycle/report,
не углубляя сейчас rubble/traps, loot и AI. Сопоставить existing scenario
adapters с этими точками подключения и явно обозначить их точность.

## Native stair placement — 2026-10-18

После streamer фазы preset bridge запускает nonpersistent `handle_level_stairs`:
запрошенные down=3–4 и up=1–2 через `rand_range`, по очереди. На собранном
native toolchain выражения аргументов вычисляются в порядке down, затем up;
это проверено по C harness с трассировкой значений и полному RNG parity.
`alloc_stairs` перемешивает внутренние клетки выбором случайного остатка,
принимает пустой floor с 3, затем 2, 1 или 0 соседними cardinal walls,
соблюдает Chebyshev minsep `max(min(width,height)//4, 0)` для лестниц того же
типа и повторно обходит сохранённую перестановку при ослаблении wall gate.
Depth 0 принудительно делает MORE; quest или `depth >= max_depth - 1` делает
LESS. Значение `world:max-depth=128` — `angband/lib/gamedata/constants.txt:98`.
Исходники: `src/gen-cave.c:943–967,1282–1292`, `src/gen-util.c:602–732`,
`src/cave-square.c:578–581,929–944,1197–1227`.

Bridge fixtures с C `handle_level_stairs` сравнивают 12 полных карт и полный RNG.
C harness задаёт WALL flags и stair predicates для terrain, включая mineral
veins; это нужно, чтобы стеновые счётчики отражали игровую terrain data.
Добавлен quest unit fixture для преобразования обоих запрошенных типов в LESS.
Stairs снимаются с pending для успешного поддерживаемого bridge.

Проверка: 17 test_connectivity/test_tunnel_parity тестов прошли за 23.580 s
без skips; включены 12 исходных C bridge cases с лестницами и полным RNG.
Native parity harness пересобран; one_room won, 8/8 фаз; оба diff-check прошли.
Использованы angband-source MCP, rg, Python/unittest, CMake/Ninja/Zig, native
probe, launcher и Git. Arbor, web, субагенты и многоэтажные сценарии
не использовались.

Persistent levels остаются неподдержанными: `handle_level_stairs` проверяет
соседние persistent chunks, connector lists и может создавать staircase rooms.
Quest preset bridge пока отклоняется до native quest-room scheduling parity;
сам stair placement helper принимает quest flag. Occupancy от монстров,
объектов, traps/webs и cross-level connector avoidance требует engine adapter.
Bridge ограничен свежим nonpersistent map без этих сущностей.
Следующий этап — audit и перенос rubble/traps; generation_complete остаётся
false, поздние стадии и population не утверждаются завершёнными.

## Native dungeon streamers — 2026-10-18

В restricted preset bridge добавлена streamer фаза после ensure_connectedness,
в исходном порядке magma затем quartz. Реализован отдельный
`build_streamer_native`: старт через rand_spread (15 по x, 10 по y), направление
из ddd[0..7], ход до границы, `den` поисков в диапазоне ±`rng`, только
TF_GRANITE без DOOR_ANY, upgrade через native one_in. `find_nearby_grid`
воспроизводит cave_find_in_range Fisher-Yates выбором случайного остатка и
swap с текущей позицией. Поддержаны игровые MAGMA/QUARTZ и видимые *_K;
скрытые варианты не добавлены. Используются профильные значения без новых
констант. Существующий scenario carve_generated_streamer API не подменялся.

Плановые C preset fixtures теперь вызывают оригинальные 3 magma и 2 quartz
streamers с параметрами фактического игрового профиля (5,2,3,90,2,40) и
сравнивают полную карту/flags/RNG на 12 seed/profile комбинациях. Один
первоначальный тест поймал несовпадение порядка swap; после исправления
проверка карты и полного RNG проходит. `streamers` снят с pending только для
успешного bridge; недостаток комнат сохраняет ранний выход и pending этапы.
Проверка: 16 test_connectivity/test_tunnel_parity тестов прошли за 22.855 s
без skips; native bridge проверяет 12 полных карт и RNG с original C streamers.
Native harness собран CMake/Ninja/Zig; one_room won, 8/8 фаз; оба diff-check
прошли. Использованы angband-source MCP, rg, CMake/Ninja/Zig, Python/unittest,
native probe, launcher и Git. Arbor, web, субагенты и многоэтажные сценарии
не использовались.

Продолжение: stairs перенесены в отдельном этапе выше; quest и persistent
ограничения и необходимые адаптеры зафиксированы в его статусе.

## Connectivity repair integrated in preset bridge — 2026-10-18

Restricted preset flow now runs native room scheduling/install, permanent edge,
traditional tunnels, then `ensure_connectedness_native(allow_vault_disconnect=True)`
in `classic_gen` order. Successful room-and-tunnel results include connectivity
diagnostics; `ensure_connectedness` is removed from their pending list. Fewer than
two rooms still exits early, preserving the original failed-generation cleanup
handoff and pending stage. `generation_complete` remains false; later stages
(streamers through gold) remain pending.

Native preset C fixtures now call original `ensure_connectedness` after original
traditional tunneling. Existing 12 profile/seed cases compare full terrain,
room/wall/glow flags and complete RNG against the Python bridge, and assert
physical region count. The standalone 20-case repair parity suite remains active.

Проверка этапа: 16 тестов test_connectivity + test_tunnel_parity прошли за
22.876 s без skips; включены 20 repair cases и 12 bridge cases против C.
Native harness пересобран; one_room won, 8/8 фаз; оба git diff --check прошли.
Использованы angband-source MCP, rg, Python/unittest, CMake/Ninja/Zig, native
probe, launcher и Git. Arbor, web, subagents и многоэтажные сценарии не запускались.

## Native connectivity repair и C parity — 2026-10-18

Реализован opt-in `runtime.ensure_connectedness_native(allow_vault_disconnect=...)`:
исходные build_colors, multi-source join_region и join_regions из
`angband/src/gen-cave.c:1759–2084`. Сохранены FIFO/row-major tie-break,
4-соседний поиск после 8-соседней раскраски, обратное прорубание пути,
PERMANENT+ROCK и VAULT gates, сохранение дверей и passable terrain,
unconditional decrement попыток даже после неудачи. RNG не расходуется.
Применяется существующий generation terrain adapter, очищающий wall flags.

Отчёт разделяет initial_regions, logical_regions и пересчитанные physical_regions;
attempts/path/carved_cells являются диагностикой. Vault может оставить реальные
области раздельными при единственном логическом цвете — это не исправляется.
Добавлены 20 C fixtures (10 карт × allow false/true), вызывающие оригинальный
ensure_connectedness, с полным сравнением terrain, ROOM/GLOW/VAULT/wall flags
и полного RNG state. Проверяются пустая/одна/несколько областей, равные пути,
диагональ, permanent barrier, vault barrier, двери, vault root и wall flags.
В C изменён только test harness; production C не менялся.

Проверка этапа: 16 тестов test_connectivity + test_tunnel_parity прошли
за 22.759 s без skips; новая native connectivity проверка включает 20 карт.
Native harness пересобран CMake/Ninja/Zig; one_room won, 8/8 фаз;
оба git diff --check прошли. Использованы angband-source MCP, rg,
Python/unittest, CMake/Ninja/Zig, native C probe, launcher и Git.
Arbor, web, subagents и многоэтажные сценарии не использовались.

Следующий этап: подключить repair к restricted preset bridge после traditional
и расширить его native fixtures соответствующим C вызовом. Пока bridge прежний:
ensure_connectedness остаётся pending и generation_complete=False.

## Native region coloring — 2026-10-18

Перенесены `ignore_point`, `build_color_point`, `build_colors` для stairs=NULL
из `angband/src/gen-cave.c:1759–1850`: row-major seeds, FIFO, исходный порядок
8 соседей (опционально 4), PASSABLE либо DOOR_ANY. Новый `connectivity.py`
возвращает `NativeRegionColors` с плоскими colors/counts и диагностическим
visit_order; runtime предоставляет read-only `build_region_colors_native`.
Не используются passable overrides и square flags вместо terrain predicates.
Полная карта и положительные размеры — adapter preconditions; для 1-cell map
есть безопасный дополнительный слот counts, без заявления C parity этой границы.
Карта и RNG не изменяются. Stairs collection и clear_small_regions не переносились.

Проверки: 7 source-derived tests (включая все terrain из gamedata), существующий
native preset-bridge differential test без skips; one_room won, 8/8 фаз.
Это не новая native differential проверка самой раскраски: она ещё требуется
в следующем этапе соединения через оригинальный C ensure_connectedness.

Следующий этап: multi-source join_region/join_regions, native C fixtures
для tie-break, permanent barriers, vault true/false и итоговой связности.
Затем отдельное подключение к preset bridge. ensure_connectedness остаётся
pending; generation_complete=False, общий сценарный генератор не менялся.

## Аудит ensure_connectedness — 2026-10-18

Завершён source audit следующей стадии classic generation.
Подробности: `angband/plans/connectivity_repair_audit.md` относительно workspace.
Подтверждены 8-соседняя раскраска, 4-соседний multi-source BFS, отсутствие RNG,
точные terrain/vault gates и unconditional decrement числа попыток.
Логическое объединение через vault не гарантирует физическую связность;
это исходное поведение необходимо сохранить и диагностировать отдельно.
Модель и C на этом этапе не изменены; ensure_connectedness остаётся pending.

Следующий этап: перенос раскраски областей с проверками порядка обхода,
terrain/door predicates и сохранения RNG, затем отдельный перенос соединения
с native C parity и подключение к restricted preset bridge.
Общие scenario adapters и последующие стадии генерации остаются прежними.

## Restricted preset scheduler bridge — 2026-10-18

Этап завершён. Source MCP исследовал classic_gen room loop и boundary/
tunneling порядок (до refactor gen-cave.c:1130-1271); Arbor использован для
classic_gen dependencies и затем callers нового build_classic_rooms.

Добавлен generate_preset_rooms_and_tunnels_native: explicit simple/overlap
profile + fresh chunk dimensions -> полный granite фон -> source block
scheduler -> registered preset builder -> background installer -> PERM edge
после всех комнат -> native traditional tunneling. Другие builders/pit,
persistent/arena и occupied/invalid input отклоняются до RNG/мутаций.
Полные исходные classic profiles не фильтруются молча.

Scheduler поддерживает новый block_build_attempt callback (ровно один из
block/centre callbacks), передающий точный выбранный block в room_build
wrapper. Старый centre API сохранён. Context.perimeter_ready=false во время
room phase: classic C ставит permanent edge только после room loop.
Standalone background context сохраняет прежний default=true. Room footprint
защищён от perimeter даже до её превращения в PERM.

Исходный C room loop выделен в production helper build_classic_rooms,
который вызывает и classic_gen, и fixture. Проверка с Git HEAD подтвердила
дословное сохранение loop/решений/RNG; room_map cleanup остаётся caller,
blocks_tried освобождает helper. Нет новой приблизительной C копии scheduler.

12 executable fixtures (simple-only, mixed overlap/simple, goal<2 x4 seeds)
сверяют target/size_percent, built count, centers/reserved blocks, всю
карту4356 cells с ROOM/GLOW/wall flags, RNG после rooms и после туннелей.
Pen/jct=0 для integrated prefix; реальные door/trap branches проверяются
отдельными прежними195 fixtures. Native fixtures используют настоящий
room_build/build_simple/build_overlap/do_traditional_tunneling.

Report всегда generation_complete=false и перечисляет pending stages:
ensure_connectedness, streamers, stairs, rubble/traps, player placement,
monsters, objects/gold. Insufficient rooms -> отдельный status, без границы/
туннелей; partial map сохранена для диагностики с pending native cleanup.
Это не обещание полного failed-chunk cleanup или всего classic_gen.

73 выбранных unittest checks прошли (30.219 s), включая восемь native parity
tests, прежние builders/registration/background/scheduler/tunnel regressions.
Native probe пересобран CMake/Ninja/Zig. one_room won, 8/8 фаз; diff --check
обоих репозиториев чистый. Full suite, GUI и многоэтажные scenarios не запускались.
Старый generate_level_layout_native/default scenario pipeline не переключён.

Следующий этап после подтверждения: source audit ensure_connectedness /
region joining и текущих model adapters; затем ограниченный перенос
connectivity repair с C numerical fixtures. Остальные builders/content,
persistent/self-placement/cap modes и полное population ещё отдельная работа.
Подробности: angband/plans/tunnel_native_validation.md.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git/source comparison и diff --check.
Не использованы web search, GUI, подагенты; full suite и многоэтажные scenarios
не запускались.

## Complete mapped background installation — 2026-10-18

Этап завершён. Source MCP подтвердил square_set_feat generation branch
(cave-square.c:1236-1269) и permanent-wall predicate (245-284); Arbor
get_callees использован для навигации setter dependencies.

Добавлены validate_native_room_background и install_room_on_native_background;
simple/overlap builders принимают optional background_context. По умолчанию
старый sparse installer без изменений. Новый путь требует complete mapped
terrain и PERMANENT+ROCK perimeter; устанавливает только terrain-only
simple/overlap в незанятый GRANITE. Защищает previous reserved_blocks,
installed_room_cells, objects/traps/live monsters. installed_room_cells —
transient adapter validation bookkeeping, не новая игровая механика.

Проверяются все target features, полные flags/removals, glow footprint и
unsupported content до мутаций. Применяется native очистка wall flags,
source-backed ROOM/GLOW/OUTER/INNER metadata, passability/glowing updates.
Installer RNG не расходует. Неполный/невалидный background отклоняется до
builder RNG; неподходящий выбранный footprint не перематывает geometry RNG,
но не меняет карту/registry. В registered callback текущие blocks ещё не
зарезервированы — source reserve выполняется после successful return.

Native room fixture теперь использует PERM perimeter и выдаёт whole terrain
всех2400 клеток. 256 mapped-background cases (128 simple +128 overlap)
совпали с C по полной terrain карте, flags региона и всему RNG; отдельно
проверены passable и неизменность вне footprint. Sparse comparisons сохранены.
Original C builders/setters не менялись, только fixture входы/вывод.

Четыре новые unit regressions: invalid map до builder RNG, защита occupied/
reserved/permanent, missing/unsupported metadata без partial install,
интеграция с registered callback до reserve и старый overlap rejection.
Старый test route helper исправлен на PERMANENT+ROCK boundary: одного
PERMANENT недостаточно для native permanent wall.

69 выбранных unittest checks прошли (28.376 s), включая семь native parity
tests и прежние room/registration/scheduler/tunnel/junction checks.
C probe пересобран CMake/Ninja/Zig; comparisons без skip. one_room won,
8/8 фаз; diff --check обоих репозиториев чистый.

Ограничения: нет automatic scheduler switch, crossed/template/content
installation, self-placement/cap compatibility. Engine events/feat_count
не добавлены. Strict footprint guards — предел поддержанного adapter,
не новая C механика и не parity некорректных native inputs.

Следующий этап после подтверждения: ограниченный opt-in preset scheduler
bridge для simple/overlap, соединяющий registration, mapped-background
installation и native traditional tunneling. Остальные приблизительные
стадии level generation должны оставаться явно отмеченными adapters.
Подробности: angband/plans/tunnel_native_validation.md.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git diff --check. Не использованы
web search, GUI, подагенты; full suite и многоэтажные scenarios не запускались.

## Preset-centre native room registration — 2026-10-18

Этап завершён. Source MCP подтвердил gen-room.c:106-132,3625-3694 и
constants.txt:69,90; Arbor get_callees использован для room_build navigation.
Добавлены NativeRoomRegistration и build_registered_room_native. Config
загружает level_room_max=100 и level_pit_max=2 с SourceRef.

Preset lifecycle: minimum depth/pit/footprint gates -> centre запись и
cent_n++ -> builder callback -> при false только cent_n--, при success
reserve_blocks и pit count++. Context сохраняет raw slots, source-ordered
entrances/duplicates и last-writer ent2room. Failed entrances/lookup НЕ
стираются; reuse индекса сохраняет прежние входы, как исходный C.
Active room_centers/room_entrances отражают cent_n, не длину storage.

Full-cap quirk явно требует compatibility adapter и отклоняется до
callback; self-placement/find_space, room events и automatic scheduler
registration пока не подключены. Raw failure/collision context может не
пройти существующий более строгий tunnel validator: это не маскируется
очисткой native state. Exceptions callback не подменяются normal false.

append_entrance экспортирован в generate.h без изменения тела. Native
parity/tunnel расширен registration suite с настоящим room_build:
32 snapshots =4 seeds x8 шагов minimum-depth/pit-limit/bounds/success/
occupied/failure/reuse/pit-success. Сверены raw centre/entrance slots,
cent_n, callback-visible count, lookup, reserved blocks, pit count и весь
RNG. Callback fixture синтетический, исключительно для проверки lifecycle.
Новые unit tests проверяют SourceRef, cap rejection без мутаций и zero
extents/shared reservation set.

64 выбранных unittest checks прошли (25.328 s), включая шесть native parity
тестов, все прежние room flags/tunnel/junction и scheduler/find_space checks.
C probe пересобран CMake/Ninja/Zig; native comparisons без skip. one_room
won, 8/8 фаз; diff --check обоих репозиториев чистый. Full suite и
многоэтажные scenarios не запускались.

Следующий этап после подтверждения: generation-aware installation в полный
mapped background для simple/overlap, с native wall-flag clear semantics и
явной защитой permanent boundary/room reservations; затем ограниченный
preset scheduler bridge. Crossed/content, self-placement и capacity-mode
остаются отдельными этапами. Подробности: tunnel_native_validation.md.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git diff --check. Не использованы
web search, GUI, подагенты; full suite и многоэтажные scenarios не запускались.

## Overlap builder native flags и C parity — 2026-10-18

Этап завершён. Source MCP подтвердил gen-room.c:2128-2195 перед изменением
правил; Arbor get_callees использован для build_overlap navigation.
В model overlap helper перенесён порядок generate_room(A/B), outer border
A/B и floor fill A/B, включая side setter/marking каждого border.
ROOM/GLOW выставляются всему union; прежний GLOW сохраняется при light=false.
Каждая замена terrain снимает INNER/OUTER/SOLID перед optional OUTER marking.
Interior FLOOR второго rectangle корректно удаляет OUTER первого.

Layout теперь передаёт square_flags, square_flags_clear и исходный centre.
Native registry и разрешение overwrite полного mapped background не
добавлялись; старый automatic scheduler/routing не переключён.

Native parity/tunnel test_room_flags переиспользует fixture setup для
настоящих build_simple/build_overlap. Добавлена overlap-flags suite:
128 cases = seeds1..64 x depth1/чистые flags и depth30/прежние
ROOM+GLOW+SOLID. Сверены terrain, ROOM/GLOW/OUTER/INNER/SOLID региона27x13
с полной асимметричной комнатой и окрестностью, glowing_terrain и весь RNG.
Исходные game builders не менялись. 128 simple cases продолжают проходить.
Existing overlap unit test дополнен проверкой overwritten OUTER -> FLOOR
на клетке(12,10), внешнего OUTER на(14,10) и centre metadata.

56 выбранных unittest checks прошли (24.828 s), включая пять native parity
тестов и связанные simple/overlap/crossed/scheduler/template/tunnel checks.
Native probe пересобран CMake/Ninja/Zig; сравнения выполнялись без skip.
one_room won, 8/8 фаз; diff --check обоих репозиториев чистый.
Full suite и многоэтажные scenarios не запускались.

Следующий этап после подтверждения: temporary native room registration
context и SourceRef для dun-gen:cent-max=100; exact centre-before-builder,
успех/отказ и ordered entrance mapping. Начать preset-centre режимом;
self-placement/cap compatibility quirks отдельно. Затем background installer
и ограниченная opt-in scheduler integration. Crossed flags/content пока
остаются неподключёнными. Подробности — tunnel_native_validation.md и
room_registration_audit.md в angband/plans.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git diff --check. Не использованы
web search, GUI, подагенты; full suite и многоэтажные scenarios не запускались.

## Simple builder native flags и C parity — 2026-10-18

Этап завершён. Source MCP подтвердил gen-room.c:2017-2118 перед изменением
правил; Arbor get_callees использован для навигации build_simple helpers.
В runtime simple builder перенесены ROOM/GLOW всего прямоугольника,
OUTER border, INNER pillars/ragged indentations, source corner exceptions
с сохранением GLOW. Добавлен исходный centre в layout, без новой регистрации.

Layout square_flags_clear явно снимает прежние INNER/OUTER/SOLID на
записываемых клетках и ROOM на исключённых pillared углах. Installer
применяет removals перед итоговыми flag additions; без этого opt-in поля
его поведение сохраняется. Existing GLOW и unrelated flags сохраняются.
Нельзя выводить wall type только из финальной GRANITE геометрии.

Native parity/tunnel расширен simple-flags suite: реальный build_simple
с preset centre(30,20), 128 fixtures = seeds1..64 x depth1/чистые flags
и depth30/старые ROOM+GLOW+SOLID по (x+y)%7. Сверены terrain, flags
ROOM/GLOW/OUTER/INNER/SOLID в27x13 region с полной комнатой и окрестностью,
subtype и полный RNG. Набор реально содержит ordinary/pillared/ragged.
Model glowing_terrain отдельно сверяется с GLOW. C build_simple не менялся.
Existing unit pillar test расширен явными assertions для corner removal.

54 выбранных unittest checks прошли (23.971 s), включая четыре native
parity tests, builder/scheduler, template metadata и tunnel regressions.
Native probe пересобран (CMake/Ninja/Zig) и его suites выполнялись без skip.
one_room won, 8/8 фаз; diff --check обоих репозиториев чистый.

Границы: self-placement find_space, center registry и installation в полный
mapped фон остаются отдельными этапами. Новый metadata contract ещё не
является полной generation setter интеграцией. Общий scheduler и модельный
маршрут не переключались. Overlap/crossed пока без соответствующих flags.
Подробности: angband/plans/tunnel_native_validation.md и room_registration_audit.md.

Следующий этап после подтверждения: overlap ROOM/GLOW/OUTER и source order
двух borders/floors с очисткой overwritten wall flags; добавить C fixtures
асимметричных пересечений, сохранения GLOW и полного RNG. Затем registration
context + cent-max SourceRef и generation-aware background installation.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git diff --check. Не использованы
web search, GUI, подагенты; full suite и многоэтажные scenarios не запускались.

## Аудит room registration / builder flags — 2026-10-18

Этап аудита завершён, код модели/scheduler не менялся. Подробная таблица
трассировки, source formulas, ограничения и порядок следующих исправлений:
angband/plans/room_registration_audit.md. Arbor MCP использован для
room_build navigation; source MCP подтвердил gen-room.c helpers/builders,
registration и template entrance rules.

Обнаружены блокирующие разрывы для общего native tunneling pipeline:
1. Procedural simple/overlap/crossed layouts не передают полноценные
   ROOM/OUTER/INNER/SOLID flags, необходимые маршрутам и junction predicate.
2. Native room_build регистрирует centre до builder; model scheduler
   сохраняет success record после callback без общего cent/ent2room context.
3. find_space возвращает centre и резервирует blocks, но callback модели
   не регистрирует centre до template content/entrance processing.
4. Integrated generator соединяет средние координаты terrain соседних
   layouts вместо зарегистрированных centers и native scrambled order.
5. Layout installer отвергает уже mapped фон и объединяет flags без native
   wall-flag clear semantics, тогда как tunnel opt-in требует complete map.

Диагностически подтверждено на текущих helpers: seed7/depth5/centre(40,40)
в map80x80: simple75 terrain cells, overlap137 (следующий вызов того же RNG),
оба0 square_flags records и без centre/entrances metadata. Числа являются
наблюдением, не константами игры. Templates уже несут flags/ordered entrances,
но registration bridge и mixed-content handoffs ещё не завершены.

Найденный следующий перенос: simple ROOM/GLOW/OUTER/INNER и corner exceptions
по gen-room.c:2043-2114, с C fixtures обычной/pillared/ragged геометрии.
Затем overlap setter order, temporary registration context + cent-max100
SourceRef, generation-aware installation полной фоновой granite map;
только затем ограниченный opt-in scheduler. Crossed/content и self-placement
подключать после отдельных проверок. Cap/failure quirks и conflicting
entrance ownership требуют compatibility audit, не произвольного исправления.

Проверка: 10 выбранных unittest checks прошли (17.020 s): три native
parity tests и семь builder/scheduler/find_space regressions. Native cases
не стали доказательством корректности procedural builder metadata — его
нехватка явно зафиксирована. one_room won, 8/8 фаз; diff --check чистый.

Использованы Arbor MCP, angband-source MCP, rg, Python diagnostic/unittest,
существующий native probe, run_mvp.sh, Git diff --check. Не использованы
CMake/Ninja/Zig в этом этапе (новой сборки не было), web search, GUI,
подагенты; full suite и многоэтажные scenarios не запускались.

Следующий этап после подтверждения: перенести flags simple builder и
добавить C differential fixtures, сохраняя текущий automatic scheduler.

## Native junction doors/traps и полный opt-in caller — 2026-10-18

Этап завершён. MCP подтвердил gen-cave.c:774-902 и trap.c:356-398;
Arbor get_callees использован для навигации по try_door dependencies.
Добавлены possible_junction_doorway_native, try_junction_door_native,
finish_tunnel_junctions_native, do_traditional_tunneling_native.

Сохранены FLOOR вне ROOM, opposing strongwalls, skips до RNG, отдельные
randint0(100) и randint0(500), short-circuit geometry, random door/lock и
trap helper, W/E/N/S, дубликаты и live previous placements. Trap context
явный; новые junction traps/locks помечаются SQUARE_TRAP. Sparse footprint
проверяется до RNG/мутаций. Общий caller возвращает pending=false;
connection-only helper сохраняет pending=true. Scheduler не переключён.

try_door экспортирован в generate.h без изменения тела. Native probe
расширен 195 direct junction fixtures, включая реальный OPEN/BROKEN/CLOSED,
lock и trap outcomes. Сверены feature, trap/lock power/kind, marker и весь
RNG. Fixture catalog синтетический и изолирован внутри теста: один trap
rarity1/min_depth0/power0 плюс default-power lock; это не игровые параметры.
Seeds1..64+83 (83 обеспечивает BROKEN). Также сверён final RNG полного C
traditional caller на прежних 12 configurations с jct=0, сохранив 28 route
snapshots. Прежние 36 individual route cases продолжают проходить.

Добавлены пять unit regressions для geometry, skips, zero chance/failed
geometry draws, W/E/N/S+duplicates+live doors, sparse input rejection.
46 unittest checks прошли (22.919 s), native probe 3/3 suites; one_room won,
8/8 фаз; diff --check обоих репозиториев чистый. Full suite и многоэтажные
scenarios не запускались. Подробности: angband/plans/tunnel_native_validation.md.

Границы: нет parity полного generated level с реальным trap catalog,
нет UI redraw/event adapter parity. Ненулевой jct проверен direct fixtures,
полный multi-room caller — пока jct=0. Это не основание глобально включать
новый pipeline для ещё непроверенных builders.

Следующий этап после подтверждения: аудит native room registration и
ROOM/OUTER/INNER/SOLID flags поддерживаемых builders; затем ограниченная
opt-in scheduler integration с честным отчётом об оставшихся adapters.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, native probe, run_mvp.sh, Git diff --check. Не использованы
web search, GUI, подагенты; full suite и многоэтажные scenarios не запускались.

## Native traditional room connection prefix — 2026-10-17

Этап завершён. Через Arbor MCP исследован get_callees для
do_traditional_tunneling; source MCP подтвердил gen-cave.c:841-902.
В runtime.py добавлен connect_rooms_traditional_native с явными
room_centers/room_entrances/entrance_to_room/profile и shared junction list.
Общая preflight validation выделена из build_tunnel без изменения правил.

Сохранён точный порядок: N произвольных swaps через два randint0(N) на
каждый swap, очистка junction buffer, выбор входа последней комнаты bias80
без target, sentinel -> center, затем по scrambled order выбор live входа
к previous и build_tunnel_native(endpoint, previous). Previous обновляется
выбранным endpoint даже при preemptive/guard route failure. Индексы и
регистрация комнат не переставляются. Empty/inconsistent context отклоняется
до RNG/мутаций. N=1 поддержан без лишних RNG draws.

Возвращаемые поля: room_order, initial_endpoint, routes, junction_cells,
junction_phase_pending=True. Это только connection prefix до final junction
doors/traps; общий automatic scheduler по-прежнему не переключён.

C do_traditional_tunneling стал доступен через generate.h (снято static,
тело без изменений). parity/tunnel probe расширен event-снимками каждого
маршрута настоящего C caller. В 12 конфигурациях (center fallback, marked
entrances, single room x seeds1/7/19/42) проверены 28 route boundaries:
ordered buffers, terrain/flags всех клеток, event fields и полный RNG.
Сравнение заканчивается на tunnel-finished event, до последующих C junction
checks; их RNG/результаты не объявляются перенесёнными.

Три новые unit regressions отдельно проверяют swaps/RNG, selected previous
при failed route, single room и invalid input без потери junction buffer.
Все 40 выбранных unittest tests прошли (8.779 s), включая два C differential
теста: прежние 36 individual route fixtures и новые 28 snapshots.
CMake/Ninja/Zig probe пересобран через прежний isolated /tmp toolchain.
one_room won, 8/8 фаз; diff --check обоих репозиториев чистый.
Подробности: angband/plans/tunnel_native_validation.md.

Наблюдение об инструментах: Arbor get_callees ошибочно связал C randint0
с Python flavor_data.py. Это зафиксировано в tools/arbor/README.md; граф
использован для навигации, правила подтверждены source MCP и C runtime.

Следующий этап после подтверждения: native next_to_corr/possible_doorway /
try_door и final junction placement в порядке W,E,N,S после всех маршрутов,
с отдельной проверкой door/trap RNG. До этого не называть helper полным
эквивалентом do_traditional_tunneling и не подключать automatic scheduler.

Использованы Arbor MCP, angband-source MCP, rg, Python/unittest, CMake,
Ninja, Zig C, run_mvp.sh, Git diff --check. Не использованы web search,
GUI, подагенты; full suite и многоэтажные сценарии не запускались.
В следующих отчётах сохранять итоговый список использованных/неиспользованных
утилит по запросу пользователя.

## Native C differential harness для tunnel routes — 2026-10-17

Этап завершён: алгоритм модели не менялся, соответствие расширено
исполняемым C probe. Arbor MCP нашёл caller build_tunnel и native test
точки; angband-source MCP подтвердил исходники/harness перед изменениями.

Добавлен angband/src/tests/parity/tunnel.c и CMake target
unittest-parity-tunnel. Оригинальный build_tunnel стал доступен через
src/generate.h (снято static); тело алгоритма без изменений. Добавлен
SandboxCore2026/projects/angband_model/tests/test_tunnel_parity.py.

На 36 fixtures (9 карт x seeds1/7/19/42) совпали полные ordered tunnel/wall/
junction buffers, terrain и ROOM/OUTER/INNER/SOLID всех 220 клеток, native
event loop count/start-end distances/early flag и RNG index+32 state words.
Покрыты random bends, marked room извне/изнутри, blocked/adjacent target,
лимит tunnel buffer, diagonal post-wall и unusable exit retry. Наблюдатели
подтверждают реальные diagonal и missing-normal вызовы, не только название
fixture. kind7 завершается guard после diagonal и совпадает с C; kind8/
seed42 содержит две неудачные нормали. Это не означает native parity всего
уровня, pen doors или всех 20 retry exhaustion вариантов.

При первом запуске обнаружена ошибка C fixture initialization:
Rand_state_init не сбрасывает state_i. Перед каждым независимым fixture
теперь state_i=0; после этого исправления harness все сравнения проходят.
Численные правила модели не подгонялись под ошибочно настроенный probe.

Системные CMake/compiler/C headers отсутствуют, sudo требует пароль.
Сборка выполнена без sudo через isolated /tmp pip wheels CMake3.31.10,
Ninja1.13.2 и Zig0.15.2 с bundled C headers. Временный source overlay
отключает автоматический X11 fallback при test frontend; оригинальные C
файлы используются напрямую. Старый build cache со старым путём сохранён.
Подробности, команды и ограничения: angband/plans/tunnel_native_validation.md.

Итог: 36 unittest checks прошли (7.471 s): 35 прежних regressions + один
C differential test с 36 subcases. one_room won, 8/8 фаз; diff --check
обоих репозиториев чистый. Full suite и многоэтажные scenarios не запускались.

Следующий этап после подтверждения: native traditional room ordering /
соединение с explicit centers и registration; затем отдельным этапом
junction doors/traps. Автоматический scheduler остаётся на старом adapter
до проверки корректности ROOM/wall flags и регистрации builders.

Утилиты этапа: использованы Arbor MCP, angband-source MCP, rg, Python,
CMake, Ninja, Zig C, unittest, run_mvp.sh и git diff --check. Не использованы
web search, GUI, full suite, многоэтажные scenarios, подагенты. По запросу
пользователя в конце следующих отчётов указывать использованные и
неиспользованные утилиты.

## Opt-in native build_tunnel orchestration — 2026-10-17

Этап завершён. Read-only MCP подтвердил gen-cave.c:492-761 перед переносом.
В runtime.py добавлен build_tunnel_native(start,target,profile=...,
room_entrances=...,entrance_to_room=...,junction_cells=list).
Все generation buffers временные: tunnel/wall создаются на каждый вызов,
junction list остаётся общим и сохраняет порядок/предыдущие записи.

Цикл связывает native direction/normal/post-wall/pierce/apply helpers:
nested chg/rnd, permanent/solid obstacles, goal outer wall, защита соседней
outer цели, marked entry изнутри/снаружи, максимум 20 exit attempts с bias
80 и его source decay, unmarked inner normal, ROOM fast traversal,
отложенные tunnel cells, intersection door_flag и preemptive termination.
Исходный guard postfix сохранён: максимум 2001 обработанная итерация,
loop_count=2002 при exhaustion. Переполнение буфера не останавливает путь.
Применение terrain/pen doors выполняется после цикла без fallback маршрута.

Возвращается отчёт start/target/end, ordered tunnel_cells/wall_piercings,
loop_count, Manhattan distances, preemptive, guard_exhausted, reached_target
и application. Native engine event остаётся адаптером, дополнительные
engine side effects не заявлены.

Первый поддержанный opt-in contract требует complete mapped chunk,
permanent perimeter без INNER, внутренние endpoints и согласованные
contiguous room_entrances/entrance_to_room. Это явные ограничения адаптера,
не новые игровые правила. Невалидный ввод проверяется до RNG/мутаций.
Существующие carve_generated_tunnel и automatic scheduler не переключались.
Traditional room shuffle/connection order и junction door/trap stage ещё
не перенесены. Registry всех builders и executable native parity остаются
отдельной работой; не все ветви подтверждены differential fixtures.

Добавлены 10 regressions: straight route/full и zero buffers, shared
junction limit/early termination, guard/solid obstacle, outer goal/equal
endpoints, unmarked room/cardinal bend suppression, marked room entry+exit,
input rejection, nested random direction draws, marked exit изнутри,
защита outer рядом с целью. Полное RNG state проверено для выбранных
прямых, blocked, room и intersection маршрутов. 35 выбранных regressions
прошли (5.193 s), затем усиленный marked room RNG test отдельно прошёл
(0.066 s). one_room won, 8/8 фаз; diff --check обоих репозиториев чистый.
Full suite, native executable parity и многоэтажные сценарии не запускались.

Следующий этап после подтверждения: расширить branch fixtures для retry /
diagonal route cases и подготовить native C differential harness перед
переключением generation scheduler; затем отдельно traditional ordering
и заключительные junction doors/traps по аудиту tunnel_routing_audit.md.

## Native post-wall bookkeeping и generation limits — 2026-10-17

Этап завершён. Read-only MCP подтвердил gen-cave.c:342-390 и
lib/gamedata/constants.txt:68-80 перед изменением поведения.
GameConfig загружает tunn_grid_max=900 и level_door_max=200 с SourceRef;
значения не подставляются fallback-константами.

Добавлен runtime.handle_post_wall_step_native(position, direction,
door_flag=..., tunnel_cells=list). Cardinal возвращает прежние position,
direction, door_flag и bend_intvl=1 без движения/RNG/terrain reads.
Diagonal сразу перемещается; GRANITE вне ROOM добавляется в caller-owned
ordered buffer при свободном месте, door_flag=false даже при заполнении.
Затем bend_intvl=0, randint0(32768)<16384 обнуляет x, иначе y.
Terrain не прорубается; helper возвращает обновлённое transient состояние.

Model adapter preconditions: unit direction и mutable tunnel list;
для diagonal требуются bounds, mapped terrain и отсутствие permanent либо
строгих GRANITE+INNER/OUTER/SOLID, как в native assertion. Невалидный ввод
отклоняется до записи и RNG. Non-granite wall flags не блокируют шаг.

Пять новых tests проверяют значения/SourceRef, cardinal no-RNG, все diagonal
направления и полный RNG state seeds1/7/19/42, full buffer, ROOM/FLOOR,
strict wall flags и invalid input. Всего 25 выбранных regressions прошли
(4.306 s); one_room won, 8/8 фаз; diff --check обоих репозиториев чистый.
Native executable parity, full suite и многоэтажные smoke не запускались.

Следующий этап после подтверждения: opt-in native build_tunnel orchestration
с явным контекстом комнат/входов и общим junction buffer, соединяющий уже
готовые directions, normal, post-wall, pierce и apply helpers. Существующий
L-образный adapter и scheduler на этом этапе не переключались.

## Native directions и wall normal — 2026-10-17

Этап завершён. Перед изменением правил read-only MCP подтвердил
src/gen-util.c:323-349 (correct_dir/rand_dir) и src/gen-cave.c:404-433
(find_normal_to_wall); порядок ddgrid_ddd подтверждён cave.c:65-73.

В generation_data.py добавлены native_correct_direction и
native_random_direction, экспортированы из пакета. Первый расходует RNG
только при двух ненулевых осях; второй использует native cardinal order.
В runtime.py добавлен find_normal_to_wall_native с live ROOM, PERMANENT,
строгими GRANITE+wall predicates, bounds и приоритетом cardinal кандидатов.
Нет кандидатов -> (0,0); один -> randint0(1), без продвижения RNG.
Неполная/неизвестная terrain map и неверный исходный wall отклоняются до RNG.

Пять новых regression tests покрывают порог 49/50, все направления,
совпавшие endpoints, cardinal/diagonal order, inner/outside, строгие wall
flags, permanent, границы и invalid input. Полное RNG state сверяется на
seeds 1/7/19/42. Всего 20 выбранных новых и прежних entrance/piercing/door/
tunnel tests прошли (1.888 s). one_room won, 8/8 фаз; diff --check чистый.
Это source-backed unit проверки; native executable parity не заявляется.

Общий маршрут и существующий L-образный adapter ещё не переключены.
Следующий этап после подтверждения: добавить source-backed tunn_grid_max /
level_door_max и post-wall bookkeeping с diagonal step, tunnel buffer и
cardinal bend suppression. Затем отдельным этапом build_tunnel orchestration.

## Аудит native build_tunnel routing — 2026-10-17

Этап аудита завершён; код маршрутизации не менялся. Подробная трассировка,
ветви/RNG, параметры, ограничения и поэтапная интеграция сохранены в
`angband/plans/tunnel_routing_audit.md` относительно workspace.

Через работающий MCP исследованы correct_dir/rand_dir (gen-util.c:323-349),
find_normal_to_wall и handle_post_wall_step (gen-cave.c:342-433), основной
цикл/применение/событие (492-761), traditional ordering и junction doors/traps
(774-900), порядок направлений cave.c:65-73. L-образный carve_generated_tunnel
остаётся явным адаптером: немедленное прорубание не эквивалентно native
отложенным buffers и немедленному изменению wall flags.

Нужны source-backed лимиты tunn_grid_max=900 и level_door_max=200,
временный контекст room centers/entrances/ent2room и общий junction buffer.
Зафиксированы nested chg/rnd draws, cardinal preference, diagonal post-wall
step, guard postfix counter, 20 entrance retries, preemptive termination,
не-Fisher–Yates shuffle и отдельная заключительная стадия junction traps.
Они не подменяются shortest path и не объявляются уже реализованными.

Проверка: 15 существующих entrance/piercing/door/tunnel regressions прошли
(1.469 s); one_room won, 8/8 фаз; diff --check в обоих репозиториях чистый.
Native executable parity маршрутов ещё не проверялась; full suite и
многоэтажные сценарии не запускались.

Следующий этап после подтверждения: перенести direction helpers и
find_normal_to_wall с exact ordered candidates, terrain preconditions и
проверкой полного RNG state; затем отдельным этапом post-wall bookkeeping.

## Применение recorded tunnels/piercings и native random doors — 2026-10-17

Этап завершён. MCP подтвердил gen-cave.c:435-469,738-756,
gen-util.c:587-612, trap.c:706-725, cave-square.c:288-343,1236-1268 и
trap.txt:73-77. Добавлены apply_recorded_tunnel_native(),
allows_wall_piercing_door_native() и place_random_door_native().

Применение: все записанные corridor cells -> FLOOR, затем каждый wall ->
FLOOR и безусловный randint0(100) по explicit profile.tunnel[3]. Только
при успешном chance проверяются live 3x3 inside/outside passable/rubble
клетки, исключая doors/shops. Source predicate пропускает только absolute
(0,0), включает центр; его особенности сохранены.

Random door: <30 OPEN, <40 BROKEN, иначе CLOSED; closed one_in(4) ->
randint1(7). При новом lock выполняется default-power randcalc RNG перед
перезаписью power. При существующих locks power меняется у всех без
повторной creation RNG и без удаления duplicate locks. Map setter очищает
INNER/OUTER/SOLID, сохраняет остальные флаги и обновляет passability.

Профиль обязателен: не выбирается автоматически с дополнительным RNG.
Маршрут и списки уже рассчитаны вызывающим кодом; для model sparse maps
полнота нужного terrain проверяется до мутаций и RNG. Общий build_tunnel,
выбор маршрута, engine events и автоматический mixed scheduler ещё не
подключены. Старый упрощённый place_generated_tunnel_doors не подменяется
незаметно новой стадией.

Семь новых regressions: пороги door branches/new lock RNG, source center/
origin/rubble/door/shop predicate, zero chance с обязательным draw, live
interleaving дверей, unmapped rejection, полный RNG на seeds1/7/19/42,
existing duplicate locks без лишнего RNG + snapshot. 45 выбранных tunnel/
door/entrance/room/snapshot tests прошли (4.472 s), затем две добавленные
проверки прошли отдельно (0.385 s). Launcher one_room won, 8/8 фаз;
git diff --check чистый. Full suite, native executables и generated-floor
сценарии не запускались.

Следующий этап после подтверждения: исследовать и перенести ограниченный
native build_tunnel route/step orchestration с явной регистрацией комнат;
соединить entry choice, wall recording и application без придуманного
shortest-path fallback. Сначала source-audit ветвей и RNG порядка маршрута.

## Native pierce_outer_wall bookkeeping — 2026-10-17

Этап завершён. MCP подтвердил gen-cave.c:296-322 и caller/apply boundaries
(564,599,658,748-756), cave-square.c:1236-1268 и источник лимита
constants.txt:75. GameConfig теперь загружает wall_pierce_max из
`dun-gen:wall-max` (500) с SourceRef.

Runtime pierce_outer_wall_native(grid, wall_piercings=list) сохраняет
координату в caller-owned per-tunnel списке, пока не достигнут лимит;
независимо от заполнения списка обходит 3x3 и превращает только
GRANITE+OUTER flags в SOLID, снимая wall helper flags и сохраняя ROOM/GLOW.
Точное исходное условие adj.x!=0 && adj.y!=0 сохранено: axis-zero cells
пропускаются, сам центр обрабатывается. Это особенность оригинала, не
исправление модели; альтернативный corrected mode не добавлялся.

Helper не превращает стены в FLOOR и не ставит двери: исходник делает это
позже в build_tunnel. RNG не расходуется. Список wall_piercings является
временным generation context вызывающего кода, не выдуманным persistent chunk
полем. Пропущенные terrain inputs отклоняются до записи/изменения flags.

Четыре новые regressions: source limit/row order/flags, full buffer и axis
quirk, следующая entrance choice + snapshot, отказ на unmapped neighbors без
частичных изменений. Все 40 выбранных piercing/config/entrance/room/snapshot
tests прошли (3.723 s). Launcher one_room won, 8/8 фаз; git diff --check чистый.
Full suite, native executables и generated-floor сценарии не запускались.

Следующий этап после подтверждения: application stage build_tunnel
(gen-cave.c:748-756): записанные walls -> FLOOR, затем native door chance,
allows_wall_piercing_door и place_random_door. Ранее перенесённый chooser
ещё не означает готовность полного build_tunnel/mixed-room scheduler.

## Native marked entrance choice — 2026-10-17

Этап завершён. MCP подтвердил choose_random_entrance
(gen-cave.c:180-293), strict granite+OUTER predicate (86-92) и native distance
(cave-view.c:38-45). Добавлен choose_native_room_entrance(): исходный порядок
и повторяющиеся кандидаты сохраняются; без target вес 1; с target вес
max(1,bias-distance)^2. Исключаются соседние клетки exc, но не сама exc.
Eligible exact target возвращается без RNG; иначе один randint0(sum weights);
при отсутствии подходящих кандидатов — нативный sentinel (0,0), без RNG.
Отрицательный bias и int32 overflow weights отклоняются, не угадываются.

Runtime choose_room_entrance_native() требует явную contiguous mapping
room_index -> ordered candidates; проверяет текущие terrain и OUTER flags.
Пробитый FLOOR с устаревшим OUTER flag не квалифицируется. Глобальная native
регистрация dun->ent/ent2room остаётся вызывающему коду, не выводится из
произвольного порядка layout. Tunnel piercing/прокладка пока не подключены.
entrance_selection_requires_adapter на layout относится к этой ещё не
подключённой integration boundary, а не к отсутствию алгоритма выбора.

Четыре новых regressions: cumulative boundaries/bias/native diagonal distance/
duplicates, exclusions и exact/no-choice без RNG, invalid inputs, explicit
room indices и live walls на реальном Ammo dump. Дополнительно полное RNG
state сверено с единственным randint0(18) на seeds 1,7,19.
35 entrance/room/snapshot tests прошли (3.570 s); после добавления RNG-state
assertions соответствующий test повторно прошёл. Launcher one_room won,
8/8 фаз; git diff --check чистый. Full suite, native executables и
generated-floor сценарии не запускались.

Следующий этап после подтверждения: native pierce_outer_wall(), учёт списка
пробитых стен и запрета соседних piercing, затем связывание chooser и room
registration с tunnel caller. Не заменять (0,0) на произвольную ближайшую
стену: source fallback должен переноситься из вызывающего native кода.

## FEW_ENTRANCES и room/wall metadata — 2026-10-17

Этап завершён на уровне metadata комнаты. Через MCP подтверждены
append_entrance (gen-room.c:106-132), первый проход/ROOM/GLOW
(1175-1251), второй проход literal # (1268-1283), place_secret_door
(gen-util.c:576-579), set_marked_granite (gen-room.c:425-428) и граница
choose_random_entrance (gen-cave.c:200-267).

Room layout возвращает entrances в исходном порядке обхода с учётом symmetry;
FEW_ENTRANCES записывает только % cells. Возвращаются SQUARE_ROOM/GLOW,
WALL_OUTER/SOLID/INNER: literal # становится INNER только при всех восьми
ROOM neighbors, optional x/) и незадействованные numeric doors остаются SOLID.
Второй проход metadata выполняется без дополнительных draws. Runtime resolver
учитывает уже известные SQUARE_ROOM соседних комнат. Свет выбранного builder
отражается в flags и glowing, отдельный direct resolver по умолчанию тёмный.

Перенос layout сдвигает entrance coordinates и literal_solid_walls, копирует
наборы flags и пересчитывает SOLID/INNER по destination neighbors без изменения
исходного layout/RNG. Установка и snapshot сохраняют map flags. Native Ammo
dump - arrows возвращает 11 % entrance candidates.

ГРАНИЦА: entrances — per-layout candidates, не зарегистрированный native
глобальный dun->ent/ent2room. entrance_selection_requires_adapter=true и
inline_geometry_adapters=FEW_ENTRANCES остаются: выбор входа, исключения,
bias и tunnel consumption ещё не перенесены. Не объявлять весь
FEW_ENTRANCES/native dungeon generator завершённым.

Пять новых regressions: все rotations/reflections/source order/RNG neutrality,
wall kinds и свет, existing room neighbors, real Ammo dump install/translation/
snapshot, destination-neighbor reclassification без мутации source.
31 выбранный room/metadata/snapshot regression прошёл (3.368 s). Launcher
one_room won, 8/8 фаз; git diff --check чистый. Full suite, native executables
и generated-floor сценарии не запускались.

Следующий этап после подтверждения: native choose_random_entrance и явное
соответствие room index/candidates, с проверкой bias, exclusions, source
RNG order и fallback. Подключение tunnel builder и mixed scheduler — после
этой границы, без замены на произвольную ближайшую входную клетку.

## Native ammunition generation rating — 2026-10-17

Этап завершён. Добавлен native_ammo_generation_rating(state): для достаточно
полного ammo state вычисляет real power/value и source OOD adjustment без
RNG и без изменения player knowledge. Нет fallback на kind cost, cached value
или object_power. Неполный/unsupported state (включая artifact power) явно
отклоняется; явный внешний rating adapter остаётся доступным.

Переиспользован и выделен общий power calculation из существующего guarded
quiver branch. Quiver по-прежнему требует native_value_known; generation
проверяет реальные полные свойства, не устанавливая этот knowledge flag.
Через MCP проверены obj-power.c:1005-1048,1099-1210 и формулы to_damage/to_hit;
OOD — obj-make.c:1211-1231. Source-backed callback помечает rating_source как
native_ammo_power, внешний — explicit_adapter.

Четыре новые regressions прошли (0.328 s): explicit combat formula/OOD,
нет изменений RNG/knowledge/registry, отказ на incomplete/unsupported state,
сохранение quiver guard, реальный Ammo dump - arrows с native rating.
Результаты комнаты (предметы/количества/позиции/ловушки и полный RNG) совпали
с прежним запуском через synthetic pure rating callback.

Расширенный отбор: 90 tests, 89 прошли сразу; один native drop-count probe
остановился на PermissionError, поскольку архивный ELF не имел execute bit.
Восстановлен user execute для angband/build/unittests/parity/native/native;
повторный запуск единственного упавшего теста прошёл. Этот native probe
проверяет drop count, НЕ ammunition valuation parity. Штатный one_room won,
8/8 фаз; git diff --check чистый. Full suite и generated-floor сценарии
не запускались.

Следующий этап после подтверждения: FEW_ENTRANCES/entrance metadata и
wall/room flags реального Ammo dump template; затем отдельное безопасное
подключение подтверждённого mixed room content к scheduler. Не считать
готовым полный native dungeon generator или artifact valuation.

## Typed ammo placement и первый реальный mixed template — 2026-10-17

Этап завершён. Добавлен place_typed_ammo_object_native(): bounds и
square_canputitem до RNG; создание detached ammo; явный обязательный final
make_object rating adapter; ORIGIN_SPECIAL/origin_depth; empty-pile carry;
cleanup created artifact при отказе; good_item и obj_rating saturation.
Через MCP проверены gen-util.c:498-540, obj-pile.c:905-975,
obj-util.c:686-690, obj-make.c:1211-1231 и cave.c:344-347.

Полная native стоимость/OOD не подставляется из базовой цены: rating_adapter
обязан вернуть final int32 make_object value, включая OOD. Предметы маркируют
rating_source=explicit_adapter. Empty floor — предусловие place_object,
поэтому здесь нет merging/ignored eviction, требующих занятой pile.
ChunkState хранит obj_rating/good_item, snapshot/restore/reset их сохраняют
или сбрасывают; старые snapshots без полей получают zero-initialized defaults.

Room builder принимает typed_ammo_rating_adapter вместе с inline_trap_context.
Whitelist расширяется на [ только для arrow/bolt/shot и при наличии adapter.
Для Ammo dump - arrows отдельная проверка использует исходный каталог:
три object allocations идут до trap allocation; origin и floor order заданы;
layout содержит inline_objects/inline_traps и не оставляет deferred markers.
FEW_ENTRANCES явно отмечен в inline_geometry_adapters, не объявлен реализованным.
Если rating adapter падает посреди комнаты, новые objects/traps, artifact
created state и rating откатываются, RNG не перематывается; ошибка передаётся
вызывающему коду. Автоматический scheduler этого opt-in ещё не включает.

Пять новых regressions покрывают preconditions без RNG, origin/rating/clamp/
snapshot, floor refusal artifact cleanup, реальный room template и ошибку
rating посреди комнаты. 67 выбранных placement/builder/snapshot/drop tests
прошли (7.036 s); после последней правки шесть focused tests прошли (0.820 s).
Launcher one_room won, 8/8 фаз; git diff --check чистый. Full suite, native
executables и generated-floor сценарии не запускались. Fixture rating=0 в
native-template test — явный test adapter, НЕ доказательство native valuation.

Следующий этап после подтверждения: source-backed native ammunition value
для достаточно полного object state, с явным отказом на неполных power inputs;
сохранить adapter для неподтверждённых случаев. После этого отдельно решать
FEW_ENTRANCES и подключение mixed room orchestration к scheduler.

## Typed ammunition make_object primitive — 2026-10-17

Этап завершён. Добавлен `make_typed_ammo_object_native()` для arrow/bolt/shot.
MCP подтвердил obj-make.c:1162-1210 и special-artifact path 578-628.
Порядок: initial rarity gate → special path или typed kind allocation →
prep → magic → stack. Неудачная special attempt включает good и lev+10
только для kind allocation; ordinary prep/magic используют исходный lev.
Book retries неприменимы: функция явно ограничена боеприпасами.

Переиспользованы source-backed prep/magic/stack primitives, без вызова
monster-drop/acquirement orchestrators. Special chooser получил optional
player_depth; новый путь передаёт реальную глубину игрока, старые вызовы
сохраняют прежнее поведение. Предмет возвращается detached, временная
регистрация для magic очищается в finally; существующие объекты не теряются.

ВАЖНАЯ ГРАНИЦА: это make_object primitive, а не place_object. Origin,
origin_depth, floor_carry, value/OOD/rating и установка в комнату здесь
не выполняются. Общая room object factory и whitelist ещё не заменены.
Native executable parity не заявляется; используются существующие численные
primitives модели с их документированными границами.

Четыре новые regressions: order/good-upgrade, детерминированные catalog-backed
ammo всех трёх типов, no-kind/adapter failure cleanup и special player-depth
town guard. Focused tests прошли (0.672 s), затем все 64 выбранных
ammo/magic/drop/stack/one_room regressions прошли (5.079 s). Отдельный launcher
one_room won, 8/8 фаз; git diff --check чистый. Full suite, native binaries
и generated-floor сценарии не запускались.

Следующий этап после подтверждения: place_object для typed ammo с origin,
empty-floor eligibility и floor carry/отказом; value/rating считать по
подтверждённым данным либо оставить явным обязательным adapter. Затем
подключить [ и проверить отдельный Ammo dump - arrows без полного этажа.

## Исправлен RNG расчёта пачек — 2026-10-17

Этап завершён. MCP подтвердил obj-make.c:1200-1209 и z-rand.c:484-513:
после успешного non-artifact quantity gate нужен полный randcalc, включая
m_bonus при нулевом бонусе. В object_data.py roll_object_stack добавлен
roll_native_m_bonus(0,0) после dice/constant, до max-stack clamp. Это не
игровой level fallback: загруженные pile metadata поддерживают только
constant/NdS, для нулевого m_bonus значение level не влияет на результат
или draws. Полная поддержка pile expressions с ненулевым M не заявляется.

Две новые regressions воспроизвели ошибку до правки. После исправления
Arrow quantity и полное RNG state совпадают с source-ordered primitive
sequence на seeds 1,7,19; проверены artifact без draws, failed gate с одним
randint1(100), dice/constant и clamp после обоих m_bonus draws.
Проверка Python sequence не объявляется native executable differential.

Семь focused tests прошли (0.485 s), все 48 выбранных drop/stack regressions
прошли (3.724 s). Launcher one_room won, 8/8 фаз; git diff --check чистый.
Full suite и generated-floor сценарии не запускались.
Следующий отдельный этап после подтверждения: typed-arrow object allocation
primitive по gen-util.c:498-539 и obj-make.c:1162-1233, включая special gate,
prep/magic, quantity, origin и floor carry. После этого — смешанный шаблон
Ammo dump - arrows; текущая упрощённая room factory ещё не заменена.

## Исправлен разбор NdS без base — 2026-10-17

Этап завершён. Через MCP прочитан `src/tests/z-dice/dice.c:17-61`.
В magic_data.py regex теперь считает ведущий integer базой только перед
+, M или концом выражения; перед d это количество dice. 7d7 -> (0,7,7,0),
2d3M4 -> (0,2,3,4). Перенос касается numeric subset; поддержка переменных
и внутренних пробелов не заявляется.

Добавлены две regressions: 16 numeric forms по native tests/Arrow и invalid
inputs; source-ordered 7d7 roll с полным RNG state и MIN/MAX/AVERAGE без
RNG draws. До исправления воспроизведены четыре неверных parse и неверный
roll (8 вместо 26 на seed7); после исправления все проходят.

Проверки: восемь focused tests прошли (0.425 s), отдельно все 16 magic tests
прошли (0.896 s). Разобраны реальные 35 trap visibility и 99 object pile
expressions; NdS сопоставлены с явными числом dice/sides. Launcher one_room:
won, 8/8 фаз. git diff --check чистый. Full suite, native executable
и generated-floor сценарии не запускались.

Следующий этап после подтверждения: исправить stack randcalc RNG. Текущий
roll_object_stack ещё пропускает m_bonus(0,level) draws после pile damroll;
этот дефект parser fix не исправляет. Далее typed-arrow primitive и
смешанный Ammo dump - arrows, согласно mixed_room_content_audit.md.

## Классификация смешанных native rooms — 2026-10-17

Этап исследования завершён. Полный список 66 шаблонов с ^ сохранён в
`angband/plans/room_content_catalog.json`, аудит и источники —
`angband/plans/mixed_room_content_audit.md` (пути от workspace).
Категории: [=12, 8=24, 89=24, 9[=4, 8[=2.
Первый кандидат — Ammo dump - arrows: 7x11, 3 typed object sites, 5 trap
sites, без guards/stairs; FEW_ENTRANCES остаётся отдельной geometry границей.

MCP подтвердил source order place_object/make_object, special rarity gate,
prep/magic/quantity/floor carry и второй проход. Текущая room object factory
упрощена и не пригодна для точного inline подключения без расширения.
Найдены prerequisites: parser NdS без base ошибочен (7d7 -> base7+1d7);
stack helper пропускает два simulate_division RNG вызова randcalc/m_bonus.
На Arrow seed7 количество совпадает (26), полные RNG states различаются.
Это воспроизведение Python primitives, не native executable differential.

Исходники/модель на этом этапе не менялись. Существующие четыре focused
regressions прошли (0.753 s), one_room won, 8/8 фаз; git diff --check чистый.
Full suite и generated-floor сценарии не запускались. Старый parser test
не покрывает найденную ошибку: проверяет base+NdS и dS, но не NdS без base.
Следующий отдельный этап после подтверждения: исправить NdS parsing по
native z-dice tests, затем stack RNG, затем typed-arrow allocation и
интеграцию реального шаблона. Не расширять whitelist до этих проверок.

## Безопасное подключение inline traps к room builder — 2026-10-17

Этап завершён. `build_room_template_profile_attempt_native()` принимает
явный `inline_trap_context`. После штатного единственного выбора шаблона
callback factory проверяет весь набор символов: только terrain/doors и ^
могут включить trap adapter; 8, 9, [, неизвестные символы сохраняют deferred
ветку без дополнительных RNG draws. При конфликте explicit action_callback
и context либо разных persistence inputs вызов отклоняется.

Добавлен preflight всей преобразованной площади после geometry/space selection
и до content allocation: выход за границы/пересечение не оставляют ловушек.
При успехе terrain устанавливается один раз, layout сообщает inline_traps;
последующая marker materialization не создаёт их повторно.
Через MCP проверен gen-room.c:1130-1187 (порядок geometry и first pass).

Три новых regressions: успешный inline builder, неизменные layout/RNG для
смешанных/неизвестных шаблонов, bounds/overlap refusal до trap callback.
Всего 13 targeted tests прошли (1.684 s); отдельный launcher one_room won,
8/8 фаз; git diff --check чистый. Full suite и generated-floor сценарии
не запускались.

ВАЖНО: загрузка реального каталога выявила 0 trap/terrain-only templates,
проходящих текущую консервативную классификацию. Поэтому новая возможность
проверена fixtures, но не добавляет ловушки реальным generated-floor runs.
Автоматический scheduler по-прежнему не включает context. Не расширять
whitelist и не создавать синтетические native templates ради включения.

Следующий этап после подтверждения: классифицировать реальные шаблоны с ^,
выбрать минимальный смешанный шаблон и исследовать native object/guard
allocation для его символов. После этого подключать общий content adapter
с сохранением первого/второго прохода и обязательным native RNG evidence.

## Inline trap adapter — 2026-10-17

Этап завершён: реализованы `place_player_trap_native()` (random-index ветка
place_trap) и opt-in `room_template_trap_adapter_native()`.
Через MCP проверены trap.c:255-269,275-344,356-393,
cave-square.c:92-95,220-223,490-493 и init.c:1591-1605.

Adapter получает chunk depth, player depth, quest, persistence и arena явно;
trap level и terrain текущей клетки приходят из first-pass context. Weighted
chooser теперь допускает отдельные chunk_depth/player_depth, сохраняя
совместимость существующих вызовов. Порядок: occupancy/terrain → town и
weighted selection → arena DOWN rejection → visibility/randcalc power →
вставка trap в голову списка. UI redraw остаётся вне численной модели.

Новый callback работает для изолированных trap/terrain templates, не создаёт
deferred trap markers и явно отказывает на других allocating actions.
Scheduler generated-room по умолчанию ещё НЕ подключает его: обычные
шаблоны с предметами/guards требуют общей orchestration. Late ^ без allocator
сохраняет честный pending. Это не заявление о полной native generation parity.

Четыре новых regressions проверяют inline RNG на соседних ^^, отсутствие
повторной materialization, occupied/trap-marker/wall/town отказ без RNG,
разделение трёх depth inputs и arena rejection между selection и power.
Constant visibility=3 используется только как явная test fixture.
Всего 11 targeted tests прошли (0.940 s), штатный one_room won, 8/8 фаз;
git diff --check чистый. Full suite, native executables и generated-floor
сценарии не запускались.

Следующий этап после подтверждения: безопасное подключение adapter к room
builder для заранее проверенных trap/terrain-only templates. Для смешанных
шаблонов сначала определить полный content callback contract и исходный
порядок предметов/guards; не подавлять unsupported actions и не возвращать
частично заселённую комнату как успешно полную.

## Исправлен RNG символа 8 при persistent=true — 2026-10-17

Этап завершён. Через работающий MCP `read_source` повторно проверен
`src/gen-room.c:1207-1218`: randint0(100) выполняется до проверки dun->persist.
В `angband_model/generation_data.py` исправлены callback и deferred ветки:
условие теперь `rng.randint0(100) < 80 or persistent`.

Добавлен `test_room_template_eight_draws_before_persistence_and_next_cell`:
8 комбинаций persistent on/off, roll 79/80, inline/deferred; проверяются
выбранная ветка и полный порядок RNG до следующей клетки ^ и второго прохода.
Тест сначала воспроизвёл четыре падения на старом коде; после исправления
проходит. Inline allocator в fixture делает явно синтетический draw для
проверки interleaving, не выдаваемый за native object allocation.

Семь focused regressions прошли (0.419 s), включая соседние layout/door/trap
ветки и отчёты victory/turn limit/player death. Отдельный launcher one_room:
won, 8/8 фаз. Full suite, native differential binaries и generated-floor
сценарии не запускались. git diff --check чистый.

Следующий отдельный этап после подтверждения: контракт inline trap adapter
и передача актуального terrain/occupancy, chunk/player/trap depth, quest,
arena и persistence; затем подключение ^ в первом проходе без позднего
сдвига RNG и без двойной materialization. Полная генерация уровней ещё
не объявляется соответствующей оригиналу.

## Angband Source MCP доступен — 2026-10-17

После предоставления полного доступа проверены локальная система и GitHub:
старая реализация отсутствует. Создан replacement server 0.2.0 на уже
объявленной optional dependency zeromcp; установлен в `.venv_f`.
Код: `SandboxCore2026/mcp/servers/angband_source/` от корня workspace.
Пользовательский `angband-source.service` установлен, enabled, работает на
http://127.0.0.1:8000/mcp; Linger=yes уже был включён. Сервис read-only,
Restart=on-failure. Подробности — README сервера и deploy/systemd.

Проверены initialize, tools/list, все шесть tools, обе актуальные project
roots и запрет path traversal/скрытых файлов. Выполнен реальный MCP search:
`src/gen-room.c:1210` подтверждает бросок до `dun->persist`.
`trace_rule` — literal source references, не semantic call graph;
`compare_rule` и coverage собирают evidence, не доказывают parity.
Для дальнейших запросов: из SandboxCore2026 запускать
`python3 mcp/servers/angband_source/client.py TOOL 'JSON_ARGUMENTS'`.
Предыдущее ограничение недоступного MCP снято; модель не менялась на этом
этапе. Следующий этап модели остаётся persistent RNG draw для символа 8.

## Аудит first-pass orchestration — 2026-10-17

Этап исследования завершён; результаты и следующий порядок реализации:
`angband/plans/room_generation_audit.md` от корня workspace.
Inline callback уже существует, но scheduler room template его не передаёт.
Выявлено отдельное расхождение: для символа 8 модель пропускает randint0(100)
при persistent=true (обе ветки), тогда как C выполняет draw до persist.
Правила не изменены: обязательный по локальному AGENTS.md angband-source MCP
недоступен, каталог его реализации отсутствует, localhost socket запрещён
средой. Для исправления требуется восстановленный MCP либо явное разрешение
пользователя проверять правила локальными C-исходниками вместо MCP.
Четыре focused regression прошли; штатный one_room won, 8/8 фаз.
Следующий ограниченный этап: persistent RNG draw с точной regression для
callback/deferred путей; затем отдельная интеграция inline trap adapter.

## Ветка смерти игрока — 2026-10-17

Этап завершён: добавлен
`test_one_room_player_death_finishes_report_without_kill_rewards`.
Короткая one_room fixture оборачивает world callback первого scheduler шага
и передаёт явный тестовый летальный урон существующему damage handler.
Это проверка отчётности, не новая игровая опасность и не доказательство
native parity. Рабочие сценарии и игровые формулы не изменены.

Проверено: lost/player_death во всех представлениях результата; совпадение
HP и alive в turns/snapshot/floor result; живой противник остаётся;
XP и награда за убийство не начисляются; player_died появляется один раз;
inventory и индекс RNG согласованы; отчёт заканчивает все фазы.
Семь focused scenario/report regressions прошли (0.492 s), ещё две
численные regressions урона и остановки blows после смерти прошли (0.131 s).
Штатный launcher one_room: won, 8/8 фаз, HP 19. Full suite и generated-floor
сценарии не запускались. git diff --check чистый.

Следующий этап после подтверждения: аудит first-pass room generation
orchestration и точек inline content allocation без подключения отложенных
trap markers постфактум. Перед изменением source-backed правил восстановить
предусмотренный локальным AGENTS.md angband-source MCP; если он недоступен,
сначала установить возможность восстановления, не подменять native RNG
порядок предположениями.

## Согласованность фаз one_room — 2026-10-17

Этап завершён. Добавлен regression
`test_one_room_phase_outputs_agree_for_victory_and_turn_limit`: короткая
fixture запускается с обычным лимитом и с лимитом в один шаг. Проверяются
порядок фаз, состав population, счётчики шагов, глубина, HP/состояние игрока,
остаток монстров, итог боя, арифметика XP, уровень, inventory и индекс RNG
между phase report, floor result и snapshot. Проверка не задаёт новых игровых
параметров и не утверждает native parity.

При победе противник удалён и XP начислен; при turn limit противник сохранён,
XP не начислен, outcome=incomplete. Завершённость отчёта не означает победу.
Шесть focused regression прошли (0.420 s). Отдельный штатный launcher:
one_room, seed 7, won, 8/8 фаз, 32 шага, +1 XP, HP 19. Full suite,
native differential executables и generated-floor сценарии не запускались.
Игровые правила менять не потребовалось.

Следующий этап после подтверждения: короткая проверка неуспешного исхода
(player death) на явной fixture, чтобы закрыть оставшуюся ветку отчёта, затем
вернуться к first-pass generation orchestration. Для изменения source-backed
правил по-прежнему требуется восстановить angband-source MCP согласно
локальному AGENTS.md.

## Возобновление в текущем workspace — 2026-10-17

- Найден и сохранён действующий план: этот файл; новый аудит не подменяет
  уже выполненные этапы. Последняя структурная задача — короткая проверка
  связей `population → turns → progression → snapshot`.
- Устранена блокировка после переноса проекта: общий `angband_model/paths.py`
  определяет соседний `angband/` относительно файла модуля, независимо от cwd.
  MVP, architecture smoke и оба тестовых модуля используют этот путь; native
  differential probes также ищутся в текущей папке игры. Команда в MVP.md
  исправлена для корня workspace. Игровые правила не изменены.
- Проверено: четыре focused regression (one_room fixture, переход со stubbed
  генератором, статусы фаз, структурный детерминированный snapshot) прошли.
  Отдельный запуск launcher: `one_room`, seed 7, won, 8/8 фаз, 32 шага боя,
  +1 XP, HP 19, pickup отражён в inventory. Полный suite, native probes и
  generated-floor сценарии на этом этапе не запускались.
- Следующий этап после подтверждения: дополнить короткую fixture-проверку
  согласованности population, результата боя, XP и итогового snapshot.
  Inline trap orchestration, полная native population и persistent chunk
  adapter остаются отдельными незавершёнными задачами. MCP angband-source
  в доступных инструментах отсутствует; указанный в локальном AGENTS.md
  каталог сервера также отсутствует. Перед изменением source-backed правил
  нужно восстановить этот канал исследования.


## Каркас отчёта фаз — 2026-10-17

Добавлена быстрая регрессия отчёта `SimulationRun`: проверяются поля planned,
recorded, completed, skipped и pending для частичного и полного прохода.
Статус `adapter` считается записанной фазой, но не числится как завершённая
или пропущенная; `complete` отражает наличие всех запланированных записей.
Это контракт каркаса запуска, не утверждение об игровой механике Angband.
Проверены только focused test и обычный `one_room`: оба прошли; в сценарии
все 8 фаз завершились, исход `won`, 32 хода. Полный suite и многоэтажные
сценарии пропущены.

## Новый приоритет: сначала рабочий MVP

По решению пользователя приоритет изменён: прежде чем расширять точность всех
подсистем, держим простой end-to-end сценарий, который запускается и
демонстрирует персонажа, команду, бой, смерть монстра, опыт и snapshot.
Реализован `projects/angband_model/angband_model/mvp.py`; команда и границы
этого среза описаны в `projects/angband_model/MVP.md`. Следующая работа должна
расширять этот рабочий срез небольшими этапами: AI/ответные атаки, затем
генерация уровня и drops. Большой точный план ниже остаётся картой дальнейшего
расширения, а не требованием завершить всё до запуска MVP. MVP уже расширен:
он рассчитывает native HP персонажа 1-го уровня и подключает противника с
native атакой через scheduler (противник атакует, только если уже соседствует
с игроком).

## Точка продолжения сквозного MVP — 2026-10-16

Сквозной запуск расширен до последовательности этажей: уровень создаётся
native room scheduler, сценарий заселяет тестового противника, выполняет ход
и бой, показывает исход и progression, находит путь к лестнице через обычные
закрытые двери, запрашивает native переход и продолжает с тем же персонажем.
`--levels N` повторяет этот цикл; `--report PATH` сохраняет JSON с фазами,
этажами и итоговым snapshot. Переходные решения о forced descent, stair skip,
quest stop и максимальной глубине остаются в runtime. JSON progression
отражает XP и уровень, которые runtime обновляет по `player_exp_gain()` и
`adjust_level()` (`src/mon-util.c:1105`, `src/player.c:208-284`).

После убийства подключены native death-drop roll, object materialization и
`drop_near()` placement. Для нового персонажа policy `ignore_item_ok()` берёт
source-confirmed reset state: `player-birth.c:1238` вызывает
`ignore_birth_init()`, которая сбрасывает kind, quality и ego ignore
настройки (`obj-ignore.c:143-160`). `MVP` показывает оставшиеся floor objects
в snapshot; регрессия `small kobold` покрывает nonzero `DROP_*` путь.
Пользовательские ignore preferences пока не задаются сценарием. Обновление
17.10.2026: native pickup уже реализован в runtime (`make_pickup_command`,
`execute_pickup_object_command`, `pick_up_player_object`) и проходит через
`process_player_command`/scheduler; inventory ownership обновляется, а
`reset_generated_level_state()` сохраняет carried objects при handoff. Добавлена
численная регрессия scheduler-команды: предмет с клетки игрока попадает в
inventory и тратит `move_energy / 10`, как `do_cmd_pickup()` в
`src/cmd-pickup.c:449-465`. Сквозной demo теперь получает явный catalog-backed
Ration of Food через editable fixture и подбирает её по отдельной policy
`scenario_objects_underfoot`; это не объявляется игровым выпадением. Population,
turns и snapshot показывают policy, pickup event и итоговый inventory. Проверки:
`one_room` — victory, все 8 фаз, pickup event один раз; focused scheduler,
scenario и двухэтажные regressions проходят. Persistent floor revisit и
town/arena chunk adapter остаются позже.

Проверено на seed 7: одиночный бой и два последовательных этажа завершились
победой сценария; на каждом этаже мышь дала 1 XP, уровень остался 1. Полный
Python suite: 499 тестов; `git diff --check` чистый.

## Проверка следующего шага — 2026-10-15

Исследован переход к погоне и выбору цели через существующий profile AI.
Полный callback выявил несоответствие имён полей в reproduction runtime
adapter. Исправлены обращения к `attempt_passed`, `learn_multiply_flag` и
`request_spawn`; добавлен regression test на применение флага lore. MVP
оставлен на проверенном двустороннем столкновении. В следующем шаге MVP
расширен: мышь появляется в двух клетках от игрока; каждый её scheduler ход
проходит через reproduction gate, native stagger/direction и обработку
занятости клетки. При seed 7 она делает два перемещения, затем бой заканчивает
игрок. Выбор цели фиксирован на единственного игрока и visibility задаётся
открытой тестовой картой.

## Продолжение MVP — 2026-10-16

Погоня теперь ограничена native видимостью: `native_los_between()` проверяет
луч по terrain карты, затем `native_monster_can_see_player()` применяет
COVERTRACKS и native max sight (`src/mon-move.c:90-101`; LOS —
`src/cave-view.c:85-250`). Если игрок не виден, прямое преследование
пропускается, поскольку sound/scent flow ещё не подключены. На тестовой карте
видимость упрощённо адаптируется геометрическим LOS: освещение, слепота и
невидимость не моделируются. При seed 7 результат остаётся: 2 перемещения,
одна атака монстра, затем убийство и опыт.

## Минимальное пробуждение — 2026-10-16

Sleeping spawn теперь записывает начальный SLEEP в timed state; runtime
поддерживает зеркальное поле `data['sleep']` при обработке таймера. Добавлен
native `make_noise()` flood по восьми соседним клеткам с NOFLOW, границами,
noise increment и пределом max hearing (`src/game-world.c:1144-1234`). MVP
начинает со спящего giant white mouse, вычисляет активность по LOS/hearing и
вызывает native sleep notice/reduction через timed boundary. При seed 7 мышь
просыпается, перемещается пять раз, атакует один раз; персонаж убивает её и
получает опыт.

## Погоня по шуму и запаху — 2026-10-16

Добавлена `native_update_scent_grid()` по `update_scent()` из
`src/game-world.c:385-454`: старение, шаблон scent strength, распространение
только от соседней более свежей клетки, NO_SCENT и COVERTRACKS. MVP обновляет
шум и запах в callback мировой фазы и при отсутствии видимости выбирает ход
через существующий `resolve_native_monster_advance_flow()` в native порядке
`ddgrid_ddd` (`src/cave.c:72-75`, `src/mon-move.c`). Между игроком и мышью
поставлена гранитная стена. Seed 7 проходит за 130 turns: мышь просыпается,
делает sound-flow шаг, всего перемещается 8 раз, один раз атакует; персонаж
выживает с 18/19 HP и получает 1 XP. Кандидаты sound flow теперь учитывают
native `monster_can_kill()`/`monster_can_move()` (включая UNIQUE, KILL_BODY,
MOVE_BODY и сравнение mexp) и `monster_hates_grid()`; `TerrainFeature` загружает
`resist-flag` (lava: `IM_FIRE`) из `lib/gamedata/terrain.txt`. В этой сцене
нет соседних конкурирующих монстров и лавы, поэтому body collision проверена
предикатами, а не полным переездом в сценарии. Урон от лавы подключён и
проверен отдельным runtime тестом ниже.

## Урон монстра от лавы — 2026-10-16

Добавлен `resolve_native_monster_terrain_damage()`. `monster_take_terrain_damage()`
вызывается после хода монстра (`src/mon-move.c:1972-1973`); для `FIERY`
terrain без расы `resist-flag` native код пробуждает монстра и наносит
`100 + randint1(100)` (`src/mon-util.c:1327-1343`). `monster_wake()` снимает
SLEEP и потребляет `randint0(100)` даже при awareness chance 0
(`src/mon-util.c:686-693`), этот порядок отражён перед броском урона.
UNIQUE и arena монстры не умирают от nonplayer damage; HP ограничиваются
нулём (`src/mon-util.c:1203-1217`; `damage.py:apply_monster_damage`).
Проверка покрывает обычную мышь на лаве и невосприимчивого саламандра без
лишнего RNG. MVP вызывает hazard check после хода живого монстра и включает
terrain damage в условие активности; на текущей карте результат seed 7 не
изменился. Полный suite после этого этапа: 454 теста.

## Перенос held objects при смерти от лавы — 2026-10-16

Проверено, что `mon_take_nonplayer_hit()` вызывает `monster_death()` при смерти
(`src/mon-util.c:1217-1226`), а `monster_death()` снимает held objects с
монстра и вызывает `drop_near()` для каждого (`src/mon-util.c:949-998`).
Модельный `_remove_active_monster()` уже выполняет
`materialize_monster_carried_objects_on_death()` до удаления монстра, поэтому
добавлена интеграционная регрессия: при смерти мыши от лавы её held Arrow
остаётся на клетке смерти. Suite: 454 теста. Это закрывает перенос уже
имеющихся carried items; создание race treasure происходит при native
размещении монстра (`src/mon-make.c:1046`).

## Spawn drop gate для MVP — 2026-10-16

`place_monster_race()` получил необязательный `origin`. При native
`ORIGIN_DROP` race с пустым списком drop rules и без DROP_* / QUESTOR флагов
проходит точный нулевой путь: `mon_create_drop_count()` возвращает 0 без RNG,
object generation loop не выполняется (`src/mon-make.c:699-748, 775-803`).
Giant white mouse подтверждена в `lib/gamedata/monster.txt:627-643`: у неё
только MULTIPLY и RAND_50. MVP теперь задаёт этому монстру ORIGIN_DROP и
показывает нулевой результат без изменения spawn RNG. Если race имеет явные
drop rules или drop flags, runtime заранее отклоняет spawn с
`NotImplementedError` до RNG/state mutations, пока object allocation/carry не
подключены. Следующий этап: поддержать race treasure allocation и удержание
объектов на монстре; точный floor `drop_near` остаётся отдельным adapter.
Полный suite: 455 тестов.

## Обновление продолжения — 2026-10-14

Добавлен JSON export всех занятых native monster/object slots после persistent
restore, который differential собирает в payload и передаёт через
`install_generated_level_entities()`. Для теста один объект получает
искусственно созданную парную запись в known chunk; native restore должен
назначить её в `object->known` по тому же slot, и это проверяется как
ненулевая pointer association.

Monster export включает slot/race, координаты, HP/maxHP, speed/energy,
именованные timed effects, primary group/role, mflags и held/mimicked object
indices. Object export включает slot/kind index/name, координаты, tval/sval,
количество, pval/вес/боевые значения/timeout, held/mimic indices, origin,
artifact/ego, notice, object flags, modifiers, elements и базовые значения
known copy. Полный payload принят моделью с группами, scheduler actors,
world-map grids и known-object map. Источники: `src/monster.h:385-422`,
`src/object.h:422-471`, `src/generate.c:1427-1437`, `src/tests/game/basic.c`.

Дополнительно экспортированы object side tables: активные brands и slays по
именам исходных таблиц, curse power и отдельно curse timeout. Differential
передаёт их через `native_object_properties` и проверяет сохранность в
установленном объекте. Это перенос состояния, а не новое вычисление атак или
эффектов. Источники: `src/object.h:83-122,391-394,449-456`,
`src/tests/game/basic.c:88-134`; размеры циклов берутся из
`z_info->brand_max/slay_max/curse_max`.

Persistent monster records расширены native target grid/index, min/best range,
mflags, remembered player object/player flags и element resistances, а также
heatmap (null либо матрица уровня). Differential несёт исходную запись в
`native_monster_state` и проверяет точное сохранение в установленном monster
state. Эти поля численно используются: target и ranges — в `mon-move.c`,
known player state — в `mon-attack.c` и `mon-util.c`. Источники:
`src/monster.h:385-422`, `src/player.h:401-438`, `src/mon-move.c:224-299`,
`src/mon-attack.c:200-217`, `src/mon-util.c:800-834`,
`src/tests/game/basic.c`. Snapshot сохраняет данные, не добавляя отдельную
реализацию AI к handoff.

Object records now also include effect descriptors (index/name/parameters,
message and whether a dice object exists), activation name, raw recharge
random_value tuple, origin race, inscription, and expanded known-copy state.
The C fixture evaluates native `object_runes_known()`,
`object_effect_is_known()` and `object_fully_known()`; differential compares
these with model predicates and checks pointer-identity effect knowledge.
Model rune comparison now accepts native bitflag words and indexed property
arrays, including null-vs-allocated side-table distinctions. Sources:
`src/object.h:422-473`, `src/obj-knowledge.c:540-545,678-762,820-857`,
`src/obj-make.c:833`, `src/obj-init.c:1164-1288`,
`src/tests/game/basic.c`.

Still unknown/unmapped: the symbolic dice expression behind C's opaque
`dice_t` is not exported by the object descriptor; it must resolve through
the matching source catalog (kind or curse) before model calculations use it.
Known cave is retained as an opaque snapshot, not a
numerically simulated chunk; raw monster target/heatmap/known-player state
within that cave and group member pointer lists remain unexported. Checks:
game/basic — 7/7, differential — 32, full suite — 449 tests.

Актуализация fixture: тестовый native объект получает ненулевые side tables
из существующих `e_info` и `a_info`. Differential сопоставляет brand/slay
имена с ego каталогом, curse power — с artifact каталогом; timeout пустой.
Это проверяет экспорт и mapping таблиц, но не утверждает, что такая комбинация
создана обычной генерацией. Исправлен `magic_data._records()`: comment-only
строка внутри record больше его не обрывает. Теперь загружаются curse Calris
(`artifact.txt:562`) и все значения Doomcaller (`artifact.txt:687`), что
проверяется тестами. game/basic — 7/7, differential — 32/32, полный suite —
449/449.

Effect dice handoff: native fixture exports every ordinary object kind with
an effect chain. Differential matches 202 kinds and 424 effect nodes against
the source catalog, comparing order, names, x/y, radius/other, dice presence
and the null per-effect message field. It also verifies subtype indices for
projection names via `list-elements.h` + `list-projections.h`, timed effects
via `list-player-timed.h`, monster timed effects via `list-mon-timed.h`, and
nourishment, summon, stat, shape, enchant, glyph and earthquake values against
their source order/constants. The catalog retains exact dice expressions
(including `2d8` for scroll `Light`). Native descriptors do not serialize
`dice_t` text, so expressions resolve by source record. The ordinary object
catalog has no `MON_TIMED_INC`, `TELEPORT:AWAY` or `TELEPORT_TO:SELF` entries;
those `effect_subtype()` branches remain without object-kind differential
examples. Three controlled `TV_SCROLL/Light` actual-known pairs now check C and
model predicates for fully matching data, effect-pointer mismatch, and STR
modifier mismatch; `runes_known`, effect identity and `fully_known` agree in
all cases.

Compatibility bug found in native `sanitize_player_loc()`: after 1000 failed
random samples, its row-major wrap scan starts at random `ix/iy`, but the
initial range includes `width-1`/`height-1` while the scan wrap excludes those
coordinates. If either anchor coordinate is on that final boundary, the loop's
`tx == ix && ty == iy` stop condition is unreachable and native restore hangs.
The fixture selects/replays a seed with an interior scan anchor so parity
checks stay finite. This original bug is not folded into the model's ordinary
finite fallback; hang emulation belongs in a separate compatibility mode.
Source: `src/generate.c:1301-1335`. Full suite: 449/449.

Исправлен parser `object_data.load_object_kinds()`: comment-only строки
пропускаются и не заканчивают запись. Регрессия проверяет `CURSE_WEAPON` и
`CURSE_ARMOR`, которые раньше терялись после `#A` комментария. Проверки:
game/basic 7/7, differential 32/32, полный suite 449/449.

Порядок native restore подтверждён: назначаются actual и known chunks,
known objects связываются совпадающими object slots, затем выполняется
`restore_monsters()`, стареет scent и очищается noise. Сохранение `keep_all`
держит исходные chunk pointers; точная симуляция полного known cave и ловушек
остаётся обязанностью engine adapter. Источники: `src/generate.c:1017-1033,
1414-1449`.

Нативный fixture теперь экспортирует known cave snapshot: размеры/depth/turn,
chunk feeling/rating/decoy/счётчики, feature counts, scent/noise, все клетки
в row-major порядке (native feature index, square light и все слова
square-info), занятые known object slots, базовые записи монстров и групп,
stair connectors, а также ловушки с индексом/именем, power, timeout и flags.
Differential сохраняет экспорт как opaque payload через
`store_persistent_level_native()` и сверяет его после lookup. Это точный
round-trip представленного payload, но пока не полный raw `struct chunk`.

Незакрытые поля: содержимое group member lists и monster-held object pointer
graph в known cave; символические `dice_t` expressions требуют разрешения по
исходному kind/curse каталогу. `ChunkState` хранит memorized terrain и часть
признаков, но не восстанавливает raw pointer graph. Текущие VIEW/SEEN/WASSEEN/PROJECT flags
передаются в square-info словах; их игровая значимость зависит от конкретного
читателя. Источники: `src/cave.h:161-214`, `src/list-square-flags.h`,
`src/generate.c:1366-1367`, `src/monster.h:385-422`; высокая уверенность
по структурам и парному хранению, численная значимость transient полей
требует reader-level аудита. Проверки: game/basic 7/7, differential 32,
полный набор 448.

Native-тесты закрыли marker scan, первый допустимый квадрат в радиусном обходе
вокруг `cave->monsters[1].grid` и последний arena fallback через
`sanitize_player_loc()`. Для последнего случая все клетки в радиусах 1..9
закрыты, а seed направляет первый random sample в единственную свободную
клетку. Python differential совпадает по позиции, методу и полному RNG state.
Источник: `src/generate.c:1456-1508`, `src/tests/game/basic.c`.
Проверки: game/basic — 7/7, differential — 32, полный набор — 448.

Оставшееся ограничение arena-контекста: fixtures управляют состоянием
сохранённой карты напрямую, чтобы изолировать ветви; полноценная arena
генерация и связанные с ней UI/game-world события остаются engine adapter.

Расширено native-наблюдение persistent entity handoff: до возврата тест
сохраняет образец живого monster slot и floor object slot, после восстановления
проверяет race, grid, HP/maxHP, speed, energy и все timed effects монстра, а
также идентичность объекта и его известной копии в том же `oidx`. Это
подтверждает стабильность этих представленных полей и ассоциацию known object,
но не является полным экспортом всех полей сущностей. Исходники:
`src/generate.c:1427-1437`, `src/tests/game/basic.c`; Python differential
проверяет native observation. Проверки: game/basic — 7/7, differential — 32,
полная suite — 448 тестов.

Дополнены native known-copy случаи: actual содержит brand, slay или curse,
а соответствующая известная копия не содержит этого свойства. Шаблоны взяты
из загруженных `e_info` и `a_info`; native `object_runes_known()` для всех трёх
возвращает false, и модель совпадает с результатом. Источники правил:
`src/obj-knowledge.c:629-645,734-746`, `src/obj-curse.c:77-91`;
fixture и проверка: `src/tests/game/basic.c`,
`projects/angband_model/tests/test_differential.py`. Проверки: game/basic —
7/7, differential — 32, полная suite — 449 тестов.

Расширена та же проверка положительными source-created копиями: совпадающие
brand, slay и curse дают `runes_known=true`. Добавлен случай, где у actual
curse изменён только timeout, а curse power совпадает; native считает руны
известными, так как `curses_are_equal()` сравнивает только `power`. Python
совпадает с этим правилом. Источники: `src/obj-knowledge.c:734-746`,
`src/obj-curse.c:77-91`; fixture — `src/tests/game/basic.c`, differential —
`projects/angband_model/tests/test_differential.py`. Проверки: game/basic —
7/7, differential — 32, полная suite — 449 тестов.

Дополнительно проверены напрямую в native `effect_subtype()` три значения,
которых нет в ordinary object catalog: `MON_TIMED_INC:SLEEP`,
`TELEPORT:AWAY`, `TELEPORT_TO:SELF`. C fixture вызывает `effect_lookup()` и
`effect_subtype()`; differential сверяет subtype индексы с исходным списком
monster timed и подтверждёнными константами `1` для двух teleport ветвей.
Источник: `src/effects.c:137-260`, fixture `src/tests/game/basic.c`, проверка
`projects/angband_model/tests/test_differential.py`. Проверки: game/basic —
7/7, differential — 32, полная suite — 449 тестов.

Следующий шаг: продолжить аудит оставшихся engine-driven правил. Полный raw
known cave, включая вложенные
pointer graphs, и полная arena генерация/UI/game-world события остаются engine
adapter работой.

Проверено направление object-flag subset. `flag_is_subset(flags1, flags2)` в
`src/z-bitflag.c:166-176` проверяет, что flags2 содержатся в flags1, поэтому
`of_is_subset(obj->known->flags, obj->flags)` допускает extra flags в known
copy, но требует наличия в ней каждого actual flag. Native fixtures покрывают
равные флаги и extra known `OF_FREE_ACT`; Python ранее проверял обратное
направление, исправлено в `runtime.py`. Источники: `src/obj-knowledge.c:724`,
`src/obj-properties.h:130`, `src/z-bitflag.c:166-176`; fixture/test:
`src/tests/game/basic.c`, `projects/angband_model/tests/test_differential.py`.
Проверки: game/basic — 7/7, differential — 32, полная suite — 449 тестов.

Закрыт conservatism gap для curse resistance в actual-rune derivation. После
завершённого `apply_selected_drop_curses()` результирующая resistance карта
уже отражает native curse/object комбинацию; ранее сам факт resistance values
у активной curse без причины возвращал неизвестное состояние. Regression на
загруженной curse `burning up` проверяет FIRE vulnerability, COLD resistance и
саму curse rune. Основание: `src/obj-curse.c:488-565`, `src/obj-knowledge.c:1743-1759`;
проверка: `tests/test_stage1.py`, вычисление: `angband_model/runtime.py`.
Добавлен native differential fixture: Arrow проходит через
`object_prep()`, `append_object_curse()` и `apply_curse_attributes()` в C;
Python строит тот же catalog-curse state через свои завершённые drop stages.
Сверяются curse power и все ненулевые element levels. Источники:
`src/obj-make.c:817-885`, `src/obj-curse.c:129-215,450-565`; fixture и
сравнение: `src/tests/game/basic.c`,
`projects/angband_model/tests/test_differential.py`. Native target — 7/7,
differential — 32, полная suite — 450 тестов; `git diff --check` прошёл.

Базовые свойства object base теперь распределяются по тем же native слоям,
что и в `parse_object_base_flags()`: object flags, kind flags и element
flags. Ранее `HATES_*`/`IGNORE_*` и `SHOW_DICE` оставались в `base_flags`, из-за
чего runtime мог выдавать их за обычные object flags. Arrow теперь имеет
`SHOW_DICE` в kind слое и `HATES_ACID/HATES_FIRE` в element слое, согласно
`src/obj-init.c:581-610`. Полный прогон после правки: 450 тестов.
Native differential fixture для curse merge теперь также экспортирует
`SHOW_DICE` и elemental `HATES_ACID/HATES_FIRE` из подготовленного C объекта;
сверка с Python подтверждает все три значения и их категории. Native
`game/basic` — 7/7; отдельная differential проверка прошла.
Базовый `object_prep()` слой также сверяется на Scroll of Light: native
`sval` сравнивается с модельным нулевым индексом + 1, сверяются вес, dice,
armor class и HATES flags для ACID/FIRE. Одноразрядность подтверждена
`src/obj-init.c:1748-1753`; differential проверка проходит.

Закрыта проверка трёх ветвей восстановления позиции при возврате на сохранённый
уровень: прямое сохранение клетки (без RNG), успешный выбор обычной безопасной
клетки первым случайным кандидатом и исчерпание 1000 попыток с обходом и
vault-fallback. Native fixture проходит через `prepare_next_level()`; Python
differential сверяет координату, метод и полное состояние WELL1024a после
операции. Источники: `src/generate.c:1290-1346,1460-1513`,
`src/cave-square.c:442-455,604-623`, `src/tests/game/basic.c`; тест:
`projects/angband_model/tests/test_differential.py`.

Следующий незакрытый участок этого направления: native точное восстановление
снимков монстров и объектов, а также отдельные ветви возврата из арены. Для
них нужно добавить native наблюдения и только затем расширять модельный
адаптер. Численные результаты текущего этапа: `game/basic` — 7/7,
`test_differential` — 32 теста.

## Точка продолжения — 2026-10-14

### Последнее завершённое продолжение

Закрыта числовая часть primary monster groups: индексы выделяются по первому
свободному слоту `1..<level_monster_max`, новые участники добавляются в начало
списка, а при удалении лидера старый слот занят на всём протяжении создания
новых групп по расам. Пустые/старые слоты освобождаются в native порядке.
Таблица групп включена в chunk snapshot с проверкой диапазона индексов при
восстановлении. Источники: `src/mon-group.c:82-145,152-240`,
`lib/gamedata/constants.txt:11`. Регрессии покрывают порядок split allocation и
snapshot round-trip. Проверки: `python -m unittest tests.test_stage1` — 405
тестов, `python -m compileall -q angband_model`, `git diff --check` — прошли.

В продолжение placement адаптеры теперь могут передавать вид ловушки через
`kind`, `trap_type` или `name`; модель разрешает значение по загруженным
`lib/gamedata/trap.txt` именам и кодам перед проверкой ward/decoy. Это закрывает
ложное разрешение занять glyph/decoy клетку при передаче native name. Источники:
`src/cave-square.c:751-760`, `lib/gamedata/trap.txt:61-72`. Этапная suite:
406 тестов; общий suite — 434 теста; scenario завершился с кодом 0.
`python -m compileall -q angband_model` и `git diff --check` прошли.

Продолжена передача состояния уровня: `commit_generated_level_change()` может
получить уже созданный `entities` payload и установить монстров, предметы,
ловушки, первичные группы, позицию игрока и scheduler slots в порядке адаптера.
Проверка данных выполняется до замены состояния; генерационные RNG rolls не
повторяются. При передаче точного `known_artifact` модель также сбрасывает
created-флаг неизвестного артефакта, поднятого старым монстром, как native
`wipe_mon_list()`; при отсутствии этого состояния handoff отклоняется без
изменений. Источник: `src/mon-make.c:562-617`, `src/obj-util.c:830-839`.
Cleanup теперь также учитывает артефакты на полу: в discarded-level ветке
`created` сохраняется при `birth_lose_arts` или уже известном артефакте и
сбрасывается для неизвестного артефакта при выключенном параметре. Для
persistent-level ветки cleanup пропускается. Источники:
`src/generate.c:1350-1398`, `src/obj-util.c:830-839`. После этих изменений
этапная suite — 410 тестов, полный suite — 438 тестов; scenario,
`compileall` и `git diff --check` прошли.

Persistent-level lookup и tracking/target граница сверены. Native
`generate.c` получает имя уровня как `level_by_depth(p->depth)->name`, ищет
сохранённый chunk через `chunk_find_name(name)` и отдельно ищет known chunk
`"%s known"`; таблица имени и глубины задаётся `world` и загружается в
`game-world.c`/`init.c`. Модель загружает эту таблицу через
`load_world_levels()` и отдаёт ключи через
`native_persistent_chunk_names(depth)`. `old_level_persisted` остаётся
решением engine adapter, который передаёт также уже восстановленные entities.
В discarded-level ветке runtime
выдаёт `generated_level_tracking_cleanup_requested`; адаптер применяет
`target_set_monster(NULL)` (с сохранением grid для fixed target) и
`health_track(NULL)`. Persistent restore не выдаёт cleanup request. Источники:
`src/generate.c:1414-1423`, `src/game-world.c:95-121`, `src/init.c:1097-1119`,
`src/target.c:150-170`, `src/mon-make.c:627-630`.

Persistent monster recovery теперь тоже перенесён в handoff. Если adapter
явно указывает, что destination chunk был восстановлен, и передаёт его native
`chunk->turn`, модель считает прошедшие ходы, HP regeneration (`num_turns / 100`)
и decay каждого timer по `turn_energy(monster speed) / move_energy` в reverse
slot order. `COMMAND` expiry требует очистить engine target reference;
`CHANGED` expiry требует engine shape revert — для них выдаются adapter actions.
Источники: `src/mon-move.c:1842-1869,2007-2031`, `src/mon-timed.c:289-316`,
`src/list-mon-timed.h`, `src/game-world.c:55-102`.

Persistent scent/noise grids также перенесены в `ChunkState` и snapshot.
Адаптер передаёт полные `uint16` карты; при restore scent стареет на разницу
`turn / 10`, переполняющиеся значения исчезают, interior noise очищается,
граничные клетки остаются без изменений. Источники: `src/game-world.c:1240-1280`,
`src/cave.c:353-361`.

Для пары основной/known карт добавлен optional complete `known_objects` handoff,
индексированный native `oidx`. Он связывает данные знания с соответствующим
восстановленным floor object по тому же индексу, что native цикл по параллельным
chunk object arrays; пропущенная известная запись означает отсутствие paired
known object. Источники: `src/generate.c:1429-1433`, `src/cave.c:448-470,507-524`.

В модели появился named persistent chunk store: `store_persistent_level_native()`
сохраняет primary/known пару под world names, включая turn, player position,
scheduler, map/entity snapshot, memorized terrain и known-object records. Полная
native known cave передаётся адаптером через optional `known_chunk_state`; без
него `engine_known_chunk_complete` остаётся false, поскольку список memorized
координат не восстанавливает точные feature/knowledge поля known chunk;
`lookup_persistent_level_native()` проверяет наличие пары, а
`remove_persistent_level_native()` удаляет обе записи после успешного возврата.
Store включён в runtime snapshot. API основан на `cave_store()`/`chunk_find_name()`;
`plan_native_level_transition()` теперь вычисляет подтверждённые ветви persist,
arena и Town, находит сохранённую целевую пару и возвращает список действий
адаптера. Он следует `persist = birth_levels_persist || target arena`, пропускает
исходный chunk `arena`, сохраняет Town primary-only при отключённом persist,
если Town ещё не сохранён, и помечает пару для удаления только после restore.
`commit_generated_level_change()` принимает этот переходный контекст. Если
целевая пара найдена, адаптер обязан указать её точные native names и передать
persistent-recovered entities; несовпадение отклоняется до изменения состояния.
Пара удаляется только после успешной установки entities и commit depth.
`prepare_native_level_transition()` теперь исполняет outgoing storage до
генерации: persistent branch получает подготовленный native runtime snapshot;
Town branch получает точный `chunk_write()` terrain/square-info payload.
Адаптер отвечает за подготовку snapshot после compaction monster slots и
обновления player marker; полная known cave передаётся через
`known_chunk_state`. Источники:
`src/generate.c:1017-1031,1349-1373,1414-1455`, `src/mon-make.c:482-542`,
`src/gen-chunk.c:46-68`.

Этап продолжается без новых числовых игровых допущений: transition storage
выполняется перед generation, lookup идёт после storage, а целевая пара удаляется
после успешного restore/commit. `resolve_native_persistent_player_position()`
переносит direct placement, RNG attempts, scan и arena-return priorities с
адаптерными square predicates; `commit_generated_level_change()` передаёт его
результат в entities до установки и восстанавливает RNG при validation reject.
Target/health политика подтверждена:
при compaction оба указателя следуют за тем же monster object; при удалении
уровня native очищает их, сохраняя fixed target grid. Модель выдаёт соответствующие
adapter instructions, но реальные native pointers остаются engine adapter state.
Добавлена C fixture `parity/native`, которая вызывает `monster_index_move()` и
проверяет identity `health_who` при переносе slot 2 → 1; Python differential
test запускает её и сопоставляет с handoff policy модели (native suite 5/5).
Следующая незакрытая часть — target pointer fixture и полный controlled runtime
parity для `prepare_next_level()`/persistent restore.


Проверено актуальное состояние проекта
`/home/izot/Documents/ANGBAND/SandboxCore2026/projects/angband_model`.
Подробные живые реестры — `implementation_status.md` и
`source_manifest.md` в каталоге целевой модели. Реализация существенно
расширилась относительно первичного аудита и включает runtime, state,
RNG, scheduler, движение, бой, магию, предметы, прогрессию, эффекты,
ловушки, генерационные записи и адаптерные интерфейсы. Не следует начинать
этапы первичного плана заново.

### Проверки текущего состояния

- Команда сценария `PYTHONPATH=src:projects/angband_model python -m
  angband_model.scenarios` завершилась с кодом 0. Seed 7, загрузка каталогов:
  375 object kinds и 91 monster spell; базовый birth-сценарий проходит один
  шаг и печатает snapshot.
- Исходно regression suite запускал 382 теста с 1 failure и 3 errors; все
  четыре ошибки исправлены после проверки native порядка и контракта layout.
- Текущая regression suite: `PYTHONPATH=src:projects/angband_model python -m
  unittest discover -s projects/angband_model/tests -q` — 413 тестов прошли.
- Scenario `PYTHONPATH=src:projects/angband_model python -m
  angband_model.scenarios` завершился с кодом 0; seed 7, 375 object kinds,
  91 monster spell records. `git diff --check` прошёл.
- Рабочее дерево оригинальной игры уже содержит посторонние локальные правки и
  новые Python bridge-файлы (см. `git status`); их не менять и не принимать за
  clean upstream baseline без отдельного сравнения.

### Выполненный этап и следующая работа

Исправлены ranged modifier вызовы без positional equipment layout: пустой
modifier source допустим только когда среди принадлежащих игроку предметов нет
экипированных. Если экипированные предметы есть, отсутствие упорядоченной
раскладки остаётся явной ошибкой адаптера. В world tail после trap timeout
обрабатываются recall/deep-descent таймеры по `src/game-world.c:677-750`.

Для activation candidate lists сверены нативные item testers:
`RECHARGE` выбирает только wand/staff; `ENCHANT` использует точные sets из
`tval_is_armor()`/`tval_is_weapon()`; chooser `BRAND_BOLTS` допускает любой bolt,
а magical/zero-cost отказ выполняет обработчик после выбора. Явные цели
ENCHANT/RECHARGE проверяются по native candidates. UI-выбор остаётся engine
adapter. IDENTIFY строит точный native rune catalog из C enum-ов и
загруженных brand/slay/curse данных; runtime канонизирует rune keys и выбирает
одну руну через Angband RNG `randint0(num)`. Глобальное знание игрока выводится
из модели. Actual руны принимаются через нормализованный вход
`native_rune_properties` с маркером полноты. Предикат `object_runes_known()`
вычисляется из полных actual/known snapshots по native-сравнениям:
combat/modifiers и curses требуют равенства, элементы проверяются по ненулевым
actual resistance, brands/slays — включением, а known flags должны содержать
все actual flags (дополнительные known flags допускаются). Actual руны теперь
можно вычислить из уже материализованных kind/ego/artifact/curse слоёв при
завершённых native stage маркерах. Активные curse resistance values уже
включаются в `magic_resistances` при curse application и вместе с самой curse
руной участвуют в IDENTIFY. После изучения руны IDENTIFY обновляет known snapshots у всех полностью известных и уже оценённых предметов через `native_refresh_assessed_object_knowledge()`; неоценённые предметы пропускаются, как в native `player_know_object()`. Focused-тест проверяет обновление нескольких modifiers, прекращение дальнейшего выбора после полного знания и отсутствие обновлений у неоценённого предмета. Native ignore settings для `drop_near`
остаются адаптерными, но модель принимает точный per-object
`native_ignore_item_ok` вместо callback; без результата она не выдумывает
политику игнорирования. Focused тесты, полный suite (409 тестов), scenario и
`git diff --check` прошли. REMOVE_CURSE
теперь требует явные target/curse choices, удаляет выбранную curse по strength,
пересобирает её object attributes и обрабатывает FRAGILE/one-in-four 5d5
разрушение с player damage, когда HP, ownership, item weight и curse base state
полны. Иначе ветка выдаёт typed adapter request.

Добавлена численная проверка native rune catalog order. Также есть проверки drop-grid equal-score tie (`one_in(2)`) и
artifact fallback первого random-walk шага с точным RNG состоянием. Локальный
search и fallback проверены; map geometry, ignore/pile policy и floor carry
остаются явными adapter входами.

Основные artifact, resistance, curse и flag merge-слои сверены с native источниками. Продолжена floor policy: `native_floor_objects_mergeable()` сравнивает итоговые curse powers по имени (с native kind/ego/artifact overwrite-порядком и более сильным duplicate append), а все resistance layers нормализуются к именам элементов и сравниваются как итоговый `el_info`. Закрыт charged throwing-stack carry в preferred quiver path: части делятся от текущего остатка зарядов/timeout, заряд попадает в соответствующую quiver/pack часть или совместимую pack-стопку, а остаток пола сохраняет нераспределённое значение. Источники: `src/obj-pile.c:779-809`, `src/obj-util.c:960-1005`; численная регрессия проверяет native округление: деление стопки 6 предметов с 10 зарядами на части 1, 1, 2 даёт 1, 1, 4 и оставляет 4; интеграционная проверка preferred throwing pickup подтверждает разделение Magic Missile wand (10 предметов/11 зарядов) между quiver (8/8) и pack (2/3). Focused stage suite: 378 тестов. В floor-drop commit теперь требуется native pile order, если имеется несколько compatible merge targets или ignored eviction candidates; без порядка ошибка явно оставляет этот выбор адаптеру, а однозначные случаи работают без него. Источник: `src/obj-pile.c:886-888,905-975`; добавлена проверка на неоднозначную/заданную раскладку. Focused stage suite: 379 тестов. Следующая работа — закрывать оставшиеся границы drop-near и генерацию/установку actors/objects, затем входы spell/equipment/visibility и differential fixtures.
После каждого локального этапа запускать focused численные проверки, полный
suite и scenario. В этом продолжении full suite (409 тестов), scenario и
`git diff --check` прошли. Также исправлен артефактный metadata merge: модель
больше не добавляет всем артефактам универсальные IGNORE_ACID/ELEC/FIRE/COLD/POIS;
переносятся только IGNORE_/HATES_ свойства, реально заданные самим артефактом.
Подтверждение: `src/obj-make.c:526-570`, где `copy_artifact_data()` объединяет
только `art->flags` и element flags. Добавлен тест для артефакта Angrist, у которого
нет собственных IGNORE_/HATES_ флагов. Также `copy_artifact_data()` заменяет все modifier slots (отсутствующие artifact values дают native zero), но заменяет resistance только при ненулевом уровне; `apply_selected_drop_values()` теперь отражает это различие. Тест проверяет сброс унаследованного STR при сохранении неназванного FIRE resistance. Материализованные actual-rune keys теперь включают template curses из merged kind/ego/artifact metadata вместе с динамическими active curses; native `copy_curses()` присваивает power в этом порядке. Добавлен тест для фиксированной curse у кольца Teleportation. В special/QUEST_ART пути также исправлено сохранение базовых resistance levels из `object_prep()` при копировании артефакта; ненулевые уровни артефакта затем перезаписывают базовые по `copy_artifact_data()`. Тест использует базовый Blade of Chaos (`RES_CHAOS=1`) и артефакт Doomcaller без собственных resistance values. Floor stack comparison теперь сводит фиксированные и динамические curses к итоговой power map и проверяет все resistance layers, канонизируя `RES_FIRE`/`FIRE` к native element index; раньше разный порядок curse записей ложно запрещал merge, а поле resistance в этом коде обращалось к несуществующему атрибуту и могло аварийно остановить сравнение. Artifact copy также удаляет унаследованные `TAKES_FUEL`/`BURNS_OUT` перед добавлением своих флагов и всегда сбрасывает timeout в 0. Runtime effective flags, metadata state и отдельный artifact-state helper повторяют этот порядок; добавлен тест с унаследованным топливным фонарём. Focused/full suite (409), scenario и
`git diff --check` прошли.

Изменения в модели сделаны только в целевом проекте; уже существовавшие
локальные изменения в SandboxCore сохранены.

Затем исправлен generated object placement: до создания объекта проверяются
границы карты, object-holding terrain, существующий предмет и ловушка. При
отказе RNG генерации предмета не расходуется, а маркер больше не переносится
на ближайший пол. Основание: `src/gen-util.c:498-525,540-558` и
`src/cave-square.c:212-230,626-630`. Добавлен RNG-state regression test;
focused suite (380 тестов), полный suite (409), scenario и `git diff --check`
прошли. Также сверены проверки установки generated monsters: после выбора
расы до spawn RNG проверяются PASSABLE terrain, позиция игрока, занятость
монстром, glyph of warding/decoy, доступность UNIQUE и FORCE_DEPTH. Источники:
`src/mon-make.c:1080-1112`, `src/cave-square.c:108-110,655-660,751-760`.
Добавлен тест пяти блокирующих условий; полный suite (409), scenario и diff
check прошли. Затем исправлены vault combo symbols: marker guard больше не
порождает лишний общий предмет для `1`/`4`/`8`/`9`, а объектная ветвь символа
`1` выполняет good-roll `one_in(8)` перед allocation. Источник:
`src/gen-room.c:1538-1600`. Два regression tests проверяют количество object
attempts и good-roll. Полный suite (411), scenario и diff check прошли.
Второй pass обрабатывает vault symbols по одному; теперь аналогично
интегрирован first pass. `*`/`:` terrain rolls и действия `^`/`&`/`[` отложены
как cell markers, которые исполняются в source order до second pass. Для `&`
создание предмета завершается перед переходом к следующему cell terrain roll.
Интегрированный генератор включает этот режим; standalone resolver сохраняет
старое поведение без deferred markers. Источник: `src/gen-room.c:1400-1480`.
Добавлен RNG-order regression test; focused suite (385), полный suite (413),
scenario и diff check прошли. Подтверждённая граница: модель заранее ставит
проверенную детерминированную форму terrain перед cell actions; выбор origin
остаётся за placement adapter. Следом сверить native origin search и начать
differential fixtures для целого vault generation trace.

Дата аудита: 2026-09-20

## Актуализация плана — 2026-09-28

Первичный аудит ниже фиксирует состояние на 2026-09-20 и не описывает текущую
степень реализации. Целевой проект продолжает развиваться в
`/home/izot/Documents/ANGBAND/SandboxCore2026/projects/angband_model`; его
`implementation_status.md` и `source_manifest.md` — актуальные подробные
реестры перенесённых правил и оставшихся границ.

Сейчас в целевом runtime есть Angband-specific RNG, данные игрока/монстров и
предметов, планировщик энергии, движение и атаки, численные world-turn стадии,
AI-политики, прогрессия, магия, инвентарь, генерация/placement объектов,
проекции, timed effects, ловушки, activation/effect handlers и snapshot.
Последняя связка `finish_native_world_cycle(...)` запускает уже перенесённые
численные стадии мира на десятом ходу в scheduler-порядке.

Модель **не является полной копией Angband**: часть генерации уровня, карты,
LOS/видимости, command/UI поведения, savefile и неперенесённых map/world
handlers остаётся за engine adapters. Exact parity также нельзя заявлять без
native differential fixtures для соответствующих путей.

Продолжать следует небольшими source-backed этапами:

1. Сверять конкретные оставшиеся runtime/adapter границы с native исходниками
   и закрывать те, где все необходимые данные уже есть в модели.
2. Не выводить пустые pack/quiver/equipment позиции из одного лишь списка
   инвентаря: native RNG выбора слота зависит от positional layout; неизвестную
   раскладку оставлять явным adapter input.
3. После закрытия такого участка обновлять целевой status/manifest; differential
   проверки и тесты запускать по отдельному запросу пользователя.
4. Не считать перенос завершённым, пока remaining adapters и native parity
   fixtures не закрыты; полный список см. в целевом `implementation_status.md`.

- Игра: `/home/izot/Documents/ANGBAND/angband`
- Фреймворк: `/home/izot/Documents/ANGBAND/SandboxCore2026`
- Целевой проект: `/home/izot/Documents/ANGBAND/SandboxCore2026/projects/angband_model`

## 1. Краткое резюме

Модель размещается в `/home/izot/Documents/ANGBAND/SandboxCore2026/projects/angband_model`; в корне игры оставлен только этот план. Поэтому предметных правил в текущей модели ещё нет: нет ни правильно реализованных, ни приблизительных, ни заведомо неверных игровых параметров.

Фреймворк уже даёт `SimulationKernel`, seed/RNG, `SimEvent`, EventBus, telemetry, scheduler, `SimulationModel`, composition, declarative combat (`StatDef`, `UnitDef`, `ItemDef`, `StatusDef`, `SkillDef`), progression, snapshot/report/overview и StressRunner. Источники: `SandboxCore2026/src/simcore/simulation.py:1-430`, `composition.py:1-280`, `combat.py:1-300,620-1230`, `progression.py:1-330`.

Но стандартный `Combat.step()` выбирает действия всех доступных units и сортирует их по initiative (`SandboxCore2026/src/simcore/combat.py:1071-1116`). Angband использует накопление energy, таблицу `extract_energy[200]`, порог `move_energy`, отдельный порядок player/monster/world. Нужен Angband-specific runtime/scheduler, а не стандартная combat-формула.

Рабочее дерево игры уже было изменено до аудита: `git status` показывает правки CMake, README, `src/cmd-core.*`, `src/game-world.*`, `src/init.*`, `src/main*`, `src/ui-*` и новые Python bridge-файлы. Эти изменения не трогались и не считаются upstream-источником без differential runtime-проверки.

## 2. Аудит исходников игры

### Базовый тик и порядок выполнения — точно подтверждено

Источники: `src/game-world.c:54-138,440-700,842-922,1005-1140`; `lib/gamedata/constants.txt:10-125`.

- `turn_energy(speed) = extract_energy[speed] * z_info->move_energy / 100`;
- текущие значения: `move-energy=100`, `max-depth=128`, `day-length=10000`, monster max `1024`, spawn chance `1/500`, level minimum `14`, repro max `100`, group max `25`, group distance `5`, OOD chance `1/25`, OOD amount `10`;
- player command обрабатывается до использования energy; cleanup списывает `energy_use`, применяет terrain damage и обновляет state;
- monsters с большей energy обрабатываются до player, затем остальные monsters; после этого world phase, затем player получает energy, увеличивается `turn`, выдаётся `EVENT_TURN`;
- world phase выполняется каждые 10 turns и включает spawn, poison/cut/starvation/regen, timers, food, noise/scent, XP drain, recharge, traps и delayed recall/descent;
- звуки, redraw, animation, message text и UI refresh численно не нужны и должны быть optional adapter events.

### Игрок — подтверждено исходниками/data

Источники: `src/player.h`, `src/player.c:46+`, `src/player-calcs.c`, `src/player-util.c`, `src/player-timed.h`, `src/effect-handler-general.c`; `lib/gamedata/p_race.txt`, `class.txt`, `player_timed.txt`, `player_property.txt`, `world.txt`.

Player — глобальное level-independent состояние со stats, race/class, inventory, hunger, timed effects, XP, HP/mana и equipment. Таймеры не сводятся к обычному duration: `FOOD`, `CUT`, `POISONED`, `STUN`, `COMMAND` имеют специальные decrement rules (`src/game-world.c:260-360`). Damage/death идут через `take_hit()`, reductions, effects и death state; полную цепочку нужно извлечь targeted pass-ом.

### Монстры и AI — подтверждено исходниками/data

Источники: `src/monster.h`, `mon-init.c`, `mon-make.c`, `mon-move.c`, `mon-attack.c`, `mon-spell.c`, `mon-timed.c`; `lib/gamedata/monster_base.txt`, `monster.txt`, `monster_spell.txt`, `blow_methods.txt`, `blow_effects.txt`, `summon.txt`.

Monster хранит race, grid, HP, energy, speed, target, timed flags, lore и group data. Energy loop и reset видны в `src/mon-move.c:1899-2050`. AI использует LOS/projectability, noise/hearing, scent/smell, fleeing, fear, groups, reproduction и race flags. Noise подтверждён `src/game-world.c:1143-1234`, scent — `src/game-world.c:350-450`.

### Оружие, projections и damage — подтверждено частично

Источники: `src/player-attack.c`, `src/mon-attack.c`, `src/project.c`, `src/project-player.c`, `src/project-mon.c`, `src/effect-handler-attack.c`, `src/effects.c`; data `object*.txt`, `artifact.txt`, `ego_item.txt`, `brand.txt`, `slay.txt`, `projection.txt`, `object_property.txt`.

- melee, missile, thrown, launcher, critical, brand/slay, weight, to-hit/to-dam и resistance реализованы разными C-путями;
- projection path зависит от карты, range и flags (`src/project.c:100-380`), поэтому single-target `Combat.ctx.attack()` неэквивалентен;
- `adjust_dam()` подтверждает immunity `resist==3 => 0`, vulnerability `-1 => dam*4/3`, последовательное применение numerator/denominator (`src/project-player.c:36-98`), плюс acid/element side effects (`147-240`);
- object allocation и OOD adjustments находятся в `src/obj-make.c:1-150+`; cumulative allocation нельзя заменить придуманным loot table.

### Spawn, dungeon и waves

Источники: `src/generate.c`, `src/gen-*.c`, `src/mon-make.c`, `lib/gamedata/dungeon_profile.txt`, `room_template.txt`, `vault.txt`, `pit.txt`.

В Angband нет подтверждённой wave-defense механики. Есть initial generation, level allocation и периодический spawn. В модели это должно называться `allocation/spawn phase`; отдельные waves нельзя добавлять как native rule.

### Перки, power-ups, pickups, XP

Отдельных native систем с этими названиями не найдено. Ближайшие подтверждённые механики: player properties/race/class (`player_property.txt`, `p_race.txt`, `class.txt`), objects/effects/activations (`object*.txt`, `activation.txt`, `effects.c`, `effect-handler-*`), inventory/pickup (`cmd-cave.c`, `obj-pile.c`, `obj-gear.c`), XP gain/loss call sites (`player.c`, `mon-attack.c`, `obj-chest.c`, `player-spell.c`, `cmd-cave.c`). Нельзя вводить отдельные perk/power-up/drop coefficients без источника.

## 3. Классификация

| Категория | Правила |
|---|---|
| Точно из исходников | energy table, threshold/order, 10-turn world phase, constants, state structures, resistance branches, parser/data format |
| Документация/тесты | chunk/player/static data, command queue, events — `src/doc/how-it-works.rst`, `src/tests/*`, `tests/*` |
| Косвенно | редкие AI branches, lore timing, save/reload edges; нужны probes |
| Неизвестно | полный RNG draw order, все critical formulas, all race/class derived formulas, complete allocation sequence, dirty bridge semantics |
| Только визуал | glyphs, colours, sounds, animation, UI and message wording |
| Engine adapter | LOS, pathfinding, chunk arrays, level generation, savefile, native command queue, C RNG/global init |

## 4. Архитектура и план реализации

1. **Архитектура.** Создать `AngbandModel`, `AngbandRuntime`, `GameConfig`, `ChunkState`, `PlayerState`, `MonsterState`, `ObjectState`, `TrapState`, `EventTrace`, `UnknownParameter`, `CompatibilityMode`. Загрузить definitions отдельно от runtime. Источники: `src/doc/how-it-works.rst`, `src/init.c:434-703`, all `lib/gamedata/*.txt`. Неизвестные поля — required/None, без guess.

2. **Базовый тик.** Создать `EnergyScheduler`, `TurnState`, `CommandQueue`, `WorldPhase`; перенести literal `extract_energy`, integer arithmetic, command cancellation, monster/player/world order. Источник: `src/game-world.c:54-138,842-922,1005-1140`.

3. **Игроки/co-op.** Реализовать один `PlayerState`: upstream Angband — single-player, подтверждённого co-op нет. Много клиентов можно поддержать только adapter-слоем, не как игровое правило.

4. **Оружие/стрельба.** Создать definitions для weapon/launcher/missile и `AttackContext`; переносить сначала melee, затем ranged/projectile, critical, brands/slays. Источники: `src/player-attack.c`, `src/obj-info.c`, `src/obj-util.c`, object/artifact/ego data.

5. **Урон.** Создать `ProjectionType`, resistance/immunity/vulnerability, typed side effects, dice/aspects. Порядок: raw dice -> path -> target handler -> resistance -> side effects -> damage/death. Источники: `project.c`, `project-player.c`, `project-mon.c`, `projection.txt`, effects.

6. **Монстры/AI.** Создать race/blow/spell definitions, `MonsterState`, target, noise/scent fields и `PathAdapter`; переносить LOS, hearing, scent, fleeing, groups, reproduction, spells. Источники: `mon-init.c`, `mon-move.c`, `mon-attack.c`, `mon-spell.c`, monster data.

7. **Spawn/волны.** Создать `LevelAllocation`, `MonsterAllocator`, `GroupPlacement`, `DungeonGeneratorAdapter`; не называть native allocation waves. Источники: `mon-make.c`, `generate.c`, constants, dungeon/pit/vault data.

8. **Перки.** Отдельный `PerkSystem` не создавать. Использовать `PlayerPropertySet`, race/class modifiers, equipment flags and timed effects. Источники: `player-calcs.c`, `player-class.c`, player data.

9. **Pickups/power-ups.** Создать `FloorPile`, `Inventory`, `Equipment`, `EffectExecutor`; слова power-up/pickup оставить aliases scenario/UI layer. Источники: `cmd-cave.c`, `obj-pile.c`, `obj-gear.c`, effects/activation data.

10. **XP/levels/rewards.** Создать custom Angband progression с exact `player_exp[]`, exp factor, level transitions, stat/HP/mana recalculation, drain/loss and death reward. Источники: `player.c`, all `player_exp_gain/player_exp_lose` call sites. Generic framework progression без custom formulas использовать нельзя.

11. **Bosses/special entities.** После базового death pipeline: uniques, Morgoth, quests, pits, vaults, artifacts, summons, polymorph. Источники: `monster.txt`, `quest.txt`, `artifact.txt`, `vault.txt`, `pit.txt`, `project-mon.c`, `effect-handler-general.c`.

12. **Timers/states.** Создать typed timer subsystem с special decrement policies, а не только `StatusDef.duration`. Источники: `player_timed.txt`, `list-player-timed.h`, `list-mon-timed.h`, `game-world.c:260-360`, `mon-timed.c`.

13. **RNG.** Создать `RngAdapter` с integer semantics `randint0/1`, `one_in_`, `damroll`, `randcalc`, named streams or documented single stream, draw trace and state snapshot. Источники: `src/z-rand.*`, `src/z-dice.*`, call sites. Python `random.Random` не считать эквивалентом C RNG.

14. **Snapshot/events.** Snapshot: seed, RNG state/positions, turn, energy, depth, chunk, player, monsters, objects, traps, timers, inventory, pending commands. Events: command, energy, move, attack, projection, damage, resist, status, spawn, death, XP, level, pickup, level change, world phase.

15. **Debug report.** `overview()` — definitions/config; `snapshot()` — full state; `report()` — metrics; `debug_trace()` — ordered events/RNG; `unknowns()` — unresolved parameters with search locations and confidence.

16. **Tests.** Port/adapt `src/tests/game/*`, `monster/attack.c`, `object/attack.c`, `object/alloc.c`, `effects/*`, `player/*`, `parse/*`; сделать native golden fixtures, не hand-written guesses. После каждого этапа: model run, numerical assertions, snapshot diff, regression suite, `git diff --check`.

17. **Runtime parity.** Создать native probe/headless adapter. Сравнивать controlled scenarios: one command, one monster blow, one projection, one 10-turn world phase, level transition, generation. Exact mode и fixed/bug-compatible mode разделить.

18. **Performance.** Только после parity: immutable definitions, sparse indexes, integer grid arrays, sampled telemetry, no deep-copy per event. Не менять RNG/event order ради скорости.

## 5. Таблица трассировки

| Механика | Статус в модели | Источник | Точность | Что сделать |
|---|---|---|---|---|
| Seed/events/telemetry | Есть generic framework | `simcore/simulation.py` | Не Angband-specific | Обернуть native RNG/state |
| Composition | Есть generic | `simcore/composition.py` | Недостаточно для energy loop | Один `angband` domain + custom scheduler |
| Declarative combat | Есть generic | `simcore/combat.py` | Default formulas неверны для parity | Использовать только schema, custom rules |
| Energy/order | Нет | `src/game-world.c:54-138,1005-1140` | Точно подтверждено | Перенести table and lifecycle |
| World phase | Нет | `src/game-world.c:462-700,1074-1084` | Точно подтверждено | Implement every-10-turn phase |
| Player/race/class | Нет | `src/player.*`, player data | Данные есть, formulas incomplete | Extraction + fixtures |
| Timers | Нет | `player_timed.txt`, `game-world.c:260-360` | Special cases | Typed timers |
| Melee/ranged | Нет | `player-attack.c`, `mon-attack.c`, `project.c` | Extraction pending | One path at a time |
| Resistance | Нет | `project-player.c:36-98`, `projection.txt` | Branch exact, tables pending | Port integer order/tables |
| Monster data/AI | Нет | `mon-*.c`, monster data | Data known, engine-dependent | Loader + grid adapter |
| Spawn/generation | Нет | `mon-make.c`, `generate.c`, constants | Parameters known, RNG pending | Native-controlled allocator |
| Waves | Нет в оригинале | Native audit | N/A | Не добавлять |
| Pickups/objects | Нет | `cmd-cave.c`, `obj-*`, effects | Частично data-driven | Inventory/effect adapter |
| XP/levels | Generic only | `player.c`, gain/loss sites | Generic formula wrong | Custom progression |
| RNG parity | Python RNG only | `z-rand.*`, dice | Неэквивалентно | Native-compatible adapter/trace |
| Snapshot/report | Generic only | framework APIs | Неполно | Canonical Angband schema |
| UI/audio | Нет numeric need | `ui-*`, `sound.*` | Visual-only | Optional adapters |

## 6. Ограничения, баги и неизвестные

- Нельзя честно подставлять hit chance, mitigation, crit multiplier, cooldown, wave interval, drop chance, XP curve, AI interval, resistance coefficient или co-op rules.
- Неизвестное хранить как required/None с указанием, где искалось; exact mode должен останавливаться, а не использовать zero/one/typical value.
- UI, rendering, sounds, animation и messages — adapters; LOS, pathfinding, chunk, generation, savefile, command queue и global initialization — engine-specific.
- Compatibility modes: `UPSTREAM_EXACT`, `NATIVE_DIRTY_TREE`, `FIXED`. Исправления нельзя смешивать с exact replay. Dirty Python bridge (`src/python-*`, `lib/scripts/*`, modified `game-world.c`) требует отдельного audit.
- Баги и дублированные/подозрительные участки source должны быть заведены как compatibility findings, но не «исправлены» при переносе exact behavior.

## 7. Приоритеты

**Высокий:** baseline commit/dirty patch, data/constants loader, RNG adapter, energy scheduler, player/monster state, basic melee/death/XP, canonical trace, resistance/timers.

**Средний:** projectiles/map geometry, monster AI, spawn/allocation, objects/inventory, generation, spells, artifacts, traps, uniques, stores.

**Низкий:** UI/audio adapters, savefile export, analytics, optimization after correctness.

## 8. Файлы для будущих этапов

```text
angband_model/
  PLAN.md
  project.toml
  angband_model/{__init__.py,config.py,data_loader.py,rng.py,state.py,
                 scheduler.py,combat.py,monsters.py,objects.py,generation.py,
                 runtime.py,adapter.py,reporting.py}
  tests/test_*.py
  experiments/run_scenario.py
```

На этом этапе изменены только файлы проекта в `SandboxCore2026/projects/angband_model` и этот план; `src/simcore` и исходники игры не менялись агентом.

## 9. Команды запуска и проверки

Сейчас target model ещё не runnable. Целевые команды после первого этапа:

```bash
cd /home/izot/Documents/ANGBAND/angband
git status --short
./tests/run-test
./build/unittests

cd /home/izot/Documents/ANGBAND/SandboxCore2026
PYTHONPATH=src .venv_f/bin/python -m unittest discover -s tests

cd /home/izot/Documents/ANGBAND/SandboxCore2026
PYTHONPATH=src:projects/angband_model \
  python -m angband_model.scenarios
PYTHONPATH=src:projects/angband_model \
  python -m unittest discover -s projects/angband_model/tests
git diff --check
```

Точный native build command нужно подтвердить по `src/tests/README` и текущему build directory; dirty Python bridge не считать обязательным baseline.

## 10. Критерии готовности

1. Для каждого gameplay field есть `file/function/line/formula/value/confidence`.
2. Unknowns не скрыты defaults и блокируют exact mode.
3. Energy/order/RNG воспроизводимы и совпадают с native trace.
4. Player, monster, object, projection, status, XP/death и generation fixtures дают zero semantic diff.
5. Snapshot позволяет детерминированно продолжить simulation.
6. Exact/compatibility/fixed modes разделены.
7. Framework, project, native и differential tests проходят.
8. Нет ненужных внешних зависимостей.
9. Performance измерена после correctness и не меняет event/RNG order.

## 11. Вопросы первичного аудита (не блокируют текущую работу)

1. Authoritative baseline — clean commit `5da31305...` или текущий dirty runtime с Python bridge?
2. Нужен идентичный C RNG sequence или достаточно native trace-replay при совпадении observable outcomes?
3. Сначала controlled arenas/chunks или сразу полный dungeon generation?
4. Какой scope объектов нужен первым: melee, ranged, consumables, spells, artifacts, stores?
5. Нужен savefile-compatible export или только deterministic model snapshot?
6. Требуется ли внешний multi-player adapter, если upstream — single-player?
7. Какие dirty-tree Python changes должны стать отдельным compatibility target?

Пользователь явно поручил продолжать перенос. Для спорных исходных режимов
использовать чистое upstream-поведение там, где оно подтверждено; dirty-tree
совместимость не смешивать с native exact rules без отдельного указания.

Актуализация 2026-10-13: сверены `find_space()` (`src/gen-room.c:1005-1096`)
и основной выбор блока (`src/gen-cave.c:1195-1239`). В модели появились
source-backed примитивы `native_find_space()` и `native_select_untried_block()`;
проверены порядок RNG, 25 попыток для поиска места, резервирование footprint,
проверка границ и reservoir-порядок y/x. Они пока не заменяют интегрированную
генерацию: там ещё нет общего состояния `blocks_tried`/`room_map` и отката
резерва при неудаче builder. Точечные проверки прошли; полный suite содержит
415 тестов и прошёл. Дополнительно устранён выдуманный случайный выбор room
profile: key/rarity теперь бросаются для каждого явно переданного origin, а
берётся первый подходящий профиль в исходном порядке. Это ещё не реализует
переход к следующим профилям после отказа builder. Полный suite после этого
изменения: 416 тестов, сценарий и diff check прошли. Следующий этап — встроить
native block scheduler и последовательные builder attempts с room-map
резервированием, затем сделать differential fixture terrain/RNG для vault.

Актуализация 2026-10-13: добавлен `AngbandRuntime.schedule_room_builds_native()`
и scheduler в `generation_data.py`. Он переносит room-count scaling
(`gen-cave.c:1141-1154`), выбор/пометку untried block, key/rarity rolls,
профильный fallback, depth/pit gates и успешное резервирование footprint
(`gen-cave.c:1195-1245`, `gen-room.c:3636-3689`). Тест использует builder
callback, который отклоняет одну попытку, и подтверждает переход в исходном
порядке без преждевременного reserve. Следом подключить профильные builders к
этому циклу, затем добавить vault differential fixture. Численная проверка
покрывает все шесть веток room-count scaling и quest override. Полный suite:
418 тестов, сценарий и diff check прошли.

Следующий builder-substage: добавлены native выбор room template по type/rating,
проверка освещения и RNG-порядок light → random door → optional walls →
symmetry → symbol pass. Source: `src/gen-room.c:55-68,1111-1152,1248-1251,
1343-1355`. Свет переносится в `glowing_terrain`; `SQUARE_ROOM` и inline
materialization содержимого остаются адаптерными. После регрессионной проверки
этого подэтапа продолжить интеграцию template builder в scheduler, затем
перейти к процедурным builders.

Актуализация: `build_room_template_profile_attempt_native()` теперь можно
передать как builder callback для profile `room template`: он вызывает native
type 1/rating selection, устанавливает terrain и lighting, сохраняет layout в
caller-owned sink и возвращает native success/failure. Контентные markers
остаются deferred до content adapter, поэтому этот путь не объявляется полным
runtime parity для шаблонов с объектами, traps или guards. Интегрирующий тест
пройден; полный suite после него — 419 тестов.

Актуализация 2026-10-14: native fixture для `monster_index_move()` расширен
через публичные target API. Подтверждено, что health reference перемещается на
новый слот, а monster target очищается: исходный код проверяет target после
смены `midx`, но до копирования monster в destination slot. Этот порядок
перенесён в persistent transition adapter policy как отдельные действия
сохранения health identity и очистки monster target; исправлением игры это не
считается. Differential, stage и полный Python suites прошли (29, 416 и 445
тестов), native parity 5/5, black-box 5/5, scenario, compileall и diff checks
прошли. Следом исследовать возможность полного native fixture для порядка
`prepare_next_level()`/persistent restore; отдельно сохранить vault terrain/RNG
differential из следующего этапа генерации.

Продолжение 2026-10-14: вместо небезопасного вызова `prepare_next_level()` без
инициализированных engine globals добавлен изолированный C fixture для
`age_scent()` и `forget_noise()`, вызываемых при persistent restore. Нативный
результат сравнивается с модельной функцией: при turn 250 и сохранённом turn
100 прирост scent равен 15 (целочисленные десятые turn), переполнение uint16
обнуляет scent, внутренний noise очищается, границы сохраняются. Первая версия
native parity прошла 6/6; differential test связывает C output с Python правилом. Затем
добавлена проверка native `chunk_list_add/find/remove` для пары primary/known
вокруг несвязанного уровня; она фиксирует list compaction и сохранение
соседнего chunk. Pair-level model storage/lookup/removal совпадает с конечным
результатом, хотя модель удаляет pair атомарно. Native parity теперь 7/7.
Затем добавлен initialized-engine round-trip в `src/tests/game/basic.c`:
`prepare_next_level()` выполняет depth 1 → 2 → 1; test фиксирует запись пары,
восстановление и удаление после restore. Differential test сопоставляет это с
model commit flow. Точные player coordinates, восстановленные entity snapshots
и RNG state пока не выгружаются для численного сравнения. После обновления
прошли differential tests (32), stage suite (416), полный suite (448), native
parity (7/7), game/basic (7/7), native black-box (5/5), scenario, compileall
и оба diff checks.

Дополнительная source проверка обнаружила placement mismatch: regular
persistent return передаёт в `sanitize_player_loc()` текущую позицию игрока с
исходящего уровня; координата игрока не является частью `struct chunk`. Model
commit теперь использует текущий `PlayerState.position` как default origin
(явное adapter значение может переопределить его). Stage и differential tests
разводят исходящую позицию `(2, 2)` и координату в destination pair `(1, 1)`,
подтверждая выбор `(2, 2)`.

Round-trip fixture сначала проверяет общую безопасную клетку: native оставляет
её без random search, а JSON record показывает координаты и неизменность всего
RNG state. Затем fixture оставляет на stored map единственную свободную vault
клетку: native проходит 1000 неуспешных random попыток, scan и vault fallback.
Перед/после выводятся полные `STATE[]`, `state_i`, `Rand_value` и `Rand_quick`;
differential Python resolver совпадает по координате, `vault_fallback` и всем
полям RNG state. Успешный random выбор non-vault клетки, arena placement и
monster/object snapshots пока остаются открытыми.


Артефактный copy-layer дополнен native fixture: `copy_artifact_data()` на
`'Angrist'` после `object_prep()`. Differential сравнивает итоговый object
record с model value/combat results: kind, weight, timeout, полный combat
block, modifier vector в native enum order и все resistance levels в порядке
`list-elements.h`. Metadata stage подготавливает объект, но C bitflags и side
tables этим тестом пока не сравниваются. Источник копирования —
`src/obj-make.c:520-565`; native `game/basic` — 7/7, focused differential —
пройден.

Актуализация 2026-10-17: предыдущий список открытых пунктов устарел — все эти
ветки уже присутствуют в `src/tests/game/basic.c` и проверяются тестом
`test_native_persistent_level_round_trip_matches_model_commit()` в
`projects/angband_model/tests/test_differential.py`. Fixture покрывает прямое
восстановление координаты без RNG, vault fallback после 1000 неуспешных
случайных кандидатов, успешный non-vault random candidate с итоговым полным
RNG state, arena возврат через `old_grid`, player marker, ближайшую безопасную
клетку к первому monster и random sanitize fallback. Differential также
сопоставляет monster/object snapshots, known-object association и сохранение
native side-table свойств. Проверка: `PYTHONPATH=../../src python -m unittest
discover -s tests -p test_differential.py -k persistent_level_round_trip -v` —
1 тест прошёл; `game/basic` завершил 7/7.

Актуализация 2026-10-17: вход на новый этаж теперь выводит `no_light()` из
моделируемых native inputs: экипированный light radius, освещённость клетки и
`UNLIGHT`; при неизвестном monster-emitted lighting оставляет явную границу
engine adapter. Arrival `search()` открывает найденные секретные двери в
соседней 3×3 области с native guards. Целевой тест поиска и двухэтажный
сценарий прошли; проверка показала `applied` на обоих этажах и native light
radius 2. Известные trapped chest, monster race light и UI/event эффекты в этот
шаг не включены.

Следующий генерационный этап: получить differential fixture для terrain и RNG
vault builder, продолжая отдельно помечать отложенные content/materialization
адаптеры. Runnable campaign skeleton уже есть; не расширять в этом этапе
редкие игровые правила, пока fixture не зафиксирует native vault output.

Актуализация 2026-10-17: добавлен controlled fixture вокруг native
`build_vault()` в `src/tests/game/basic.c` и differential в
`projects/angband_model/tests/test_differential.py`. Для шаблона 2x5 с
символами `%/#/@/*/:` совпали размещённые terrain cells и полный RNG state
после native symmetry transform и первого прохода glyphs. Fixture также
выводит native `SQUARE_ROOM`, `SQUARE_VAULT`, `SQUARE_WALL_OUTER` и
`SQUARE_WALL_SOLID`; на момент этой записи per-cell state ещё отсутствовал,
что закрыто следующим подэтапом ниже.
Символы содержимого, monster/object/trap materialization и полный random
placement здесь намеренно не проверяются. Проверка: `cmake --build build
--target unittest-game-basic -j2`; `PYTHONPATH=. python -m unittest discover -s
tests -p test_differential.py -k native_vault_terrain_and_rng -v` — прошёл;
native `game/basic` — 7/7.

Продолжение 2026-10-17: `ChunkState.square_flags` теперь хранит native
`SQUARE_MON_RESTRICT`, `SQUARE_ROOM`, `SQUARE_VAULT`, `SQUARE_WALL_OUTER` и
`SQUARE_WALL_SOLID`. Vault resolver ставит MON_RESTRICT на весь transformed
bounding box, включая пустые glyph cells; layout placement сдвигает эти флаги
вместе с terrain, install добавляет их в chunk, snapshot/restore сохраняет их.
Differential сравнивает точные C-флаги с установленной модельной картой и
проверяет перенос флагов на новый origin и обратное восстановление snapshot.
Тесты: native build `unittest-game-basic`, differential `native_vault_terrain_and_rng`,
регрессии `vault_first_pass_interleaves_object_allocation_with_later_terrain_rng`
и `native_room_template_selection_light_and_geometry_rng_order`, one_room — победа
за 32 хода. Следующий этап: применить native flags в условиях генерации и
разобрать inline-порядок содержимого vault; редкие content rules и сложные
placement adapters оставить последующими шагами.

Продолжение 2026-10-17: в `generate_level_layout_native()` материализация
содержимого vault-профиля перенесена сразу после установки его геометрии,
перед следующей попыткой комнаты. Добавлена регрессия, фиксирующая этот порядок.
Подтверждение C: `src/gen-room.c:1369-1719` — `build_vault()` выполняет
terrain/object pass, content pass и `get_vault_monsters()` до возврата.
Проверены новая регрессия, прежний тест
`vault_first_pass_interleaves_object_allocation_with_later_terrain_rng`,
differential fixture terrain/RNG/flags и `one_room` (победа за 32 хода).
Ограничение: placement для части vault-маркеров всё ещё зависит от адаптеров,
а содержимое невольтовых комнат по-прежнему материализуется пакетом после
layout loop. Поэтому полный межкомнатный RNG-порядок пока не считается точным.
Следующий шаг — переносить завершённое содержимое каждого поддержанного room
builder в точку его native возврата, сохраняя порядок и отмечая engine-adapter
handoffs для неподдержанных действий.

Обновление 2026-10-17: обычные layout builder-ы в двух путях модели
(`use_native_room_scheduler` и `candidate_origins`) теперь вызывают
`materialize_generated_layout_native()` сразу после установки layout и до
следующей попытки комнаты. Добавлены две regressions: template content и vault
content обязаны попасть в timeline до второй попытки. Источник порядка для
room template: `src/gen-room.c:1111-1339` — первый проход features/объектов,
затем second pass, затем возврат; vault: `src/gen-room.c:1369-1719`.
Scheduler build callbacks вызывают room builder последовательно; итоговое
материализованное summary сохраняется в `result['materialized']['room']` без
повторного исполнения уже обработанных layout.

Проверки: обе inline-order регрессии; три scheduler проверки для template,
simple и overlap room; тесты template RNG и vault first-pass RNG; `git diff
--check`; `./run_mvp.sh` (`one_room`: won за 32 хода). MCP HTTP endpoint снова
не ответил за 3 секунды; новый сервер не запускался.
Ограничения остаются: crossed-room callback и редкие content actions имеют
отдельные engine-adapter границы; этот этап подтверждает порядок вызова
материализатора, но не доказывает численное совпадение каждого allocation или
полного RNG trace между всеми room families. Следующая работа — differential
fixture на один room-template glyph с содержимым и native/model RNG/result,
затем отдельные adapter handoffs для неподдержанных действий.

Продолжение 2026-10-17: controlled `build_template()` fixture теперь включает
закрывающуюся дверь, которая при выбранном RNG seed получает lock. Сначала
differential выявил отличие RNG на два вызова: native `place_trap()` делает
`randcalc(kind->power, ..., RANDOMISE)` при создании door-lock trap, и даже
нулевой `m_bonus(0, 0)` расходует два значения в `simulate_division()` до того,
как сила ловушки заменяется силой замка. Модель теперь воспроизводит этот путь
через `roll_native_m_bonus(0, 0, rng)`; `_rand_normal()` также выполняет native
ранний возврат для `stand < 1`. Источники: `src/trap.c:356-386,706-723`,
`src/z-rand.c:408-413,464-476,484-512`, `src/gen-util.c:587-592`,
`src/gen-room.c:1183`. Differential fixture сравнивает terrain, glow, lock
power и полный RNG state, затем проверяет установленную side-table ловушки.
Проверки: native room parity тест прошёл; все 34 `test_differential.py`
прошли; `one_room` завершился победой за 32 хода, 8/8 фаз; `git diff --check`
чистый. Общий набор Python-тестов был прерван после более 90 секунд без
итогового отчёта; он не считается пройденным. Следующий шаг — продолжить
покрытие простого room-template content glyph и отделить действия, требующие
engine adapter, не расширяя fixture до редких механик.

Продолжение 2026-10-17: MCP HTTP `trace_rule(place_object)` и
`compare_rule(case '[' / typed_treasure)` подтвердили, что room-template glyph
`[` в `src/gen-room.c:1223-1228` вызывает `place_object()` для заданного tval.
Добавлен отдельный native C fixture для `TV_FOOD` и модельный differential:
обе стороны создали в одной клетке один и тот же предмет, Handful of Dried
Fruits. Контентный fixture намеренно отделён от предыдущей точной проверки
terrain/glow/locked-door/RNG: после создания объекта native завершает с
`state_i=10`, модель — с `state_i=24`. Следовательно, текущий generated-object
factory не воспроизводит RNG всего `make_object()`/`apply_magic()` пути и это
нельзя объявлять подтверждённым. Источники: `src/gen-room.c:1223-1228`,
`src/obj-make.c:1162-1233` и модель `runtime.py` object factory. Проверки:
`unittest-game-basic` — 7/7; `test_differential.py` — 35/35; `one_room` — победа,
8/8 фаз; `git diff --check` чистый. Полный Python suite не запускался повторно:
предыдущий полный запуск был остановлен после 90 секунд без итога. Следующий
шаг для скелета — сохранить `[` как работающий content path и явно маркировать
его RNG как неполный engine/object-generation handoff; точную генерацию магии,
артефактов и стеков оставить отдельным расширением, вместо того чтобы
задерживать end-to-end сценарий.

Продолжение 2026-10-17: в результатах room-template generation добавлены
`content_rng_handoffs` и `content_requires_adapter_rng`. Для glyph `[` handoff
содержит координату маркера, причину (неполный RNG `make_object()`/
`apply_magic()`) и переход к исходным функциям C. Сводка также поднимается в
результат `generate_level_layout_native()`, чтобы потребитель отчёта мог увидеть
ограничение без анализа списка внутренних маркеров. Для комнаты без такого
маркера список пуст и флаг выключен. Добавлены assertions в differential
fixtures для обоих случаев.

Проверки: обе затронутые differential проверки прошли; `./run_mvp.sh`
завершил только `one_room` победой за 32 хода, все 8 фаз complete;
`git diff --check` чистый. MCP повторно не вызывался: нужный путь C уже был
проверен предыдущими запросами, эта правка только переносит известное
ограничение в модельный отчёт. Следующий приоритет — вывести эти поля в
пользовательский debug/report snapshot при фактической генерации этажа, затем
продолжать следующий частый content glyph с отдельной parity-проверкой.

Продолжение 2026-10-17: `generate_scenario_level()` теперь переносит сводку
`content_requires_adapter_rng`/`content_rng_handoffs` из native layout result в
level phase JSON. `SimulationRun` также печатает эти поля в `phase_done`, чтобы
ограничение было видно в консоли. Fixture-level по умолчанию сообщает `false`
и пустой список — он не изображает случайную генерацию оригинала. Проверены
оба differential fixture и `./run_mvp.sh`: `one_room` выигран, 8/8 фаз
завершены, JSON и консоль содержат поля. Следующий шаг — передать в
генераторный trace доказуемые handoff-ы для остальных частых content marker-ов;
для каждого сначала определить точный native content path, не превращая общий
флаг в необоснованное заявление о RNG-полноте.

Продолжение 2026-10-17: проверил живой MCP HTTP (initialize/tools/list и
read-only `search_game_source`/`trace_rule`). MCP подтвердил в
`src/gen-room.c:1184` путь `^ -> one_in_(4) -> place_trap`; `1207-1216` для `8`
путь выбора `place_object` или `place_random_stairs`; `1219`, `1287-1307` для
`9` — отдельный контентный проход стражи/сокровища; `1223-1228` для `[` —
`place_object` с заданным tval. В deferred room-template layouts теперь все
эти четыре типа имеют структурированные `content_rng_handoffs` с координатой,
причиной и ссылкой на C. Для `8` также явно записано, что текущий materializer
не выполняет native 80/20 object/stairs branch; для `^` указано, что parity
выбора ловушки и RNG пока не доказана; для `9` — что parity порядка и RNG
выбора стражи/сокровища не доказана. Это маркировка ограничения, механики не
объявлены точными.

Проверки: новый marker-audit test и две существующие room-template
differential-проверки прошли; `one_room` — победа и 8/8 фаз, fixture level
выводит пустой handoff; `git diff --check` и `py_compile` прошли. На следующем
этапе `8` перенесён точнее: при неперсистентном уровне модель делает native
`randint0(100) < 80`; при выборе stairs делает второй `randint0(100) < 50` и
сохраняет запрошенное направление. При `persistent=True` первый бросок
пропускается, как в C short-circuit. `random_stairs` materializer ставит
`LESS`/`MORE` с native правилами town depth (всегда вниз), quest/max depth
(вверх) и обычным запрошенным направлением. Источники: MCP
`src/gen-room.c:1207-1216`, `src/gen-util.c:450-484`. Новый RNG regression
сравнивает полные состояния двух native-compatible RNG потоков на 64 seed и
проверяет persistent short-circuit; отдельная проверка покрывает направление
лестницы для обычного, town, quest и последнего этажа. Все 38 differential
тестов прошли; `one_room` победил за 32 хода (8/8 фаз). RNG полного
`place_object()` всё ещё отложен до конца layout-обработки и отмечается
handoff-ом; объектная ветка `8` пока не считается точной.

Следующий шаг — differential fixture с native room-template glyph `8`, которая
сверит ветку, feature клетки и RNG для stairs-исхода; затем отдельно решать
межмаркерный порядок `place_object()` без расширения редких механик.

Продолжение 2026-10-17: MCP trace подтвердил `place_random_stairs()` и выбор
`FEAT_LESS`/`FEAT_MORE` (`src/gen-util.c:480-484`), а также второй проход glyph
`8` (`src/gen-room.c:1287-1293`) — `vault_monsters(c, grid, depth + 2,
randint0(2) + 3)`. Resolver теперь добавляет второй-pass marker с точным count
roll после обработки всех first-pass glyph-ов; runtime направляет его в
`place_vault_monsters_native()`. В результате fixture ветки `8` проверяет
object/stairs roll, stair-direction roll, second-pass count draw, порядок RNG
и persistent short-circuit на 64 seed. Random stairs применяют depth/quest
правила; соответствующие handoff-ы сообщают, что native friend/escort group
expansion остаётся адаптерной границей.

Пытался включить полноценный C parity fixture `build_template()` с glyph `8`.
Он периодически падал внутри native guard/group placement с assertion
`square_isno_esp(): square_in_bounds(c, grid)`, хотя отдельные запуски проходили.
Чтобы не оставлять flaky тест и не менять native runtime, экспериментальный
fixture удалён. После этого C target `unittest-game-basic` пересобран, его
бинарник прошёл пять последовательных запусков. Поэтому точная формула ветки
подтверждена MCP-источником и RNG regression модели, но end-to-end C parity
для генерации монстров glyph `8` остаётся незакрытой; возможный native bug
требует отдельной изоляции.

Проверки текущего состояния: все 38 `test_differential.py` прошли; пять
затронутых targeted tests прошли; `one_room` завершился победой и 8/8 фаз;
`py_compile` и `git diff --check` чисты. Следующий этап — исследовать и
изолировать flaky native assertion без включения нестабильного fixture, после
чего строить отдельный устойчивый C probe для glyph `8`. RNG object allocation
при 80% object-ветке всё ещё deferred и остаётся главным модельным handoff.

Проверка продолжения 2026-10-17: повторно пройдены все 38 differential tests,
`one_room` (победа за 32 хода, 8/8 фаз), `py_compile` и `git diff --check`.
HTTP MCP оставался доступен; `trace_rule(vault_monsters)` подтвердил вызов из
`build_room_template` и тело `src/gen-util.c:895-918`, где `scatter_ext()` ищет
свободную клетку и затем вызывается `pick_and_place_monster()`.
`trace_rule(update_mon)` показал, что `place_monster()` зовёт `update_mon()`
(`src/mon-make.c:1035`), а `update_mon()` проверяет `square_isno_esp()` как для
клетки монстра, так и для клетки игрока (`src/mon-util.c:342`). Сам assertion
`square_isno_esp(): square_in_bounds(c, grid)` (`src/cave-square.c:554-556`)
не доказывает, какая из этих координат была неверной. Временный glyph-`8`
fixture с seed 6 поймал assertion под gdb; native backtrace подтвердил путь
`build_template -> build_room_template -> vault_monsters -> pick_and_place_monster
-> place_new_monster -> place_monster -> update_mon -> square_isno_esp`.
Координаты и размеры chunk в этой сборке без debug locals не раскрыты. Пять
повторных запусков после удаления временного fixture прошли 7/7; исходный
`src/tests/game/basic.c` восстановлен побайтно, тест не оставлен в дереве.
Причина уже локализована до guard spawn/update path, но нельзя утверждать,
какая координата/предусловие нарушено или что это общий баг Angband. Следующий
шаг — временная Debug-сборка с тем же fixture и условный останов на
`square_isno_esp()`, чтобы зафиксировать `grid`, `mon->grid`, `pgrid` и размеры
chunk. До этого C parity для glyph `8` остаётся отложенной и нестабильный тест
не возвращать.

Завершение изоляции 2026-10-17 (заменяет предыдущий следующий шаг): временная
Debug-сборка с glyph-`8` fixture поймала точку отказа. В `update_mon()` неверен
был `pgrid`, а не `mon->grid`: обе координаты равнялись `(44, 4)`, тогда как
fixture создавал chunk `40×40`. Причина — helper теста переносил генерацию на
временный chunk, но оставлял глобальную координату игрока с настоящего этажа.
Это ошибка fixture harness, а не подтверждённая ошибка игровой механики.

Helper `emit_native_room_template_fixture()` теперь временно ставит игрока в
центр тестового chunk и восстанавливает координату после генерации; seed стал
параметром helper-а. Добавлены native fixtures для glyph `8` на seed 4
(лестничная ветка) и seed 6 (объектная ветка) и differential assertion, что
модель и C выбирают одинаковую ветку, координату и feature; объектная проверка
подтверждает только факт появления объекта, не его генерацию или RNG parity.
Полный `vault_monsters()` RNG после guard spawn остаётся вне этой parity
проверки: native group/race selection и object allocation требуют отдельного
разбора.

После исправления: `unittest-game-basic` прошёл 5 последовательных запусков
(7/7 каждый), `tests.test_differential` — 39 тестов, `one_room` — победа за 32
хода, 8/8 фаз. Проверки `py_compile` и `git diff --check` выполняются перед
закрытием этого этапа. Следующий полезный этап — измерить и описать RNG handoff
для `vault_monsters()` после корректного fixture, затем отдельно перейти к
object allocation ветки `8`.

Исследование RNG handoff 2026-10-17: MCP `trace_rule` для `place_new_monster`,
`get_mon_num`, `monster_group_assign` и `mon_create_drop` подтвердил точки
вызова: `pick_and_place_monster()` выбирает race (`src/mon-make.c:1461`),
`place_new_monster()` делает placement leader, затем friends и friends-base
(`src/mon-make.c:1385-1434`), а `place_monster()` назначает группу, обновляет видимость и
вызывает spawn-drop pipeline при ненулевом origin (`1033-1047`). Внутри
`place_new_monster_one()` RNG order включает sleep, HP, speed, energy и random
attribute (`src/mon-make.c:1141-1193`). `vault_monsters()` выбирает локальную
клетку через `scatter_ext()` до каждого race roll (`src/gen-util.c:901-915`).

В модели уже есть native allocation (`allocation.py`), native spawn RNG
(`monsters.py`) и friend/friends-base placer (`spawning.py`), однако прямой
`vault_monsters` marker в `materialize_generated_layout_markers()` сейчас
раньше вызывал placement без group callback и не передавал origin. В этой
итерации прямой путь подключён к существующему `SpawnPipeline` и source-backed
`ORIGIN_DROP_SPECIAL` передаётся leader-у и его группе; crossed-room default
callback теперь также наследует тот же origin из контекста. Добавлен тест для
native group на реальной race data без spawn drops, чтобы отдельно подтвердить
связь callback и origin. Общий RNG parity всё ещё не заявляется: race allocation,
spawn-time drops/mimics и friend placement должны быть сверены на C fixture;
handoff теперь называет оставшиеся race-specific drop/mimic случаи, а не
неподключённый group callback.

Полный прогон `test_differential` + `test_stage1` сначала выявил один старый
ожидаемый результат `ego_combat`, не связанный с vault placement. Источник
`src/z-rand.c:407-410, 462-469` подтверждает два `simulate_division()` RNG draw
в `m_bonus(0, level)`; поэтому тестовый ожидаемый кортеж исправлен на фактический
source-compatible результат `(1, 4, 4)`. Целевой тест после поправки проходит;
полный повторный прогон завершился успешно: 516 тестов. `one_room` после group
callback также победил за 32 хода с 8/8 фазами; `py_compile` и `git diff --check`
прошли. После этого output marker materializer дополнен ID созданных friend
monsters; его regression теперь идёт через полный layout-marker путь. Повторно
прошли group-origin targeted test, все 39 differential tests и `one_room`.

Короткая синхронизация 2026-10-17: default `vault_monsters` marker уже
материализует leader, передаёт `ORIGIN_DROP_SPECIAL`, прогоняет имеющиеся
подтверждённые spawn-drop/mimic обработчики и вызывает `SpawnPipeline` для
friends/friends-base. MCP `find_model_coverage` подтвердил эту границу; MCP
`trace_rule(place_monster)` подтвердил C placement entry point и callers
(`src/mon-make.c:999-1001,1193`). Обновлены `source_manifest.md` и
`implementation_status.md`, где оставался устаревший тезис, будто группа только
внешний callback. Прямая низкоуровневая materializer-функция по-прежнему
возвращает `groups_pending`, если её вызывают без callback. Следующий этап — не
расширять редкие механики, а проверить короткими native fixtures общий RNG
handoff для vault race allocation → placement → группы/drop/mimic; пока parity
там не доказана, отчёты не должны объявлять её полной. Полный набор тестов и
многоэтажные сценарии в этой итерации не запускались.

Исправление vault scatter 2026-10-17: MCP `trace_rule(square_isempty)` и
`src/cave-square.c:578-607` подтвердили, что C-кандидат использует floor без
монстра, player trap, web и object, но не фильтрует игрока. `place_new_monster_one`
проверяет игрока позже (`src/mon-make.c:1087-1101`), уже после race allocation.
`scatter_ext()` модели раньше предварительно исключал клетку игрока через общий
`can_place_monster`, что могло менять размер пула, выбранную клетку и последующие
RNG результаты. Vault путь теперь передаёт native predicate напрямую
(`predicate_only=True`), а сам predicate явно исключает монстров. Добавлена
проверка: при первом выбранном player-grid candidate поиск останавливается и
передаёт его placement callback после одного scatter draw. Целевой тест прошёл;
`one_room` завершился победой за 32 хода, 8/8 фаз; `git diff --check` прошёл.
Полные тесты и длинные сценарии пропущены. Следующий ограниченный шаг — сверить
близлежащие placement predicates и RNG handoff для friend групп на таком же
узком C/model сравнении.

Сверка friend placement 2026-10-17: MCP `trace_rule(place_friends)` и
`trace_rule(place_new_monster_group)` подтвердили C call chain. `spawning.py`
совпадает с подтверждённым порядком: leader, linked-list friends, затем
friends-base; в named/base группах `square_isopen` допускает клетку игрока, а
размещение проверяет её отдельно; BFS добавляет кандидата в очередь только при
успешном placement и ограничивает общее число, включая стартовую клетку.
`throttle_friend_count()` также повторяет C округление и `randint0(10)` условие.
Целевых C parity fixtures для friend rules в `src/tests` не найдено (есть только
общие test helpers), поэтому точность этих веток подтверждается пока исходником
и короткими Python regressions, а не прямым native differential test. Пять
прицельных тестов (`same`, `named`, `friends-base`, linked-list order, orchestration)
прошли за 0.24 секунды. Расхождения, обосновывающего очередную правку runtime,
не обнаружено; следующий реальный этап — сделать малый C fixture/harness для
вывода выбранных race/grid/group и RNG state, затем сравнить с моделью. Не запускать
полный C suite или длинные сценарии ради этого.

Приоритет обновлён по повторному требованию о рабочем каркасе (2026-10-17):
точечный audit подтвердил friend-placement, но прямой C differential fixture —
это углубление в редкую механику, его откладываем. MCP `inspect_project` и
`find_model_coverage` показывают, что сценарный проход и отчётный контракт уже
есть; генераторный encounter пока намеренно выставляет одного
`scenario_selected_opponent`, а полная штатная популяция уровня остаётся
отдельной границей. Следующая реализация должна вернуться к базовому проходу
обычного сгенерированного этажа и простому населению/контенту без включения
непроверенных редких marker-механик. Для рутинной проверки запускать только
`one_room`; `one_floor`/`two_floors` не запускались в этой итерации.

Уточнение обычной популяции уровня 2026-10-17: MCP `trace_rule(cave_generate)`
и чтение `src/generate.c:1087-1262` показали, что после builder в общей функции
есть только отдельный quest-monster pass; обычные монстры выставляются
профильными builders через room/vault content markers. В модели marker
materialization существует и тестируется, но scenario config сознательно
держит его выключенным; включение сразу подключит traps, treasure, guards и
vault-specific RNG вместе, а это не является небольшим безопасным шагом.
Поэтому общий счетчик/популяцию в сценарий не добавлял: подтвержденного
универсального количества здесь нет. До отдельной классификации обычных
room-template markers сгенерированные сценарии сохраняют явно помеченного
одного `scenario_selected_opponent`; `one_room` остается коротким routine smoke.
Следующий шаг — выделить один безопасный common room-content marker из уже
загруженных шаблонов и подключить его отдельно, сохранив остальные markers
явными handoff-ами.

Проверка обычного маркера двери `+` — 2026-10-17: короткий MCP-запрос
`trace_rule(place_closed_door)` подтвердил C entry point в `src/gen-util.c`;
вызов из шаблона и lock branch сверены локально (`src/gen-room.c:1183`,
`src/gen-util.c:587-593`, `src/trap.c:706-723`). Модель уже переносила закрытую
дверь, шанс замка 1/4, его мощность 1..7 и RNG вызовы установки trap; нового
runtime поведения добавлять не потребовалось. Добавлен узкий regression
`test_room_template_locked_door_installs_native_lock_and_preserves_rng`, который
проверяет terrain, lock power и порядок/границы RNG. Marker `+` теперь покрыт
интеграционным тестом. Маркеры `8`/`9` не подключать следующим шагом: их
обработчики смешивают расстановку содержимого комнаты, лестницы/объекты,
охрану и RNG. Универсального room-population count по-прежнему нет.

Дальше сохраняем основной сквозной проход и добавляем только обычное содержимое,
для которого можно отделить небольшой подтверждённый путь; до этого — сверить
`one_room` и оставить остальные builder markers явными передачами модели.

Проверка следующего контент-маркера `^` — 2026-10-17: MCP `trace_rule(place_trap)`
подтвердил native entry point `src/trap.c:356-393`. В C невалидный `t_idx`
выбирает ловушку через `pick_trap()` с проверкой допустимого terrain и таблицей
весов, затем рассчитывает power на `trap_level`. Текущий model `trap_factory`
в `angband_model/runtime.py` выбирает равномерно из всех `TRAP`-флагов и не
повторяет эти правила. Поэтому `^` не считается безопасным для включения в
generated scenario; `materialize_markers=False` остаётся правильной политикой
для рабочего сквозного каркаса до переноса native trap allocation. MCP
`find_model_coverage("room trap marker")` подтверждает, что pending/factory
граница уже документирована, но сам поиск — только literal coverage index.
Следующий небольшой полезный runtime этап: вынести trap allocation в точную
source-backed функцию с terrain eligibility, весами и RNG order, только если
эти таблицы и броски можно передать без engine adapter; иначе оставить честный
pending handoff. Routine check остаётся `one_room`.

Выполнена безопасная часть этого этапа: удалён равномерный выбор ловушки из
`TRAP`-флагов в `materialize_generated_level_native()`. Пока точного
terrain-weighted `pick_trap()` allocator нет, `^` теперь возвращает
`generated_layout_marker_pending` с причиной `trap_factory_required`, без
создания вымышленного типа и без RNG draw. Regression
`test_generated_room_trap_without_native_allocator_stays_pending` проверяет
pending ID, причину и неизменность RNG. Targeted test прошёл; `one_room` победил
за штатный прогон, все 8 фаз завершены; `git diff --check` проекта чистый.
Полный набор тестов и generated-floor сценарии пропущены. Следующая работа:
перенести `pick_trap()` через исходные allocation weights/terrain eligibility
либо оставить allocator адаптером, затем продолжать сквозной этап без
включения непроверенных room markers.

Уточнение trap-плана после повторной проверки исходников: weighted chooser
уже есть в `angband_model/trap_data.py` как `choose_native_player_trap()` и
фильтрует TRAP/FLOOR, rarity, min depth, quest, max depth и persistent-level
ограничения; тест `test_native_player_trap_selection_respects_depth_and_quest_trapdoor_rule`
покрывает выбор. Не хватает связи этого выбора с template factory и точного
`TrapKind.power` из поля `visibility` (`src/init.c:1591-1605`), включая native
`randcalc()` RNG-порядок. До переноса этого power расчёта marker `^` остаётся
pending; сам weighted allocation не нужно писать заново.

Аудит `trap.txt` завершён короткой проверкой каталога: все 35 записей, которые
могут быть выбраны как player traps (`TRAP` + положительная rarity), имеют
`visibility`, и `parse_native_random_expression()` разбирает каждое выражение.
Следовательно, оставшаяся блокировка `^` — не недостающая формула или параметр,
а порядок выполнения: `build_room_template()` делает `place_trap()` прямо в
первом проходе клетки; текущая high-level модель собирает deferred markers и
материализует их после построения terrain. Чтобы не сдвинуть RNG относительно
других маркеров, не подключать фабрику постфактум. Приоритет остаётся на
сквозном сценарии; точный inline trap callback отложен до этапа общей
first-pass allocation orchestration.

Отдельно исправлена устаревшая подсказка MCP `inspect_project`: она предлагала
CLI `--seed/--levels`, хотя сценарии теперь запускаются без аргументов и
настраиваются в `scenario_config.py`. MCP server version поднята до 0.1.2,
README и suggested test command синхронизированы с routine `one_room`. Файл
сервера скомпилирован, рекомендованный короткий fixture test прошёл; уже
запущенный процесс MCP продолжит показывать старые подсказки до перезапуска
пользователем. MCP не запускался и не перезапускался агентом.

Проверка сценарного каркаса — 2026-10-17: MCP `inspect_project` остаётся
доступным, но текущий живой процесс всё ещё отдаёт старые команды 0.1.1
(`--seed/--levels` и full unittest); исправленный исходник 0.1.2 уже проверен
локально, серверу нужен пользовательский restart. `scenario_config.py` содержит
`one_room`, `one_floor`, `two_floors`, `twenty_floors`; тест сохранения
состояния между этажами прошёл. Отдельный переходный тест сам создаёт целый
generated floor и зависел в room scheduler дольше routine smoke, поэтому
прерван; его не запускать в обычной короткой проверке. `one_room` прошёл: victory,
8/8 фаз. Дальше двигаться по обычному generated-floor каркасу только короткими
проверяемыми кусками и не включать полный runtime generation test в routine.

Быстрый regression для перехода — 2026-10-17: добавлен
`test_scenario_transition_pipeline_with_stubbed_floor_generator()`. Он
проверяет передачу целевой глубины и параметров генерации, сохранение HP,
commit depth, очистку pending request и порядок пяти стадий transition pipeline,
подменяя только полную генерацию этажа. Тест завершился за 0.045 s; `one_room`
снова победил за обычный короткий прогон, 8/8 фаз; `git diff --check` чистый.
Дорогой end-to-end тест генерации этажа остаётся интеграционной проверкой и не
входит в routine. Следующий структурный участок: проверить оставшиеся входы и
выходы фаз `population → turns → progression → snapshot` небольшими unit
fixtures; не добавлять случайную популяцию без подтверждённого builder rule.


## Каркасный этап: частичный commit при сбое — 2026-10-18

Контракт provenance расширен ролью `partial_after`: она допустима только после
начала commit, на терминальном `failed` lifecycle boundary, с failed commit record
и digest зафиксированного частичного manifest. Readiness разделяет этот случай с
успешным `after`; состояние не объявляется завершённым. Rollback/retry и очистка не
реализуются без подтверждённой политики движка. Проверка модельного каркаса: 411
тестов, `one_room` выигран, 8/8 phases. Дальше: описать внешнее наблюдение abort и
discard/retention состояния, не выполняя их автоматически.


## Каркасный этап: terminal disposition — 2026-10-18

Введено внешнее наблюдение `retained` / `discarded` / `unknown`, привязанное к
терминальному lifecycle boundary `aborted` или `failed`. Readiness/debug показывают
пропущенное или неизвестное состояние; валидная запись не доказывает выполнение
cleanup или retention. Никакие cleanup/rollback/retry не запускаются. Проверка:
411 selected tests (`check_skeleton.sh`) плюс 3 новых focused tests; `one_room` won, 8/8 phases. Следующий шаг: перечислить поля adapter-owned
cleanup request/result и идемпотентность как неподтверждённое evidence.


## Каркасный этап: adapter cleanup evidence — 2026-10-18

Описаны `discard_scratch` request и внешние attempt/result observations. Записи должны
сохранять transfer/boundary, request digest и idempotency key; inspector выявляет
несовпадение identity и конфликтующие завершённые результаты. Он не вызывает adapter
и не подтверждает реальную идемпотентность. Четыре focused tests и 411 selected tests
прошли; smoke `one_room` — won, 8/8 phases. Следующий пункт: согласовать declared
cleanup result с retained/discarded disposition, не меняя состояние.


## Каркасный этап: cleanup/disposition reconciliation — 2026-10-18

Readiness сверяет transfer/boundary identity и completed cleanup disposition с
терминальным disposition record. Противоречие отмечается как blocker; unknown остаётся
unresolved. Согласованные записи не доказывают реальное состояние и не запускают
cleanup. Два новых focused tests, штатный skeleton check (411 tests) и `one_room`
smoke прошли. Следующий шаг: определить форму упорядоченной истории внешних попыток
и допустимые переходы их статусов без выполнения повторов.


## Каркасный этап: порядок cleanup attempts — 2026-10-18

Попытки cleanup нумеруются непрерывным zero-based `sequence_index`. Повторные попытки
требуют caller-declared `allowed_status_transitions`; инспектор сверяет только внешний
лог с этой политикой. Отсутствующая политика остаётся unresolved, недопустимые пары
и индексы фиксируются. Никакая native retry semantics не предполагается. Семь focused
cleanup tests и штатный check (411 tests + one_room smoke, 8/8) прошли. Дальше:
оговорить агрегацию failed/completed попыток без утверждений о runtime.
