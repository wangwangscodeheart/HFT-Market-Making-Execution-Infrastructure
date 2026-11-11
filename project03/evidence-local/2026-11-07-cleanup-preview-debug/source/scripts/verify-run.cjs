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
  for(const file of required)assert.ok(artifacts.has(file),`Required artifact absent: ${file}`);
  compareRecords(expectedSources,currentSources,'Source');compareRecords(status.artifacts,currentArtifacts,'Artifact');
}
function currentSources(root){
  const walk=dir=>fs.readdirSync(dir,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(dir,e.name)):[path.join(dir,e.name)]);
  return ['include','src','tests','examples','diagnostics','scripts'].flatMap(folder=>walk(path.join(root,folder)))
    .concat(path.join(root,'CMakeLists.txt')).map(file=>({path:path.relative(root,file).replaceAll('\\','/'),sha256:hash(file)}));
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
  return {dir,status,sources,current,artifacts,archive};
}
module.exports={verifyRun,validateEvidence,hash};
if(require.main===module){const historical=process.argv[3]==='--historical';const result=verifyRun(path.resolve(__dirname,'..'),process.argv[2]||'',{requireCurrent:!historical});console.log(`PASS: completed ${result.status.configuration} run; archived sources and ${result.artifacts.length} artifact hashes verified; ${historical?'historical mode, current workspace not compared':'current workspace source matches'}.`);}
