const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),{spawnSync}=require('node:child_process');
const {verifyRun,hash}=require('./verify-run.cjs');
const root=path.resolve(__dirname,'..'),run=process.argv[2]||'';
const {dir,status,sources,ui}=verifyRun(root,run);
const output=path.join(dir,'m1a-evidence-report.json');
assert.ok(!fs.existsSync(output),'Report already exists. Preserve it and use a fresh build run.');
const sameNames=(a,b,label)=>assert.deepEqual([...a].sort(),[...b].sort(),label);
const declared=[];
for(const file of fs.readdirSync(path.join(root,'tests')).filter(f=>f.endsWith('_tests.cpp'))){
  fs.readFileSync(path.join(root,'tests',file),'utf8').split(/\r?\n/).forEach((line,i)=>{
    const match=line.match(/^TEST_CASE\(([A-Za-z0-9_]+)\)/);
    if(match)declared.push({name:match[1],source:`tests/${file}`,line:i+1});
  });
}
const names=declared.map(t=>t.name);assert.equal(new Set(names).size,names.length,'Duplicate test declaration');
const registered=fs.readFileSync(path.join(dir,'test-list.log'),'utf8').trim().split(/\r?\n/);
sameNames(names,registered,'Source declarations differ from compiled registry');
const log=fs.readFileSync(path.join(dir,'tests.log'),'utf8');
const passed=[...log.matchAll(/^PASS (\w+) checks=(\d+)\r?$/gm)].map(m=>({name:m[1],checks:Number(m[2])}));
assert.ok(!/^FAIL /m.test(log));sameNames(names,passed.map(t=>t.name),'Compiled registry differs from aggregate passes');
const summary=log.match(/RESULT cases=(\d+) failed=(\d+) assertions=(\d+)/);assert.ok(summary);
assert.equal(Number(summary[1]),names.length);assert.equal(Number(summary[2]),0);
assert.ok(passed.every(t=>t.checks>0));assert.equal(passed.reduce((n,t)=>n+t.checks,0),Number(summary[3]));
const junit=fs.readFileSync(path.join(dir,'ctest-junit.xml'),'utf8');
assert.ok(!/<(?:failure|error|skipped)\b/.test(junit),'CTest contains failed/skipped cases');
const ctestNames=[...junit.matchAll(/<testcase\b([^>]*)>/g)].map(m=>m[1].match(/(?:^|\s)name="([^"]+)"/)?.[1]);
sameNames(names,ctestNames,'CTest JUnit differs from compiled registry');
const controls=JSON.parse(fs.readFileSync(path.join(dir,'negative-controls.json'),'utf8'));
assert.deepEqual(controls,{assertion_failure_exit:1,unknown_case_exit:2});
function runScript(name,args,logName){
  const result=spawnSync(process.execPath,[path.join(__dirname,name),...args],{encoding:'utf8'});
  fs.writeFileSync(path.join(dir,logName),(result.stdout||'')+(result.stderr||''));
  assert.equal(result.status,0,`${name} failed; see ${logName}`);return (result.stdout||'').trim();
}
const guard=runScript('check-evidence.cjs',[run],'evidence-guard-check.log');
const benchmark=status.core_step_benchmark?runScript('check-core-benchmark.cjs',[run],'core-step-benchmark-check.log'):null;
const preview=runScript('prepare-cases.cjs',[run],'preview-generation.log');
const frontend=runScript('check-workbench.cjs',[],'workbench-check.log');
const replayPreview=status.component_scope==='m1b-local-replay-slice3'
  ?runScript('prepare-local-replay.cjs',[run],'local-replay-generation.log'):null;
const replayFrontend=replayPreview?runScript('check-local-replay.cjs',[],'local-replay-workbench-check.log'):null;
const runnerPreview=status.quote_runner_export?runScript('prepare-quote-runner.cjs',[run],'quote-runner-generation.log'):null;
const runnerFrontend=runnerPreview?runScript('check-quote-runner.cjs',[],'quote-runner-workbench-check.log'):null;
// Generators only change excluded data files; static UI and core must still match.
verifyRun(root,run);
const bundle=ui?runScript('offline-bundle.cjs',[run],'offline-bundle-generation.log'):null;
const bundleDir=path.join(dir,'offline-preview');
const bundleChecks=bundle?[
  runScript('check-workbench.cjs',[bundleDir],'offline-workbench-check.log'),
  runScript('check-local-replay.cjs',[bundleDir],'offline-replay-check.log'),
  runScript('check-quote-runner.cjs',[bundleDir],'offline-runner-check.log')]:null;
