<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Ассеты, исходные папки и безопасная замена модели

## 1. Что действительно найдено

| Путь | Содержимое, проверенное чтением | Следующее действие в плане |
|---|---|---|
| `D:\Новая папка\дорожные знаки` | `8e02cdfbb26244f495dc2d64c7f6b557.glb`; `modular_russian_road_signs_pack_urban.glb`; `russian_road_sign_decals_pack.glb`; `speed_bumpfreelow.glb` | Inventory, glTF metadata, source URL/license, dimensions, materials, пригодность под sign grammar |
| `D:\Новая папка\заборы` | `po_2_fence.glb` | Проверить silhouette и layout против P-6V direction; PO-2 не автоматически идентичен P-6V |
| `E:\Development\дедкорн\DESKTOP_SNAPSHOT\DEADRACE` | `New folder`, `tatra`, `tatra-603-2-b5-marathon`, `ZAGLUSHKA`, `ZAGLUSHKA_TATRA_613`, `мир_пример`, `рефы`, PNG, `рефы.zip` | Read-only inventory и salvage map; никаких массовых переносов |
| `...\мир_пример` | Нижний/второй слой схем, `отчерк.docx`, заборы, потолок, перспективы, телефон | Current/legacy/proposed tagging; документы не становятся кодом автоматически |
| `...\tatra-603-2-b5-marathon\content\cars\tmm_tatra_603` | `.kn5` + LOD b/c/d, `collider.kn5`, `data.acd`, `.ksanim`, `.bank`, skins, ReadMe.pdf, ui_car.json | Rights gate прежде conversion/import |

## 2. Marathon: установленное ограничение

Прочитан локальный **ReadMe.pdf, страница 2**. Он прямо требует разрешение авторов для изменения, конвертации, повторного размещения и использования любой части physics/3D/textures/sound. Это не ситуация «лицензию не нашли». В том же тексте: базовая 3D-модель куплена на коммерческом портале; интерьер/шины/дополнения сделаны командой; звук сделал ACFan; присутствуют acknowledgements Kunos. `ui_car.json`: TM-Modding, version 1.2, URL `https://tm-modding.eu`.

Права на каждый слой могут различаться. Бесплатность Assetto Corsa mod не даёт автоматически право включать его в коммерческий Unreal-билд. Перекраска, уменьшение полигонов, другой формат или замена шильдика не решают отсутствие разрешения.

### Путь A — получить permission/source pack

Планируемый запрос: 1) commercial use in PINK CAB, 2) Unreal conversion, 3) modification/optimization/LOD/material replacement, 4) cooked redistribution to testers/shareholders/Steam, 5) trailer/screenshots, 6) exact scope meshes/interior/textures/animations/audio, 7) rights to underlying purchased body and third-party contributions, 8) credit wording, 9) cost/term/revocation terms, 10) availability of Blender/FBX/glTF and WAV stems. Сохранить письмо/лицензию, дату, автора, разрешённые файлы и source hashes. **Письмо в этой работе не отправляется.** Отдельное разрешение на отправку потребуется, если это не поручено напрямую.

### Путь B — разрешённая самостоятельная геометрия

Если permission не получено, G1 использует текущий donor только после его собственной license audit, либо собственный low-poly placeholder. Это не повод вернуть пустую кабину: та же матрица, те же pivots, hands, glass, gauges и полная функциональность. Исторические формы Tatra исследовать по допустимым референсам; не извлекать/обходить защиту `.kn5/.bank/.acd`.

### Путь C — покупка базового body

Только отдельное решение владельца о расходах. Одна покупка body не лицензирует интерьер и звук мод-команды. Бюджет по умолчанию 0; покупку не включать обязательной зависимостью G1.

## 3. Интеграция без боли после rights gate

