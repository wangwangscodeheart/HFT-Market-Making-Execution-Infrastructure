param([string]$OutputPath = '')
$ErrorActionPreference='Stop'
$manifest=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'manifest.json') -Raw -Encoding UTF8 | ConvertFrom-Json
if(-not $OutputPath){$OutputPath=Join-Path (Split-Path -Parent $PSScriptRoot) $manifest.original.name}
$OutputPath=[IO.Path]::GetFullPath($OutputPath)
if(Test-Path -LiteralPath $OutputPath){throw 'Output already exists; do not overwrite the original archive.'}
foreach($part in $manifest.parts){
    $inputPath=Join-Path $PSScriptRoot $part.name
    if((Get-Item -LiteralPath $inputPath).Length -ne $part.bytes){throw ('Part length mismatch; run git lfs pull: '+$part.name)}
    if((Get-FileHash -LiteralPath $inputPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $part.sha256){throw ('Part checksum mismatch: '+$part.name)}
}
$output=[IO.File]::Open($OutputPath,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write)
try {foreach($part in $manifest.parts){$input=[IO.File]::OpenRead((Join-Path $PSScriptRoot $part.name));try{$input.CopyTo($output)}finally{$input.Dispose()}}}finally{$output.Dispose()}
if((Get-FileHash -LiteralPath $OutputPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.original.sha256){throw 'Restored ZIP checksum mismatch; retain the output for inspection.'}
Write-Output ('Verified archive: '+$OutputPath)
