param(
    [Parameter(Mandatory=$true)]
    [string]$InputFile,

    [string]$OutputFile = ""
)

if (-not (Test-Path $InputFile)) {
    Write-Host "ERROR: Input file not found: $InputFile" -ForegroundColor Red
    exit 1
}

if (-not $OutputFile) {
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($InputFile)
    $OutputFile = Join-Path $PSScriptRoot "ZyrexMenu\projects\menu\src\$($baseName)_glb.h"
}

$bytes = [System.IO.File]::ReadAllBytes($InputFile)
$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("#pragma once")
$lines.Add("#include <cstdint>")
$lines.Add("")
$lines.Add("// Embedded $([System.IO.Path]::GetFileName($InputFile)) - auto-generated, do not edit")
$lines.Add("namespace embedded_glb {")
$lines.Add("static const uint8_t data[] = {")

$row = ""
for ($i = 0; $i -lt $bytes.Length; $i++) {
    $row += "0x" + $bytes[$i].ToString("X2")
    if ($i -lt $bytes.Length - 1) { $row += "," }
    if (($i + 1) % 20 -eq 0) {
        $lines.Add("    " + $row)
        $row = ""
    }
}
if ($row -ne "") { $lines.Add("    " + $row) }

$lines.Add("};")
$lines.Add("static const uint32_t size = " + $bytes.Length + "u;")
$lines.Add("} // namespace embedded_glb")

[System.IO.File]::WriteAllLines($OutputFile, $lines)
Write-Host "Done. $($bytes.Length) bytes written to $OutputFile"
