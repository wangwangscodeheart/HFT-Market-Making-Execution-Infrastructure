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
fs.writeFileSync(path.resolve(root,'..','demos','round02','quote-runner-data.js'),'window.P03_QUOTE_RUNNER = '+JSON.stringify(data,null,2)+';\n');
console.log('PASS: sealed quote-runner JSON, cancel-before-replace, withdrawal, final inventory and zero held quantity checked.');
