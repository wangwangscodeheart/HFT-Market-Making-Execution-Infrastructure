// Validate the actual exporter against the independently scheduled text demo,
// then embed its JSON for file:// use without a server or a WebBridge.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'..');
const run=process.argv[2];
if(!run||!/^[a-zA-Z0-9-]+$/.test(run))throw Error('Provide RunName used for a successful build');
const evidence=path.join(root,'evidence-local',run);
const raw=fs.readFileSync(path.join(evidence,'snapshot.json'),'utf8');
const data=JSON.parse(raw);
const demo=fs.readFileSync(path.join(evidence,'demo.log'),'utf8').trim().split(/\r?\n/);
assert.equal(data.schema,'p03.offline.v1');
assert.equal(data.frames.length,8);
assert.match(fs.readFileSync(path.join(evidence,'tests.log'),'utf8'),/RESULT cases=85 failed=0 assertions=24868/);
assert.match(fs.readFileSync(path.join(evidence,'ctest.log'),'utf8'),/100% tests passed/);
for(const [index,frame] of data.frames.entries()){
  assert.equal(frame.sequence,index+1);
  assert.equal(frame.invariant,true);
  if(frame.order)assert.equal(frame.order.parts.reduce((a,b)=>a+b,0),frame.order.quantity);
  const line=demo.findIndex(text=>text.startsWith(`step=${frame.sequence} `));
  assert.ok(line>=0);
  assert.equal(frame.canonical_state,demo[line+1],`snapshot/text demo divergence at ${frame.sequence}`);
}
assert.equal(data.frames[6].error,'Duplicate');
assert.deepEqual(data.frames[6].positions,data.frames[5].positions);
assert.equal(data.frames[6].funds_required,data.frames[5].funds_required);
assert.equal(data.frames[7].error,'NotImplemented');
assert.equal(data.frames[7].applied,false);
assert.deepEqual(data.frames[7].order.parts,[2,0,0,1,0]);
assert.deepEqual(data.frames[7].positions,[2,0,0,0]);
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const collect=dir=>fs.readdirSync(dir,{withFileTypes:true}).flatMap(entry=>entry.isDirectory()?collect(path.join(dir,entry.name)):[path.join(dir,entry.name)]);
const sources=['include','src','examples','tests','scripts'].flatMap(dir=>collect(path.join(root,dir))).concat(path.join(root,'CMakeLists.txt'));
data.provenance={generated_utc:new Date().toISOString(),run,build:'MSVC / C++20 / Debug',snapshot_sha256:hash(path.join(evidence,'snapshot.json')),executable_sha256:hash(path.join(evidence,'build','p03_export_snapshot.exe')),sources:sources.map(file=>({path:path.relative(root,file).replaceAll('\\','/'),sha256:hash(file)}))};
const output=path.resolve(root,'..','demos','round02','core-snapshot.js');
fs.writeFileSync(output,'// Generated from a successful C++ offline run. Do not edit values.\nwindow.P03_CORE_SNAPSHOT = '+JSON.stringify(data).replaceAll('<','\\u003c')+';\n');
fs.writeFileSync(path.join(evidence,'preview-verification.json'),JSON.stringify({frames:8,canonical_matches:8,duplicate_idempotent:true,outbound_unavailable:true,provenance:data.provenance},null,2));
console.log('PASS: 8 C++ snapshots match text-demo canonical states; quantity conservation, duplicate idempotency, unavailable send; preview generated.');
