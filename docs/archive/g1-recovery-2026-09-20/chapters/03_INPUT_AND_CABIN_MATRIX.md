<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Контекстное управление и полный салон

## 1. Зафиксированная грамматика

| Контекст | Мышь XY | ПКМ | ЛКМ | Колесо |
|---|---|---|---|---|
| Вождение, нет захвата | Руль | Только допустимый физический target | Только target с momentary capability | E → W → Q; иначе capability target |
| Space удерживается | Взгляд, bounded raycast в салон | Захват подходящего target | Нажатие momentary target по принятой строке | Target с `bWheelInGaze`, если нет педального владельца |
| Rotary захвачен | Не крутит руль; движение только если rotary declares drag | Держать один орган | Удержание для fine/secondary только если описано | Крутить выбранный параметр |
| КПП захвачена | Driver right/forward → H-gate | Держать рычаг | Для перемещения не обязательна | Existing ShiftBy, если остаётся, через тот же validator |
| Ручник захвачен | Короткий аналоговый ход | Держать рычаг | Для перемещения не обязательна | Настройка скорости отпускания, если принята; не второй brake input |
| Меню / потеря фокуса | UI | Не gameplay | UI | Не gameplay |

**Q** выжимает сцепление и подготавливает руку над КПП. Q сам не включает grip, не выбирает передачу и не отнимает руль. Клавиша 3 остаётся. При реальном захвате рычага мышь перестаёт рулить; удерживаемый угол руля сохраняется без автопилота. Остальная рука визуально держит руль; она не выбирает траекторию.

**Q + wheel:** release time, вверх быстрее, вниз медленнее. **E + wheel:** throttle target, обязателен заново на каждом новом старте. **W + wheel:** brake target. W+E допустимы. Приоритет E > W > Q перед салоном сохраняется по page 16744449. Если для radio нужна иная priority, это явное DEC-01, а не скрытая правка.

В режиме взгляда радио можно вращать одним колесом при валидном наведении. ЛКМ + wheel поддерживает typed alternate/fine режим, если он выбран для конкретного регулятора. Не делать ЛКМ случайным тумблером питания на каждом начале вращения. Отпускание, выход из hit-volume и focus-loss должны различаться: capture держит исходную ручку, gaze-only теряет ownership при уходе взгляда.

## 2. Правила арбитра

- Один mouse XY sample может менять ровно один channel: UI / gaze / manipulation / steering.
- Один wheel sample потребляет один owner; событие получает receipt с frame/serial/owner/reason.
- Quick recall выбирает target, не действует и не телепортирует физический рычаг.
- Pick ограничен reach distance, cabin layer и occlusion; не `GetAllActorsOfClass` каждый Tick.
- Control descriptor задаёт capabilities, пределы, detents, persistence, feedback. Неподдерживаемое действие имеет безопасный no-op с понятным hover hint.
- Escape/focus loss отменяет незавершённое включение передачи, останавливает transient presses и чистит остаточные mouse deltas. Уже реально включённую передачу не стирает без механической причины.
- `Q` не перехватывает активный ручник/радио каждый кадр: staging применяется на edge/разрешённом idle; спорное overlap фиксируется INP-04 тестом.
- Не добавлять dwell 1.5 секунды, если это не требует актуальный runtime contract. Старое число из чата не превосходит свежий input authority.
- Клавиатура RU/EN проверяется по физическим Q/W/E и надписям; буква Й не должна становиться отдельной механикой.

## 3. H-gate

| Ряд | Левый | Средний | Правый |
|---|---|---|---|
| Вперёд | 1 | 3 | 5 |
| Поперечный | N | N | N |
| Назад | 2 | 4 | R |

Перемещение через N обязательно; широкий рабочий коридор, фиксаторы, никакого автоисправления 5→R в 5→4. Направление кулисы, actual gear, expected RPM, звук и mesh читаются совместно. Release/cancel не создаёт отложенное включение. Clutchless shift возможен при допустимом совпадении, а не запрещён одним `if !Q`.

## 4. Реестр элементов — правило полноты

Ниже принят **полный аудит fitted controls**. `G1` — обязательный scope пакета; `AUDIT` — проверить наличие/принять либо явно исключить в DEC-02; `POST` — будущая система, не блокирует G1. Нельзя уменьшить знаменатель, просто убрав неудобную строку. Если функциональный элемент виден в принятом салоне G1, он должен работать или иметь согласованную маркировку неподвижной конструкции.

