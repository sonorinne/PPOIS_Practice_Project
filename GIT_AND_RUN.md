# Перенос GUI в командный репозиторий и запуск

`commit` сохраняет изменения в локальном репозитории; `push` отправляет коммиты на GitHub.
Распакованный ZIP не содержит истории Git. Удобнее создать новый клон, не копируя старую папку build.
Реальное имя ветки команды: **feature-core_logic** (после core — подчёркивание).

## Один новый коммит от своего имени

Установите Git for Windows и откройте Git Bash. Примеры используют отдельную новую папку:

```bash
cd /c/Projects
git clone https://github.com/sonorinne/PPOIS_Practice_Project.git OwnGameGit
cd OwnGameGit
git config user.name "sonorinne"
# Замените значение ниже на свой подтверждённый email из GitHub Settings → Emails.
git config user.email "YOUR_VERIFIED_EMAIL"
git remote add upstream https://github.com/OOOnikproOOO/PPOIS_Practice_Project.git
git fetch --all
git switch -c feature-gui-integration upstream/feature-core_logic
git merge --squash origin/feature-gui
git status
git commit -m "Add Qt GUI, single-player modes and retro TV presentation"
```

`origin` — ваша копия sonorinne, `upstream` — исходный репозиторий команды.
Локальная ветка создаётся именно от ядра команды. Squash переносит готовый GUI одним коммитом,
автором которого будете вы с указанным email. Он не переписывает историю ядра.
Не копируйте поверх клона файлы из старого ZIP: все готовые изменения берутся с GitHub.
Если после скачивания ZIP вы самостоятельно меняли исходники, сначала отдельно сохраните эти правки.

## Отправка и Pull Request

Рекомендуемый путь, не требующий права записи в репозиторий команды:

```bash
git push -u origin feature-gui-integration
```

На GitHub откройте свою ветку → Contribute / Open pull request (либо Compare & pull request).
Выберите:

- base repository: `OOOnikproOOO/PPOIS_Practice_Project`;
- base: `feature-core_logic`;
- head repository: `sonorinne/PPOIS_Practice_Project`;
- compare: `feature-gui-integration`.

Create pull request публикует запрос от вашего GitHub-аккаунта. Слияние выполняется после ревью команды.

Если команда дала вашему аккаунту право записи и нужно создать **feature-gui прямо у команды**:

```bash
git push -u upstream feature-gui-integration:feature-gui
```

Затем создайте PR в исходном репозитории: base `feature-core_logic`, compare `feature-gui`.
403 означает отсутствие права записи: используйте fork + PR либо попросите владельца дать доступ.
Если feature-gui у команды уже существует, согласуйте её использование, не перезаписывайте ветку.

## Запуск без Qt Creator

Qt Creator — редактор/среда разработки. Графическому приложению нужны библиотеки Qt,
но Qt Creator или установленный Qt на компьютере конечного пользователя не требуются,
если необходимые библиотеки скопированы рядом с exe.

1. В Qt Creator переключите сборку с Debug на Release и соберите OwnGameGui.
2. Запустите `CreateShortcut.cmd`: скрипт ищет exe в build*, находит windeployqt по CMakeCache,
   копирует библиотеки и создаёт ярлык с котом.
3. Для переноса на другой компьютер скопируйте **всю папку exe**, включая DLL, плагины,
   папки QML и assets. Один exe отдельно не переносите.
4. Если скрипт не нашёл Qt, выполните windeployqt вручную из терминала соответствующего Qt kit:

```powershell
windeployqt --qmldir "C:\Projects\OwnGameGit\gui\qml" "C:\path\to\OwnGameGui.exe"
```

Путь к exe возьмите в Qt Creator → Проекты → Настройки запуска → Программа.
При запуске из обычного PowerShell может потребоваться полный путь к windeployqt.exe
в папке bin вашего Qt kit. Пример для установленного Qt 6.12 MinGW:

```powershell
& "C:\Qt\6.12.0\mingw_64\bin\windeployqt.exe" --qmldir "C:\Projects\OwnGameGit\gui\qml" "C:\path\to\OwnGameGui.exe"
```

## Совсем без Qt — консольная версия

Графический интерфейс написан на Qt и не запускается без его библиотек.
Исходное консольное приложение и тесты можно собрать без Qt:

```bash
cmake -S . -B build-console -DOWNGAME_BUILD_GUI=OFF
cmake --build build-console --config Release
ctest --test-dir build-console -C Release --output-on-failure
```

Для сборки нужны CMake, компилятор C++17 и доступ к загрузке GoogleTest.
OwnGame.exe находится в build-console или build-console/Release, в зависимости от генератора.
Запускайте его из терминала; графическое оформление в этой версии отсутствует.

## Что хранить в Git

Коммитятся C++/QML, CMake, тесты, workflow, документация и необходимые исходные ресурсы:
assets, шрифты с лицензиями, иконки, звуки и готовый .qsb для выпуклого экрана.
Не коммитятся build*, exe, DLL, локальные БД, настройки Qt Creator и отчёты покрытия — они в .gitignore.
Учтите, что уже отслеживаемый Git файл не перестаёт отслеживаться от добавления в .gitignore.

Перед отправкой: собрать с OWNGAME_BUILD_GUI=ON, выполнить ctest и проверить git status.
CI уже содержит проверку ядра Windows и GUI Linux. Дополнительная настройка Qt в репозитории не нужна.
Согласуйте с командой целевую ветку PR. Защиту main и обязательные CI-проверки настраивает владелец
репозитория через Settings → Rules / Branches.
