$ErrorActionPreference='Stop'
$workspace=Split-Path -Parent $PSScriptRoot
$zipPath=Join-Path $workspace 'PROJECT03_FREEZE_2026-11-11.zip'
$restorePath=Join-Path $workspace 'PROJECT03_FREEZE_VERIFY_2026-11-11'
if(Test-Path -LiteralPath $zipPath){throw 'Archive already exists; never overwrite'}
if(Test-Path -LiteralPath $restorePath){throw 'Restore directory already exists; never overwrite'}
Push-Location -LiteralPath $workspace
try {
    & tar -a -cf $zipPath demos handover_review project03 freeze-2026-11-11 2> (Join-Path $workspace 'PROJECT03_FREEZE_ARCHIVE.log')
    if($LASTEXITCODE -ne 0){throw 'Archive creation failed; retain log and incomplete archive'}
    New-Item -ItemType Directory -Path $restorePath | Out-Null
    & tar -xf $zipPath -C $restorePath 2> (Join-Path $workspace 'PROJECT03_FREEZE_EXTRACT.log')
    if($LASTEXITCODE -ne 0){throw 'Extraction failed; retain logs and partial restore'}
    & node (Join-Path $PSScriptRoot 'verify-package.cjs') $restorePath $zipPath
    if($LASTEXITCODE -ne 0){throw 'Extracted hashes failed'}
} finally {Pop-Location}