1. Ничего не менять в исходной папке. Завести AssetId и hash original.
2. Получить разрешённый exchange source. Изолированная working copy; conversion log.
3. Измерить габариты, wheel centers, wheelbase/track, authored steering/gear/handbrake pivots, seat/head/camera, door hinges и clearances.
4. Единицы `1 UU=1 cm`; +X forward/+Y right/+Z up; positive determinant, никаких зеркальных transforms. Axis conversion один раз в import profile.
5. Render body и cockpit отделены от collision/vehicle dynamics. Engine physics из AC не импортируется. Колёса донора сохраняются, если владелец не принял замену.
6. Назначить semantic IDs, не искать gameplay часть по material index. Старые имена материалов можно использовать только один раз в audited import mapping.
7. Тахометр/скорость/топливо/oil-temp needles отдельно; RestTransform и axis; sweep endpoint tests. Стекло прозрачное снаружи/внутри и читаемые приборы ночью.
8. Grip/reach/socket table; фиксировать ручник и КПП на исходных authored местах. Недостижимость решается rig/reach/interaction volumes, а не переносом рукоятки через салон.
9. Центр колёс физики и render согласовать profile; не uniform-scale всей модели под chassis. Hero 603 и временный 613 — разные visual profiles.
10. Collision hulls body; doors collision policy; салонные interactive hit volumes отдельны; wheel suspension traces не бьют по собственным компонентам.
11. LOD/material/texture budgets по profile. Проверить transparency sorting, shadows, Nanite eligibility по типу asset; не включать всё автоматически.
12. Запустить visual-slot, input, vehicle, save regression на обеих моделях; поменять профиль туда-обратно без изменения логики и с одинаковыми input/torque/route traces.

Если asset swap изменил массу/радиус без явного physical profile version — FAIL. Если заменили model и понадобилось менять input mappings — FAIL архитектурного контракта.

## 4. Формат передачи ассетов владельцем

Предлагаемые deliverable folders **для новых пакетов**, не команда переорганизовать текущие:

`AssetId/Source/`, `AssetId/Textures/`, `AssetId/License/`, `AssetId/Preview/`, `AssetId/asset.json`.

Минимальный `asset.json`:

```json
{
  "asset_id": "MODEL_TRAFFIC_SEDAN_A",
  "source_filename": "original.glb",
  "source_url": "SOURCE_TO_BE_VERIFIED_BEFORE_ADMISSION",
  "author": "UNKNOWN_UNTIL_INVENTORY",
  "rights_state": "UNVERIFIED",
  "license_evidence": [],
  "intended_use": "G1 traffic presentation",
  "units": "cm",
  "forward_axis": "+X",
  "up_axis": "+Z",
  "source_sha256": null,
  "admission_state": "BLOCKED_PENDING_EVIDENCE"
}
```

Это **валидный пример записи неизвестного ассета**, не production-ready license. Validator обязан отклонять UNVERIFIED/unknown/null hash перед cooked distribution. Нельзя подставить выдуманный URL, автора или лицензию для зелёной галочки.

Модели: предпочтительно Blender source + FBX/glTF; отдельные moving parts/rig, transforms, collision notes, LODs если есть. Текстуры: original PNG/TGA, color-space, alpha usage, tiling/atlas layout; roughness не перепутать с gloss. Звук: dry masters, loops/cue sheet, права на sample/исполнение/композицию. Шрифты, логотипы, голос/радиомузыка тоже входят в release BOM.

## 5. Как получить точный shopping/production list

`10_ASSET_REGISTER.md` перечисляет конечные семейства и минимальные варианты для G1 и последующих этапов. Это план потребности, **не утверждение что файлы уже есть**. `ASSET-01` связывает каждую строку с FOUND / MISSING / CREATE_OWN / RIGHTS_BLOCKED / REJECTED. Один найденный pack может закрывать несколько строк, только если состав, права и техническая приёмка доказаны.

Количество вариантов — production proposal, не требование покупать каждый отдельно. Full product здесь = текущий offline FIRST EURO плюс явно помеченные future families; для ещё не придуманного L3/online нельзя честно обещать окончательный список каждого уникального файла. Там ставится bounded discovery task, а не выдуманный «точный состав».