const checks=new Map(passed.map(t=>[t.name,t.checks]));
const matrix=declared.map(t=>({...t,checks:checks.get(t.name),contracts:[...t.name.matchAll(/(?:^|_)(A\d{2}|O\d{2}|P\d{2})(?=_|$)/g)].map(m=>m[1])}));
const required=['A01','A04','A05','A06','A07','A08',...Array.from({length:9},(_,i)=>'O0'+(i+1)),...Array.from({length:6},(_,i)=>'P0'+(i+1))];
for(const contract of required)assert.ok(matrix.some(t=>t.contracts.includes(contract)),`No named business evidence for ${contract}`);
assert.ok(names.includes('BOUNDARY_future_capabilities_never_return_success'));
if(['m1b-localsim-slice1','m1b-local-replay-slice3'].includes(status.component_scope)) assert.match(fs.readFileSync(path.join(dir,'local-sim-demo.log'),'utf8'),/FINAL peer_core_match=1 fills=3 booked=10 origin=LocalSimulator/);
if(status.component_scope==='m1b-local-replay-slice3'){
  const replay=JSON.parse(fs.readFileSync(path.join(dir,'local-replay.json'),'utf8'));
  assert.equal(replay.schema,'p03.local-replay.v1');assert.equal(replay.passed,true);assert.equal(replay.frames.length,10);
  assert.ok(replay.frames.every(f=>f.passed===true));
  assert.deepEqual(replay.frames[2].peer_positions,[2,0,0,0]);assert.deepEqual(replay.frames[2].core_positions,[0,0,0,0]);
  assert.deepEqual(replay.frames[6].parts,[0,2,0,8,0]);assert.deepEqual(replay.frames[9].parts,[2,0,0,8,0]);
  assert.equal(replay.frames[9].error,'Duplicate');
}
const report={component_scope:status.component_scope,schema:'p03.local-m1a-evidence.v1',run,configuration:status.configuration,generated_utc:new Date().toISOString(),
  result:'LOCAL_REGRESSION_GATE_PASSED',cases:names.length,assertions:Number(summary[3]),failed:0,skipped:0,
  source_manifest_sha256:status.source_manifest_sha256,source_files:sources.length,source_archive:'source',
  ui_source_seal:ui?{manifest_sha256:status.ui_manifest_sha256,files:ui.expected.length,archive:'ui-source'}:null,
  offline_bundle:bundle?{path:'offline-preview',manifest_sha256:hash(path.join(bundleDir,'bundle-manifest.json')),generation:bundle,checks:bundleChecks}:null,
  declared_registry_aggregate_ctest_match:true,negative_controls:controls,evidence_guard:guard,preview_check:preview,frontend_logic_check:frontend,
  local_replay_preview:replayPreview,local_replay_frontend:replayFrontend,
  quote_runner_preview:runnerPreview,quote_runner_frontend:runnerFrontend,
  core_step_benchmark:benchmark,
  boundaries:{local_simulator:'Independent synchronous peer slice; full M1B strategy/runtime not complete',origin:'LocalFixture and LocalSimulator profiles isolated',native_browser_layout:'not verified',linux_current_source:'not verified; no installed WSL distribution or GCC/Clang compiler found',
    real_ctp:'not implemented/blocked',journal_writer_dispatch_runtime:'not implemented',performance:benchmark?'Local single-thread step diagnostic only; no production throughput or gateway latency claim':'not measured',
    phase_decision:'Local regression evidence is complete for the enumerated M1A scope. Scoped M1B local engineering may proceed under the current authorization; this is not external/production readiness.'},tests:matrix};
fs.writeFileSync(output,JSON.stringify(report,null,2),{flag:'wx'});
const lines=['# 当前本地回归证据清单（M1A 与已实现的 M1B 切片）','',`运行：${run} / ${status.configuration}。实际 ${report.cases} 项、${report.assertions} 次断言，0 失败、0 跳过。`,
  '', '源码声明、已编译注册表、聚合 PASS 与 CTest JUnit 四方逐项一致；失败退出码与证据负控通过。',
  '', '这是 LocalFixture / LocalSimulator 本地回归清单；不证明 Linux、CTP、真实线程、文件持久化或性能验收。JOURNAL/DISPATCH/RECOVERY 类型测试只验证值契约，不等于 J/K/Q 运行时测试。',
  '', '| 测试 | 直接编号 | 断言数 | 源码 |','|---|---|---:|---|',
  ...matrix.map(t=>`| ${t.name} | ${t.contracts.join(', ')||'补充回归/值契约'} | ${t.checks} | ${t.source}:${t.line} |`),''];
const markdown=lines.join('\n');fs.writeFileSync(path.join(dir,'m1a-test-matrix.md'),markdown,{flag:'wx'});
fs.writeFileSync(path.join(root,'docs','M1A_CURRENT_TEST_MATRIX.md'),markdown);
fs.writeFileSync(path.join(dir,'report-sha256.json'),JSON.stringify({report:hash(output),matrix:hash(path.join(dir,'m1a-test-matrix.md')),offline_bundle:bundle?hash(path.join(bundleDir,'bundle-manifest.json')):null},null,2));
console.log(`PASS: ${report.cases} tests / ${report.assertions} assertions; declarations, compiled registry, aggregate results and CTest JUnit match; local M1A evidence report generated.`);
