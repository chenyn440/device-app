Param(
    [string]$AppVersion = "",
    [string]$BuildDir = "build-release-windows",
    [string]$QtCMakeDir = $env:Qt6_DIR
)

$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($AppVersion)) {
    $cmakeFile = Join-Path $RootDir "CMakeLists.txt"
    $projectLine = Select-String -Path $cmakeFile -Pattern 'project\(DeviceApp VERSION ([0-9.]+)'
    if ($projectLine.Matches.Count -gt 0) {
        $AppVersion = $projectLine.Matches[0].Groups[1].Value
    }
}

if ([string]::IsNullOrWhiteSpace($AppVersion)) {
    throw "Unable to determine app version."
}

if ([string]::IsNullOrWhiteSpace($QtCMakeDir)) {
    throw "Qt6_DIR is required. Pass -QtCMakeDir or set Qt6_DIR."
}

$BuildPath = Join-Path $RootDir $BuildDir
cmake -S $RootDir -B $BuildPath -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=$QtCMakeDir
cmake --build $BuildPath --config Release

$ExePath = Join-Path $BuildPath "Release\device-app.exe"
if (-not (Test-Path $ExePath)) {
    $ExePath = Join-Path $BuildPath "device-app.exe"
}
if (-not (Test-Path $ExePath)) {
    throw "Expected executable not found."
}

$QtRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $QtCMakeDir))
$WinDeployQt = Join-Path $QtRoot "bin\windeployqt.exe"
if (-not (Test-Path $WinDeployQt)) {
    throw "windeployqt.exe not found at $WinDeployQt"
}

& $WinDeployQt --release --compiler-runtime $ExePath

$DistDir = Join-Path $RootDir "dist"
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

$PackageDir = Join-Path $BuildPath "package"
if (Test-Path $PackageDir) {
    Remove-Item -Recurse -Force $PackageDir
}
New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null
Copy-Item -Recurse -Force (Split-Path -Parent $ExePath)\* $PackageDir

$ArchiveName = "device-app-$AppVersion-windows.zip"
$ArchivePath = Join-Path $DistDir $ArchiveName
if (Test-Path $ArchivePath) {
    Remove-Item -Force $ArchivePath
}

Compress-Archive -Path (Join-Path $PackageDir '*') -DestinationPath $ArchivePath
Write-Host "Created: $ArchivePath"
