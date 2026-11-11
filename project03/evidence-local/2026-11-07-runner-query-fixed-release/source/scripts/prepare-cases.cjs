const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'..'),run=process.argv[2];
if(!run||!/^[a-zA-Z0-9-]+$/.test(run))throw Error('Provide a successful RunName');
const verified=require('./verify-run.cjs').verifyRun(root,run);
const dir=path.join(root,'evidence-local',run);
const cache=fs.readFileSync(path.join(dir,'build','CMakeCache.txt'),'utf8');
const configuration=cache.match(/^CMAKE_BUILD_TYPE:STRING=(Debug|Release)$/m)?.[1];
assert.ok(configuration,'Missing verified build configuration');
assert.equal(configuration,verified.status.configuration);
const tests=fs.readFileSync(path.join(dir,'tests.log'),'utf8');
const result=tests.match(/RESULT cases=(\d+) failed=(\d+) assertions=(\d+)/);
assert.ok(result,'Missing aggregate test result');assert.equal(Number(result[2]),0);
assert.match(fs.readFileSync(path.join(dir,'ctest.log'),'utf8'),/100% tests passed/);
const catalog={schema:'p03.catalog.v1',default_case:'normal',cases:{}};
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const summaries=[];
for(const [key,prefix,count] of [['normal','normal-',6],['race','',8]]){
  const snapshot=path.join(dir,`${prefix}snapshot.json`);
  const data=JSON.parse(fs.readFileSync(snapshot,'utf8'));
  assert.equal(data.schema,'p03.offline.v1');assert.equal(data.scenario,key);assert.equal(data.frames.length,count);
  assert.equal(data.business_scope.account,data.account);assert.equal(data.business_scope.instrument,data.instrument);
  assert.equal(data.business_scope.exchange,'SHFE');assert.equal(data.business_scope.trading_day,20261105);
  assert.equal(data.business_scope.position_epoch,1);
  assert.ok(typeof data.strategy==='string' && data.strategy.length>0);
  const text=fs.readFileSync(path.join(dir,`${prefix}demo.log`),'utf8').trim().split(/\r?\n/);
  for(const [i,f] of data.frames.entries()){
    assert.equal(f.sequence,i+1);assert.equal(f.invariant,true);
    assert.equal(f.ingress_sequence,i+1);assert.equal(f.admission,'BusinessInput');
    if(f.order){
      assert.equal(f.order.parts.reduce((a,b)=>a+b,0),f.order.quantity);
      assert.ok(f.order.client_id?.producer && f.order.client_id?.value);
      assert.equal(f.order.strategy,data.strategy);
    }
    const line=text.findIndex(s=>s.startsWith(`step=${i+1} `));assert.ok(line>=0);
    assert.equal(f.canonical_state,text[line+1],`${key}: canonical divergence at ${i+1}`);
  }
  if(key==='normal'){
    assert.equal(data.source_order_quantity,10000);assert.equal(data.quantity_scale,1000);
    assert.match(data.source_url,/^https:\/\/www\.fixtrading\.org\//);
    assert.deepEqual(data.frames.map(f=>f.last_quantity),[0,0,0,2,1,7]);
    assert.deepEqual(data.frames.map(f=>f.order?.parts[0]||0),[0,0,0,2,3,10]);
    assert.deepEqual(data.frames.slice(1).map(f=>f.order.execution),['Created','Working','PartiallyFilled','PartiallyFilled','Filled']);
    for(const f of data.frames){assert.equal(f.error,'None');assert.equal(f.applied,true);}
    assert.deepEqual(data.frames.at(-1).order.parts,[10,0,0,0,0]);
    assert.equal(data.frames.at(-1).funds_required,1020);
  }else{
    assert.equal(data.frames[6].error,'Duplicate');assert.equal(data.frames[6].applied,false);
    assert.deepEqual(data.frames[6].positions,data.frames[5].positions);
    assert.equal(data.frames[6].funds_required,data.frames[5].funds_required);
    assert.equal(data.frames[7].error,'NotImplemented');assert.equal(data.frames[7].applied,false);
    assert.deepEqual(data.frames[7].order.parts,[2,0,0,1,0]);
  }
  data.provenance={run,build:`MSVC / C++20 / ${configuration}`,generated_utc:new Date().toISOString(),snapshot_sha256:hash(snapshot),executable_sha256:hash(path.join(dir,'build','p03_export_snapshot.exe')),test_cases:Number(result[1]),test_assertions:Number(result[3])};
  catalog.cases[key]=data;summaries.push({case:key,frames:count,canonical_matches:count});
}
const collect=folder=>fs.readdirSync(folder,{withFileTypes:true}).flatMap(e=>e.isDirectory()?collect(path.join(folder,e.name)):[path.join(folder,e.name)]);
const sources=['include','src','examples','tests','scripts'].flatMap(s=>collect(path.join(root,s))).concat(path.join(root,'CMakeLists.txt'));
const manifest=sources.map(file=>({path:path.relative(root,file).replaceAll('\\','/'),sha256:hash(file)}));
fs.writeFileSync(path.join(dir,'source-sha256.json'),JSON.stringify(manifest,null,2));
fs.writeFileSync(path.join(dir,'preview-verification.json'),JSON.stringify({cases:summaries,tests:Number(result[1]),assertions:Number(result[3]),source_manifest:'source-sha256.json'},null,2));
fs.writeFileSync(path.resolve(root,'..','demos','round02','core-snapshot.js'),'// Generated from verified C++ runs. No hand-authored business states.\nwindow.P03_CORE_CATALOG = '+JSON.stringify(catalog).replaceAll('<','\\u003c')+';\nwindow.P03_CORE_SNAPSHOT = window.P03_CORE_CATALOG.cases.normal;\n');
console.log(`PASS: 2 cases / 14 snapshots match independent text-demo scheduling; ${result[1]} tests, ${result[3]} assertions; normal-source mapping and negative boundaries verified.`);
