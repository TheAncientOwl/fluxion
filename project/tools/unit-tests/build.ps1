<#
.SYNOPSIS
    Builds and runs unit test targets using CMake and CTest.
.DESCRIPTION
    Usage: .\run_test.ps1 [-b] [-r] [-d <build_dir>] <target_name_or_path> [extra_gtest_flags...]
#>

[CmdletBinding()]
param(
    [Alias("b")]
    [switch]$Build,

    [Alias("r")]
    [switch]$Run,

    [Alias("d")]
    [string]$BuildDir = "build",

    [Alias("h")]
    [switch]$Help,

    [Parameter(Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$Params
)

$ErrorActionPreference = "Stop"

function Show-Usage {
    Write-Host "Usage: .\run_test.ps1 [-b] [-r] [-d <build_dir>] <target_name_or_path> [extra_args...]"
    Write-Host "  -b, --build       Build the unit test target"
    Write-Host "  -r, --run         Run the unit test target"
    Write-Host "  -d, --dir <dir>   Build directory (default: build)"
    Write-Host "  -h, --help        Show this help message"
    exit 1
}

if ($Help -or -not $Params -or $Params.Count -eq 0) {
    Show-Usage
}

$Target = $Params[0]
$ExtraArgs = if ($Params.Count -gt 1) { $Params[1..($Params.Count - 1)] } else { @() }

# Extract target name if a path was passed
$TargetName = [System.IO.Path]::GetFileName($Target)

# Build phase
if ($Build) {
    Write-Host "==> Building target: ${TargetName} in ./${BuildDir}" -ForegroundColor Cyan
    cmake --build "$BuildDir" --target "$TargetName"
}

# Run phase
if ($Run) {
    Write-Host "==> Running unit tests matching: ${TargetName}" -ForegroundColor Cyan
    
    # Try finding the executable in the build tree (.exe on Windows first)
    $ExePath = Get-ChildItem -Path "$BuildDir" -Recurse -Filter "${TargetName}.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName -First 1
    
    if (-not $ExePath) {
        $ExePath = Get-ChildItem -Path "$BuildDir" -Recurse -Filter "$TargetName" -ErrorAction SilentlyContinue | Where-Object { $_.Extension -eq "" } | Select-Object -ExpandProperty FullName -First 1
    }

    if ($ExePath -and (Test-Path $ExePath)) {
        Write-Host "==> Executing binary directly: $ExePath" -ForegroundColor Green
        $BuildRoot = (Resolve-Path $BuildDir).Path
        $env:Path = "$BuildRoot\bin;$BuildRoot\bin\plugins\logs;$env:Path"
        & "$ExePath" @ExtraArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Test executable exited with code $LASTEXITCODE."
        }
    } else {
        Write-Host "==> Binary not found directly, falling back to ctest..." -ForegroundColor Yellow
        ctest --test-dir "$BuildDir" -R "^${TargetName}$" --output-on-failure
    }
}
