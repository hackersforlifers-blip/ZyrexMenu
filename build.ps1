$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sln = Join-Path $scriptDir "ZyrexMenu\ZyrexMenu.sln"
$outDir = Join-Path $scriptDir "bin"

$msbuildPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)

$msbuild = $null
foreach ($path in $msbuildPaths) {
    if (Test-Path $path) {
        $msbuild = $path
        break
    }
}

if (-not $msbuild) {
    Write-Host "ERROR: MSBuild not found. Install Visual Studio 2022 with C++ workload." -ForegroundColor Red
    exit 1
}

Write-Host "Using MSBuild: $msbuild"
Write-Host "Solution: $sln"
Write-Host "Building menu.exe (Release x64)..."

& $msbuild $sln /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /t:Build /m /nologo 2>&1 | Out-File (Join-Path $outDir "build_output.txt")

Write-Host "Build exit code: $LASTEXITCODE"
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded!" -ForegroundColor Green
} else {
    Write-Host "Build failed. Check bin\build_output.txt for details." -ForegroundColor Red
}
