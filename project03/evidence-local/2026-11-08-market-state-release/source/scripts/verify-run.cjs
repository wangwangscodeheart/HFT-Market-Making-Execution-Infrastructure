const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const requiredArtifacts=['toolchain.json','run.cmd','configure.log','build.log','ctest.log','ctest-junit.xml','test-list.log','tests.log',
  'runner-negative.log','unknown-case-negative.log','negative-controls.json','demo.log','normal-demo.log','snapshot.json','normal-snapshot.json',
  'build/p03_tests.exe','build/p03_m1a_demo.exe','build/p03_export_snapshot.exe','build/CMakeCache.txt'];
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
function resolveWithin(base,relative){
  assert.equal(typeof relative,'string');assert.ok(relative.length>0 && relative.length<512);
  const target=path.resolve(base,relative),back=path.relative(base,target);
  assert.ok(back && back!=='..' && !back.startsWith('..'+path.sep) && !path.isAbsolute(back),'Evidence path escapes root');
  return target;
}
function recordMap(records){
  assert.ok(Array.isArray(records)&&records.length>0,'Missing evidence records');
  const map=new Map();for(const r of records){
    assert.equal(typeof r.path,'string');assert.match(r.sha256,/^[a-f0-9]{64}$/);
    assert.ok(!map.has(r.path),`Duplicate evidence path: ${r.path}`);map.set(r.path,r.sha256);
  }return map;
}
function compareRecords(expected,current,label){
  const a=recordMap(expected),b=recordMap(current);assert.equal(a.size,b.size,`${label} file set changed`);
  for(const [file,digest] of a)assert.equal(b.get(file),digest,`${label} mismatch: ${file}`);
}
function validateEvidence(status,expectedSources,currentSources,currentArtifacts){
  assert.equal(status.schema,'p03.build-run.v2');assert.equal(status.mode,'business');
  assert.equal(status.state,'completed','Run did not complete');assert.equal(status.exit_code,0,'Run failed');
  assert.ok(['Debug','Release'].includes(status.configuration));
  const artifacts=recordMap(status.artifacts);
  const sim=['m1b-localsim-slice1','m1b-local-replay-slice3'].includes(status.component_scope);
  const required=sim?requiredArtifacts.concat('local-sim-demo.log','build/p03_localsim_demo.exe'):requiredArtifacts;
  if(status.component_scope==='m1b-local-replay-slice3')required.push('local-replay.json','build/p03_export_local_replay.exe');
  if(status.query_first_replay===true)required.push('query-first-replay.json');
  if(status.quote_runner_export===true)required.push('quote-runner.json','build/p03_export_quote_runner.exe');
  if(status.quote_expiry_export===true)required.push('quote-expiry.json');
  if(status.quote_query_gap_export===true)required.push('quote-query-gap.json');
  if(status.quote_retry_export===true)required.push('quote-retry.json');
  if(status.quote_overflow_export===true)required.push('quote-overflow.json');
  if(status.quote_cleanup_export===true)required.push('quote-cleanup.json');
  if(status.quote_late_fill_export===true)required.push('quote-late-fill.json');
  if(status.quote_inventory_export===true)required.push('quote-inventory.json');
  if(status.core_step_benchmark===true)required.push('core-step-benchmark.json','build/p03_core_step_benchmark.exe');
  if(status.core_state_benchmark===true)required.push('core-state-benchmark.json','build/p03_core_state_benchmark.exe');
  if(status.funds_view_benchmark===true)required.push('funds-view-benchmark.json','build/p03_funds_view_benchmark.exe');
  for(const file of required)assert.ok(artifacts.has(file),`Required artifact absent: ${file}`);
  compareRecords(expectedSources,currentSources,'Source');compareRecords(status.artifacts,currentArtifacts,'Artifact');
}
function currentSources(root){
  const walk=dir=>fs.readdirSync(dir,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(dir,e.name)):[path.join(dir,e.name)]);
  return ['include','src','tests','examples','diagnostics','scripts'].flatMap(folder=>walk(path.join(root,folder)))
    .concat(path.join(root,'CMakeLists.txt')).map(file=>({path:path.relative(root,file).replaceAll('\\','/'),sha256:hash(file)}));
}
const generatedUi=new Set(['core-snapshot.js','local-replay-data.js','quote-runner-data.js']);
function currentUiSources(root){
  const dir=path.resolve(root,'..','demos','round02');
  return fs.readdirSync(dir,{withFileTypes:true}).filter(e=>e.isFile()&&!generatedUi.has(e.name))
    .map(e=>({path:e.name,sha256:hash(path.join(dir,e.name))}));
}
function validateUiEvidence(expected,archive,current){
  const files=recordMap(expected);
  for(const file of ['index.html','runner.html','quote-runner.js','runner-charts.js','terminal.css'])assert.ok(files.has(file),`Required UI source absent: ${file}`);
  for(const file of files.keys())assert.ok(!generatedUi.has(file),'Generated data is not static UI source');
  compareRecords(expected,archive,'Archived UI source');compareRecords(expected,current,'UI source');
}
function verifyRun(root,run,{requireCurrent=true}={}){
  assert.match(run,/^[a-zA-Z0-9-]+$/);const dir=path.join(root,'evidence-local',run);
  const status=JSON.parse(fs.readFileSync(path.join(dir,'run-status.json'),'utf8'));
  assert.equal(status.run,run);
  const manifestFile=path.join(dir,'build-source-manifest.json');
  assert.equal(hash(manifestFile),status.source_manifest_sha256,'Build source manifest was changed');
  const sources=JSON.parse(fs.readFileSync(manifestFile,'utf8'));
  assert.equal(status.source_archive,'source');
  const archiveRoot=path.join(dir,'source');
  const archive=sources.map(r=>({path:r.path,sha256:hash(resolveWithin(archiveRoot,r.path))}));
  compareRecords(sources,archive,'Archived source');
  const current=requireCurrent?currentSources(root):archive;
  const artifacts=status.artifacts.map(record=>({path:record.path,sha256:hash(resolveWithin(dir,record.path))}));
  validateEvidence(status,sources,current,artifacts);
  let ui=null;
  if(status.ui_source_seal===true){
    const manifest=path.join(dir,'ui-source-manifest.json');
    assert.equal(hash(manifest),status.ui_manifest_sha256,'UI manifest was changed');
    const expected=JSON.parse(fs.readFileSync(manifest,'utf8'));
    const archived=expected.map(r=>({path:r.path,sha256:hash(resolveWithin(path.join(dir,'ui-source'),r.path))}));
    const uiCurrent=requireCurrent?currentUiSources(root):archived;
    validateUiEvidence(expected,archived,uiCurrent);ui={expected,archive:archived,current:uiCurrent};
  }
  const reportFile=path.join(dir,'m1a-evidence-report.json');
  if(fs.existsSync(reportFile)){
    const report=JSON.parse(fs.readFileSync(reportFile,'utf8'));
    if(report.offline_bundle){
      const seals=JSON.parse(fs.readFileSync(path.join(dir,'report-sha256.json'),'utf8'));
      assert.equal(hash(reportFile),seals.report,'Evidence report changed');
      const bundleDir=path.join(dir,'offline-preview'),manifestHash=hash(path.join(bundleDir,'bundle-manifest.json'));
      assert.equal(manifestHash,report.offline_bundle.manifest_sha256);assert.equal(manifestHash,seals.offline_bundle);
      const bundle=require('./offline-bundle.cjs').verifyBundle(bundleDir);
      assert.equal(bundle.manifest.run,run);assert.equal(bundle.manifest.ui_manifest_sha256,status.ui_manifest_sha256);
      assert.ok(ui);const packaged=new Map(bundle.records.map(r=>[r.path,r.sha256]));
      for(const r of ui.expected)assert.equal(packaged.get(r.path),r.sha256,'Packaged static UI differs from build archive');
    }
  }
  return {dir,status,sources,current,artifacts,archive,ui};
}
module.exports={verifyRun,validateEvidence,validateUiEvidence,hash};
if(require.main===module){const historical=process.argv[3]==='--historical';const result=verifyRun(path.resolve(__dirname,'..'),process.argv[2]||'',{requireCurrent:!historical});console.log(`PASS: completed ${result.status.configuration} run; archived sources and ${result.artifacts.length} artifact hashes verified; ${historical?'historical mode, current workspace not compared':'current workspace source matches'}.`);}
