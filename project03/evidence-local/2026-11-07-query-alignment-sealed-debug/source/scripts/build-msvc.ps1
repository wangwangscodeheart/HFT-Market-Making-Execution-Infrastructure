param([switch]$TraitsOnly,[string]$RunName='',
    [ValidateSet('Debug','Release')][string]$Configuration='Debug')
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
if(-not $RunName){$RunName=[DateTime]::UtcNow.ToString('yyyy-MM-dd-HHmmssZ')+'-'+[Guid]::NewGuid().ToString('N').Substring(0,6)}
if($RunName -notmatch '^[a-zA-Z0-9-]+$'){throw 'RunName must be an ASCII directory name'}
$outputRoot=Join-Path $projectRoot ('evidence-local\'+$RunName)
if(Test-Path -LiteralPath $outputRoot){throw 'Run directory already exists. Choose a fresh RunName; historical evidence is never overwritten.'}
New-Item -ItemType Directory -Path $outputRoot | Out-Null
function Write-JsonFile($file,$value){
    [IO.File]::WriteAllText((Join-Path $outputRoot $file),($value|ConvertTo-Json -Depth 12),[Text.UTF8Encoding]::new($false))
}
function Source-Manifest {
    $files=@(Join-Path $projectRoot 'CMakeLists.txt')
    foreach($folder in @('include','src','tests','examples','diagnostics','scripts')){
        $files+=@(Get-ChildItem -LiteralPath (Join-Path $projectRoot $folder) -File -Recurse | ForEach-Object FullName)
    }
    foreach($file in ($files|Sort-Object)){
        [ordered]@{path=$file.Substring($projectRoot.Length+1).Replace([IO.Path]::DirectorySeparatorChar,[char]'/');sha256=(Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()}
    }
}
$status=[ordered]@{schema='p03.build-run.v2';run=$RunName;configuration=$Configuration;mode=$(if($TraitsOnly){'traits'}else{'business'});state='started';started_utc=[DateTime]::UtcNow.ToString('o');finished_utc=$null;exit_code=$null;source_manifest_sha256=$null;source_archive='source';artifacts=@()}
Write-JsonFile 'run-status.json' $status
$status['component_scope']='m1b-local-replay-slice3'
$status['query_first_replay']=$true
$status['quote_runner_export']=$true
$status['quote_expiry_export']=$true
$status['quote_query_gap_export']=$true
$status['quote_retry_export']=$true
$status['quote_overflow_export']=$true
$status['quote_cleanup_export']=$true
$status['quote_late_fill_export']=$true
Push-Location -LiteralPath $outputRoot
try {
    $sources=@(Source-Manifest)
    Write-JsonFile 'build-source-manifest.json' $sources
    $status.source_manifest_sha256=(Get-FileHash -LiteralPath 'build-source-manifest.json').Hash.ToLowerInvariant()
    $archiveRoot=Join-Path $outputRoot 'source'
    foreach($record in $sources){
        $target=Join-Path $archiveRoot $record.path
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target)|Out-Null
        [IO.File]::Copy((Join-Path $projectRoot $record.path),$target,$false)
        if((Get-FileHash -LiteralPath $target).Hash.ToLowerInvariant() -ne $record.sha256){throw 'Source changed while archiving'}
    }
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $info=New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName=$vswhere
    $info.Arguments='-latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json -utf8'
    $info.UseShellExecute=$false;$info.RedirectStandardOutput=$true;$info.StandardOutputEncoding=[Text.Encoding]::UTF8
    $process=[Diagnostics.Process]::Start($info)
    $installationJson=$process.StandardOutput.ReadToEnd();$process.WaitForExit()
    if($process.ExitCode -ne 0){throw 'vswhere failed'}
    $installation=@($installationJson|ConvertFrom-Json)|Select-Object -First 1
    if(-not $installation){throw 'MSVC x64 installation not found'}
    $vcvars=Join-Path $installation.installationPath 'VC\Auxiliary\Build\vcvars64.bat'
    if(-not (Test-Path -LiteralPath $vcvars)){throw 'MSVC x64 toolchain not found'}
    $env:P03_VCVARS=$vcvars
    Write-JsonFile 'toolchain.json' ([ordered]@{installation=$installation.installationPath;version=$installation.installationVersion;generator='NMake Makefiles';configuration=$Configuration})
    if($TraitsOnly){
        @('/nologo','/std:c++20','/EHsc','/utf-8','/W4',('/I"'+$projectRoot+'\include"'),('"'+$projectRoot+'\diagnostics\swap_traits.cpp"'),'/Fe:swap_traits.exe')|Set-Content traits.rsp -Encoding UTF8
        @('@echo off','call "%P03_VCVARS%" >nul','if errorlevel 1 exit /b 1','cl @traits.rsp > traits-build.log 2>&1','if errorlevel 1 exit /b 1','swap_traits.exe > traits-result.txt','exit /b %errorlevel%')|Set-Content run.cmd -Encoding ASCII
        $artifactNames=@('traits-build.log','traits-result.txt','swap_traits.exe','run.cmd','traits.rsp','toolchain.json')
    }else{
        @('@echo off','call "%P03_VCVARS%" >nul','if errorlevel 1 exit /b 1',
          ('cmake -S "'+$projectRoot+'" -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE='+$Configuration+' -DBUILD_TESTING=ON > configure.log 2>&1'),
          'if errorlevel 1 exit /b 1','cmake --build build > build.log 2>&1','if errorlevel 1 exit /b 1',
          'ctest --test-dir build --output-on-failure --output-junit "%CD%\ctest-junit.xml" > ctest.log 2>&1','if errorlevel 1 exit /b 1',
          'build\p03_tests.exe --list > test-list.log','if errorlevel 1 exit /b 1',
          'build\p03_tests.exe > tests.log 2>&1','if errorlevel 1 exit /b 1',
          'build\p03_tests.exe --self-check-failure > runner-negative.log 2>&1','set P03_FAILURE_EXIT=%errorlevel%',
          'if not "%P03_FAILURE_EXIT%"=="1" exit /b 1',
          'build\p03_tests.exe --case nonexistent_case > unknown-case-negative.log 2>&1','set P03_UNKNOWN_EXIT=%errorlevel%',
          'if not "%P03_UNKNOWN_EXIT%"=="2" exit /b 1',
          'echo {"assertion_failure_exit":%P03_FAILURE_EXIT%,"unknown_case_exit":%P03_UNKNOWN_EXIT%} > negative-controls.json',
          'build\p03_m1a_demo.exe > demo.log 2>&1','if errorlevel 1 exit /b 1',
          'build\p03_export_snapshot.exe > snapshot.json','if errorlevel 1 exit /b 1',
          'build\p03_m1a_demo.exe --normal > normal-demo.log 2>&1','if errorlevel 1 exit /b 1',
          'build\p03_export_snapshot.exe --normal > normal-snapshot.json','if errorlevel 1 exit /b 1',
          'build\p03_localsim_demo.exe > local-sim-demo.log 2>&1','if errorlevel 1 exit /b 1',
          'build\p03_export_local_replay.exe > local-replay.json','if errorlevel 1 exit /b 1',
          'build\p03_export_local_replay.exe --query-first > query-first-replay.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe > quote-runner.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --expiry > quote-expiry.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --query-gap > quote-query-gap.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --retry > quote-retry.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --overflow > quote-overflow.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --cleanup > quote-cleanup.json','if errorlevel 1 exit /b 1',
          'build\p03_export_quote_runner.exe --late-fill > quote-late-fill.json','exit /b %errorlevel%')|Set-Content run.cmd -Encoding ASCII
        $artifactNames=@('toolchain.json','run.cmd','configure.log','build.log','ctest.log','ctest-junit.xml','test-list.log','tests.log',
          'runner-negative.log','unknown-case-negative.log','negative-controls.json','demo.log','normal-demo.log','snapshot.json','normal-snapshot.json',
          'build/p03_tests.exe','build/p03_m1a_demo.exe','build/p03_export_snapshot.exe','build/CMakeCache.txt',
          'local-sim-demo.log','build/p03_localsim_demo.exe','local-replay.json','build/p03_export_local_replay.exe','query-first-replay.json',
          'quote-runner.json','build/p03_export_quote_runner.exe','quote-expiry.json','quote-query-gap.json','quote-retry.json','quote-overflow.json','quote-cleanup.json','quote-late-fill.json')
    }
    & cmd.exe /d /c run.cmd
    $runExit=$LASTEXITCODE
    $status.exit_code=$runExit
    if($runExit -ne 0){throw "Build/verification failed with exit $runExit; inspect this run's logs"}
    $after=@(Source-Manifest)
    if(($sources|ConvertTo-Json -Compress) -ne ($after|ConvertTo-Json -Compress)){throw 'Source changed during build; evidence is not a coherent run'}
    $status.artifacts=@(foreach($file in $artifactNames){[ordered]@{path=$file;sha256=(Get-FileHash -LiteralPath $file).Hash.ToLowerInvariant()}})
    $status.state='completed';$status.finished_utc=[DateTime]::UtcNow.ToString('o')
    Write-JsonFile 'run-status.json' $status
    Write-Output ('Evidence: '+$outputRoot)
    if($TraitsOnly){Get-Content traits-result.txt}else{
        Get-Content ctest.log|Select-Object -Last 4
        Get-Content tests.log|Select-Object -Last 2
        Get-Content normal-demo.log|Select-Object -Last 1
    }
}catch{
    $status.state='failed';$status.finished_utc=[DateTime]::UtcNow.ToString('o')
    $status['error']=$_.Exception.Message
    Write-JsonFile 'run-status.json' $status
    throw
}finally{Pop-Location}
