param([string]$ExecutablePath)
$ErrorActionPreference = "Stop"
try {
    if (-not $ExecutablePath) {
        $candidate = Get-ChildItem -Path (Join-Path $PSScriptRoot "build*") -Filter OwnGameGui.exe -Recurse -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($candidate) { $ExecutablePath = $candidate.FullName }
    }
    if (-not $ExecutablePath -or -not (Test-Path -LiteralPath $ExecutablePath -PathType Leaf)) {
        throw "Сначала соберите OwnGameGui в Qt Creator. Если сборка вне папки проекта, передайте путь к OwnGameGui.exe параметром -ExecutablePath."
    }
    $ExecutablePath = (Resolve-Path -LiteralPath $ExecutablePath).Path
    # Deploy the matching Qt runtime so the shortcut also works outside Qt Creator.
    $buildDir = Split-Path -Parent $ExecutablePath
    if (-not (Test-Path (Join-Path $buildDir "CMakeCache.txt"))) { $buildDir = Split-Path -Parent $buildDir }
    $cache = Join-Path $buildDir "CMakeCache.txt"
    $deploy = $null
    if (Test-Path $cache) {
        $qtEntry = Get-Content -LiteralPath $cache | Where-Object { $_ -match '^Qt6_DIR:[^=]+=(.+)$' } | Select-Object -First 1
        if ($qtEntry -and $qtEntry -match '=(.+)$') {
            $candidate = Join-Path $Matches[1] "../../../bin/windeployqt.exe"
            if (Test-Path -LiteralPath $candidate) { $deploy = (Resolve-Path -LiteralPath $candidate).Path }
        }
    }
    if ($deploy) {
        Write-Host "Копирую библиотеки Qt для запуска с рабочего стола..."
        & $deploy --qmldir (Join-Path $PSScriptRoot "gui\qml") $ExecutablePath
        if ($LASTEXITCODE -ne 0) { throw "windeployqt завершился с ошибкой. Ярлык не создан." }
    }
    $icon = Join-Path $PSScriptRoot "gui\assets\cat.ico"
    $desktop = [Environment]::GetFolderPath("Desktop")
    $shell = New-Object -ComObject WScript.Shell
    $shortcut = $shell.CreateShortcut((Join-Path $desktop "Своя игра.lnk"))
    $shortcut.TargetPath = $ExecutablePath
    $shortcut.WorkingDirectory = Split-Path -Parent $ExecutablePath
    $shortcut.IconLocation = "$icon,0"
    $shortcut.Description = "Интеллектуальный тренажёр Своя игра"
    $shortcut.Save()
    Write-Host "Ярлык с котом создан на рабочем столе."
    Write-Host "Если вне Qt Creator не запускается, выполните windeployqt, как описано в GUI.md."
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