Для каждой строки обязательны: `ControlId`, source/reference, authored part/socket, hit volume, reach, interaction kind, min/max/default, spring/detents, effective state owner, animation, audio event, cancellation, save/reset, damage channel, QA evidence. Отсутствующие координаты берутся измерением, а не догадкой.

| ControlId | Элемент | Input / state | Связь с поведением и проверка | Scope |
|---|---|---|---|---|
| CKP-DRV-STEER | Руль | Mouse steering; один authored pivot | Front wheel right, mesh right, руки следуют; без зеркального логотипа | G1 |
| CKP-DRV-GEAR | КПП | 3/Q recall; RMB+XY | 1..5/N/R, реальный neutral и валидатор; рычаг на исходном месте | G1 |
| CKP-DRV-HBRAKE | Ручник | 4 recall; RMB+XY | Стоя latch; на ходу hydraulic analog/return; 25/50/100% различаются | G1 |
| CKP-PED-CLUTCH | Сцепление | Q; Q+wheel release time | Непрерывная педаль, coupling, interrupt release | G1 |
| CKP-PED-BRAKE | Тормоз | W; W+wheel | Фактическое давление, стоп-сигналы, возможна блокировка без ABS | G1 |
| CKP-PED-THROTTLE | Газ | E; E+wheel | Новый launch требует re-dose; actual travel, не engine RPM | G1 |
| CKP-ENG-IGN | Ключ/зажигание | Typed keyed detents, momentary Start | Off/ignition/cranking/running/stalled; release starter, restart без reset | G1 |
| CKP-ENG-CHOKE | Подсос | Pull/slider | Только если adopted cold-start; не декоративная working knob | AUDIT |
| CKP-SIG-TURN | Поворотники | 1; wheel down L/up R | Состояние рычага, лампы, наружные огни, relay audio | G1 |
| CKP-SIG-HORN | Сигнал | 2; LMB momentary, DEC-01 | Звук только пока нажато; фокус очищает | G1 |
| CKP-SIG-HAZARD | Аварийка | Typed toggle | Обе стороны, приоритет над turn; save policy | G1 |
| CKP-SIG-HIBEAM | Дальний/мигание | Detent + momentary capability | Реальные beams и telltale, состояние не меняется от grip | G1 |
| CKP-LGT-MAIN | Габариты/ближний | Detents Off/Position/Dipped | Свет, подсветка, electrical health | G1 |
| CKP-LGT-CENTER-DIR | Центральная фара | Mechanical steering link | Yaw следует actual steering; отдельный read-only slot | G1 если adopted hero optics |
| CKP-LGT-INT | Салонный свет | Toggle | Cabin visibility, без засветки меню/приборов | G1 |
| CKP-LGT-DIM | Яркость приборов | Rotary wheel | Только яркость, не сигнальные значения | G1 |
| CKP-LGT-REV | Задний ход | Mechanical actual gear | Не от RequestedGear; обе lamps при исправности | G1 |
| CKP-VIS-WIPER | Дворники | Off/Low/High detents | Ход щёток, park position, звук, очищенная область | G1 |
| CKP-VIS-WASH | Омыватель | Momentary hold | Ограниченный spray/audio и реакция стекла | G1 |
| CKP-VIS-RMIR | Салонное зеркало | Read-only, preset | Мир сзади отражается в движении; budget capture | G1 |
| CKP-VIS-SMIR-L | Левое зеркало | Read-only, preset | Видимая задняя боковая зона без зеркального повторного flip | G1 |
| CKP-VIS-SMIR-R | Правое зеркало | Read-only, preset | Аналогичная отдельная камера/proxy | G1 |
| CKP-VIS-WIN-L | Левое стекло | Rotary winder wheel/drag | Open fraction, звук улицы, collision policy | G1 |
| CKP-VIS-WIN-R | Правое стекло | Rotary winder wheel/drag | Без перемещения механизма в удобное место | G1 |
| CKP-VIS-VENT | Форточка | Latch + hinge | Если fitted: открытие и acoustic leak | AUDIT |
| CKP-VIS-VISOR | Козырёк | Toggle/hinge | Крайние положения, occlusion, не пересекает head path | AUDIT |
| CKP-HVAC-HEAT | Отопитель | Stepped switch | Вентилятор/индикация; air-cooled engine не получает выдуманный радиатор | G1 |
| CKP-HVAC-FRESH | Заслонка | Pull/slider | Направление/доля воздуха, видимый ход | G1 |
| CKP-HVAC-VENT | Распределение | Slider/detent | Стекло/салон, согласованная индикация | G1 |
| CKP-HVAC-BLOW | Вентилятор | Rotary 0..N | Реальная громкость/режим, без дублирования HEAT если один орган | G1 |
| CKP-HVAC-OUT | Дефлекторы | Reachable drag | Жалюзи, направление; если нефункциональная решётка — explicit reject | AUDIT |
| CKP-DOOR-IN-L | Ручка водителя | Lever momentary | Latch/open при разрешённых условиях; self-collision исключить | G1 |
| CKP-DOOR-IN-R | Пассажирская ручка | Lever | Состояние двери и звук | G1 |
| CKP-DOOR-LOCK-L | Замок водителя | Toggle | Блокирует соответствующее открытие | G1 |
| CKP-DOOR-LOCK-R | Замок пассажира | Toggle | Не блокирует весь input | G1 |
| CKP-TAXI-DOORS | Управление пассажирскими дверями | Physical switch | Остановка обязательна для pickup/dropoff; aperture согласован с donor/hero | G1 |
| CKP-SEAT-ADJ | Регулировка сиденья | Lever/slider or fixed accepted setup | Не менять camera origin произвольно; reach extremes | AUDIT |
| CKP-SEAT-FOLD | Спинка | Latch/hinge | Fitted mechanism, без прохода камеры через геометрию | AUDIT |
| CKP-GLV-LATCH | Бардачок | Toggle latch/hinge | Open/close, persistence, звук | G1 |
| CKP-ASH | Пепельница | Slider | Открывается/закрывается; курение не добавлять | G1 |
| CKP-LIGHTER | Прикуриватель/розетка | Momentary / socket | Принятый click/heat-ready/device state; не новая мини-игра | AUDIT |
| CKP-RAD-PWRVOL | Питание/громкость радио | Rotary+typed press | Gaze wheel регулирует; питание только оговорённым press/detent | G1 |
| CKP-RAD-TUNE | Настройка станции | Rotary wheel | Частота, static→station crossfade; никаких сетевых потоков | G1 |
| CKP-RAD-BAND | Диапазон/пресеты | Detent/button | Переключение воспроизводимого approved catalog | G1 |
| CKP-INS-SPEED | Спидометр | Read-only telemetry | Скорость от actual chassis; единица km/h | G1 |
| CKP-INS-TACH | Тахометр | Read-only engine RPM | Разметка/стрелка до 8500 и зоны отказа, без throttle-as-RPM | G1 |
| CKP-INS-FUEL | Топливо | Read-only | Fuel state, low fuel warning | G1 |
| CKP-INS-OILT | Температура масла/двигателя | Read-only | Подпись соответствует air-cooled V8 | G1 |
| CKP-INS-ODO | Одометр/trip | Read-only + fitted reset | От distance accumulator; reset только trip | G1 |
| CKP-INS-CLOCK | Часы | Read-only/typed adjustment | Game time согласно принятому workday contract | G1 |
| CKP-INS-WARN | Oil/charge/temp/handbrake lamps | Read-only array | Каждая от своего состояния; bulb-check если adopted | G1 |
| CKP-TAXI-METER | Таксометр | Physical workflow controls | Fare state/money, не дублирует ledger | G1 |
| CKP-TAXI-NAV | Заказ/адрес | Physical work surface | Route/active destination, без полной Neural соцсети | G1 |
| CKP-TAXI-PAY | Сенсор оплаты | Context event | Awaiting payment, exactly-once credit, отказ/повтор | G1 |
| CKP-TAXI-RECEIPT | Чек | Physical handoff | Принятая tip/receipt логика; не бесконечный tip exploit | G1 |
| CKP-SVC-FUEL | Топливный/service прибор | Read-only/action per existing system | Рабочий service handoff; не дублировать SYS | G1 |
| CKP-MAG-STATE | Магнит/attitude | System-specific | В G1 выключен явно, не обещает готовую верхнюю механику | POST |
| CKP-REAR-SERVICE | Место проводника | Future surface | Сохранить clearance; full character work после G1 | POST |

## 5. Анимация каждого органа

Источник transform — authoring pose. На старте сохранить `RestTransform`; кадр считается как Rest × local delta, не накоплением world rotation. Lever axis и пределы задаются явно. Движущиеся детали не являются новой физической системой. Руки используют reach/grip/operate/release и interruption; пальцы не проходят через ручку на основном ракурсе. Анимация кнопки начинается от state event, а не независимо от actual action.

Для каждого G1 control проверить: idle → focus → acquire → operate min/mid/max → release → повтор → отмена → menu → load. Один shared automation fixture допустим, но таблица результатов содержит отдельную строку каждого ControlId. Общий test count без этой таблицы недостаточен.
