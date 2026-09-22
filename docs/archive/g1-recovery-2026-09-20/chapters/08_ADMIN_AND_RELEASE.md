<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Административка, Git и релиз

## 1. Карта существующих владельцев

| Работа | Существующие Jira кандидаты | Правило |
|---|---|---|
| Активный продукт | CD-519 | G1 — milestone/child outcome, не второй проект |
| Broader freeze | CD-848 | Не закрывать от shareholder demo |
| Модель/салон placeholder | CD-855 | Дополнить точными task links после duplicate search |
| Historical pre-model | CD-856 DONE | Не отменять историю и не использовать как доказательство текущего G1 |
| Input/pedals/gear | CD-609,611,643,644,645 | Связать INP/VEH с существующими owners |
| Steering/handling/handbrake | CD-648,649,651,653,659 | Manual gates остаются открыты до владельца |
| Health/save | CD-740,670,801 | Переносить новые criteria, не создавать второй save pipeline |
| City core | CD-797 | Historical Done не равен 100% нынешнего города |
| Road/signs/interchanges | CD-563,568,564,573,589,678..681 | Проверить карточки и не создавать дубли |
| Presentation kits | CD-714..719 | Reuse существующих владельцев |
| QA/build | CD-541,559,574,586 | Exact artifact evidence |
| Legacy salvage | CD-539,558 | Read-only source inventory, current authority links |

Текущие Jira статусы прочитаны лишь для выборки в sources/Jira_snapshot.json. Для остальных IDs связь из Confluence — **кандидат на reconciliation**, не выдуманное подтверждение статуса/AC.

## 2. Иерархия

`Product CD-519 → Milestone G1 Shareholder → Player Result → Technical task → Acceptance evidence`.
Локальные `ADM-/VEH-/INP-/CAB-/...` — IDs этого пакета, не созданные Jira keys. В tasks.json `jira_candidates` содержит существующие кандидаты. При импорте сначала exact key/summary/labels/dedup search; либо добавить checklist/subtask к найденной карточке, либо создать отсутствующую с traceability. Массового blind CSV import нет.

Нужные поля задачи: результат игрока; dependencies; owner module; source authority; acceptance criteria; allowed files; explicit out-of-scope; test IDs; proof link; manual gate; version; blocker reason. Statutes отражают evidence, а не желаемую дату.

## 3. Source authority reconciliation

После разрешения реализации ADM-02 обновляет зеркало документации с точечными supersession notes. Не переписывать историю PASS прошлых SHA. Старый текст FGear/VDS, 530i, left-front reverse, универсальный START→ATTENTION→GO, fixed old clutch range не остаётся незаметной current authority. В каждой противоречащей странице короткая ссылка на актуальный contract и область отмены.

Confluence — стабильные contracts и gates. Jira — задачи/blockers. Git — исполнимые профили, code/tests, docs/ADR. Build logs — доказательство поведения. Чат — новые решения; в тот же проход перенести их в source of truth, если запись разрешена scope.

## 4. Git без потери пользовательской работы

В начале каждой сессии read-only: repository root, branch, HEAD, remote, upstream, status, diff --stat, untracked inventory, recent log, active PR. Не checkout/reset/clean/stash поверх незнакомой работы. Если нужна isolation — worktree от явно установленной базы; сохранить unrelated changes untouched.

PR #5 зависит от integration/CD848-CD855; PR #3/#4 имеют отдельную историю. Не склеивать stack force-push ради красивой истории. Сначала установить merge topology, определить уже включённые commits, потом предложить минимальную интеграцию. Новый пакет плана не требует merge runtime.

В commits только allowlisted файлы текущей задачи. Не `git add .` из огромного dirty worktree. Commit message с owner key и TaskId. Model binaries + licensing metadata атомарно, LFS по текущей политике, не push нелицензированные sources. Нельзя назначать непроверенную модель approved от того что она уже скачана.

## 5. Build pipeline risk

Прочитан `Scripts/fast-delivery.ps1`: он требует clean status, умеет CPP_BINARY_PATCH/INCREMENTAL_RECOOK, переключает OwnerIteration, имеет операции удаления `_next/_old`, при которых можно потерять предыдущий accepted artifact. Его нельзя запускать автоматически в этой planning-задаче. При будущей реализации обеспечить owner-approved retention policy или использовать новый immutable `BuildId` output path. Не удалять пользовательские старые билды без разрешения.

`BUILD_SHA.txt` — полезная метка, но не provenance всего пакета. Release manifest должен содержать source SHA, engine CL, profile version, map list, asset BOM/license manifest, exe/cooked hashes, configuration, commands, test results. После binary patch проверять совместимость cooked reflection/config/assets; изменение headers/.uproject/assets/ini требует соответствующей rebuild/cook strategy.

## 6. Candidate promotion

1. Freeze candidate tree; не брать параллельно меняющийся worktree.
2. Focused+full automation и code-health; независимый review изменённого scope.
3. Build/cook/package в отдельный immutable path; сохранить commands/logs.
4. Запустить именно этот artifact в D3D, проверить inputs/render/audio/spawn.
5. Regression scenarios/soak; зафиксировать hashes и evidence.
6. Собрать RELEASE_NOTES: известные ограничения, G1 scope, controls, save compatibility, rollback path.
7. Подготовить shortcut на exact candidate; переключить только в разрешённой release-задаче, без удаления предыдущего accepted build.
8. Owner human gate с checklist. PASS → tag/milestone snapshot/STOP. FAIL → reopen конкретных tasks и сохранить предыдущий baseline.

Code review нужен перед merge и завершением крупной задачи. Если независимый reviewer недоступен/лимит — записать blocker, PR draft; самопроверку не переименовывать в independent approval. Не расходовать квоту повторными одинаковыми запросами reviewer.

## 7. Экономное исполнение Sol 5.6

Native execution, одна активная задача; допускаются пакетные независимые reads. Не запускать дополнительные модели/агентов автоматически. Полный suite только на нужных integration gates, focused tests после микрошагов. Повторный тест — для конкретного риска, а не ритуала.

Checkpoint после каждой завершённой задачи: TaskId/status, SourceSha, изменённые paths, что реально проверено, где logs, открытый blocker, следующая задача. Короткий ответ таблицей: task/result/evidence/human state. Если лимит закончился — оставить checkpoint и незакрытые статусы; не имитировать завершение.

Пример checkpoint schema:

```json
{
  "milestone": "G1_SHAREHOLDER",
  "active_task": "VEH-01",
  "task_state": "IN_PROGRESS",
  "source_sha": "8a70dfa5f4120fce63ab6f0cfc9235128987da77",
  "build_id": null,
  "checks_run": [],
  "blockers": ["Runtime immobilization cause not yet reproduced"],
  "next_action": "Capture physical wheel contacts and input ownership in exact packaged candidate",
  "human_gate": "PENDING"
}
```

Это стартовый пример, не отчёт о проделанной реализации.
