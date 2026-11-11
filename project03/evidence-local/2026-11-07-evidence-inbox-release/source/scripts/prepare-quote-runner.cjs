const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {verifyRun,hash}=require('./verify-run.cjs'),root=path.resolve(__dirname,'..'),run=process.argv[2]||'';
const {dir,status}=verifyRun(root,run);assert.equal(status.quote_runner_export,true);
const file=path.join(dir,'quote-runner.json'),data=JSON.parse(fs.readFileSync(file,'utf8'));
assert.equal(data.schema,'p03.quote-runner.v1');assert.equal(data.passed,true);assert.equal(data.frames.length,5);
assert.equal(data.market_origin,'InterfaceFixture');assert.equal(data.historical_trace,false);
let sequence=0;
for(const [i,f] of data.frames.entries()){
  assert.equal(f.ordinal,i+1);assert.equal(f.completed,true);assert.deepEqual(f.core_positions,f.peer_positions);
  for(const c of f.calls){assert.ok(c.core_sequence>sequence);sequence=c.core_sequence;assert.equal(c.error,'None');}
  for(const o of f.orders)assert.equal(o.parts.reduce((a,b)=>a+b,0),o.intent.quantity);
}
const replaced=data.frames[1].calls.map(c=>c.input.type),firstNew=replaced.indexOf('SubmitOrder');
assert.equal(replaced.slice(0,firstNew).filter(t=>t==='TerminalReport').length,2);
assert.equal(data.frames[3].error,'Blocked');assert.equal(data.frames[3].decision.withdraw,true);
assert.equal(data.frames[4].mode,1);assert.deepEqual(data.frames[4].core_positions,[1,0,0,0]);
assert.ok(data.frames[4].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
data.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_quote_runner'};
if(status.quote_expiry_export){
  const expiryFile=path.join(dir,'quote-expiry.json'),expiry=JSON.parse(fs.readFileSync(expiryFile,'utf8'));
  assert.equal(expiry.schema,data.schema);assert.equal(expiry.scenario,'market-expiry');assert.equal(expiry.passed,true);
  assert.equal(expiry.frames.length,6);assert.equal(expiry.market_origin,'InterfaceFixture');assert.equal(expiry.historical_trace,false);
  assert.equal(expiry.frames[1].at-expiry.frames[0].input.received_at,expiry.max_book_age);
  assert.equal(expiry.frames[1].calls.length,0);assert.equal(expiry.frames[1].error,'None');
  assert.equal(expiry.frames[2].error,'StaleMark');assert.equal(expiry.frames[2].calls.length,6);
  assert.equal(expiry.frames[3].calls.length,0);assert.equal(expiry.frames[4].orders.length,4);
  let seq=0;for(const f of expiry.frames){assert.equal(f.completed,true);assert.deepEqual(f.core_positions,[0,0,0,0]);assert.deepEqual(f.peer_positions,f.core_positions);
    for(const c of f.calls){assert.ok(c.core_sequence>seq);seq=c.core_sequence;assert.equal(c.error,'None');}}
  for(const i of [2,3,5])assert.ok(expiry.frames[i].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
  expiry.provenance={run,configuration:status.configuration,sha256:hash(expiryFile),producer:'p03_export_quote_runner'};data.expiry=expiry;
}
if(status.quote_query_gap_export){
  const queryFile=path.join(dir,'quote-query-gap.json'),query=JSON.parse(fs.readFileSync(queryFile,'utf8'));
  assert.equal(query.schema,data.schema);assert.equal(query.scenario,'query-gap');assert.equal(query.passed,true);
  assert.equal(query.frames.length,8);assert.equal(query.market_origin,'InterfaceFixture');assert.equal(query.historical_trace,false);
  const f=query.frames;assert.deepEqual(f[1].core_positions,[0,0,0,0]);assert.deepEqual(f[1].peer_positions,[1,0,0,0]);
  assert.equal(f[1].pending_reports,1);assert.equal(f[1].calls.length,0);
  assert.equal(f[2].error,'Blocked');assert.ok(f[2].calls.every(c=>c.input.type!=='SubmitOrder'));
  assert.equal(f[3].error,'Blocked');assert.equal(f[3].calls.length,0);assert.equal(f[3].canonical,f[2].canonical);assert.equal(f[3].pending_reports,1);
  assert.equal(f[4].error,'None');assert.equal(f[4].pending_reports,0);assert.deepEqual(f[4].core_positions,[1,0,0,0]);
  assert.equal(f[5].error,'Duplicate');assert.equal(f[5].calls[0].error,'Duplicate');assert.equal(f[6].orders.length,4);
  let seq=0;for(const frame of f){assert.equal(frame.completed,true);for(const c of frame.calls){assert.ok(c.core_sequence>seq);seq=c.core_sequence;assert.ok(['None','Duplicate'].includes(c.error));}
    for(const order of frame.orders)assert.equal(order.parts.reduce((a,b)=>a+b,0),order.intent.quantity);}
  assert.equal(f[7].mode,1);assert.deepEqual(f[7].core_positions,f[7].peer_positions);assert.ok(f[7].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
  query.provenance={run,configuration:status.configuration,sha256:hash(queryFile),producer:'p03_export_quote_runner'};data.query_gap=query;
}
fs.writeFileSync(path.resolve(root,'..','demos','round02','quote-runner-data.js'),'window.P03_QUOTE_RUNNER = '+JSON.stringify(data,null,2)+';\n');
console.log('PASS: sealed quote-runner JSON, cancel-before-replace, withdrawal, final inventory and zero held quantity checked.');
