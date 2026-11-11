const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {verifyRun,hash}=require('./verify-run.cjs');
const root=path.resolve(__dirname,'..'),run=process.argv[2]||'';
const {dir,status}=verifyRun(root,run);
assert.equal(status.component_scope,'m1b-local-replay-slice3');
const file=path.join(dir,'local-replay.json'),data=JSON.parse(fs.readFileSync(file,'utf8'));
assert.equal(data.schema,'p03.local-replay.v1');assert.equal(data.origin,'LocalSimulator');
assert.equal(data.historical_trace,false);assert.equal(data.time_unit,'logical_step');
assert.equal(data.passed,true);assert.equal(data.frames.length,10);
for(const f of data.frames){
  assert.equal(f.passed,true);assert.equal(f.peer_positions.length,4);assert.equal(f.core_positions.length,4);
  assert.equal(f.parts.length,5);assert.ok(f.parts.every(Number.isSafeInteger));
  assert.equal(f.parts.reduce((a,b)=>a+b,0),10);
}
assert.deepEqual(data.frames[6].parts,[0,2,0,8,0]);assert.deepEqual(data.frames[9].parts,[2,0,0,8,0]);
data.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_local_replay'};
fs.writeFileSync(path.resolve(root,'..','demos','round02','local-replay-data.js'),'window.P03_LOCAL_REPLAY = '+JSON.stringify(data,null,2)+';\n');
console.log('PASS: verified C++ local replay exported for offline workbench; '+run);
