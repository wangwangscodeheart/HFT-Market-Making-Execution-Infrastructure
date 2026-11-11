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
if(status.quote_retry_export){
  const retryFile=path.join(dir,'quote-retry.json'),retry=JSON.parse(fs.readFileSync(retryFile,'utf8')),f=retry.frames;
  assert.equal(retry.schema,data.schema);assert.equal(retry.scenario,'evidence-retry');assert.equal(retry.passed,true);
  assert.equal(retry.market_origin,'InterfaceFixture');assert.equal(retry.historical_trace,false);assert.equal(f.length,10);
  assert.equal(f[4].error,'InjectedFailure');assert.equal(f[4].calls.length,1);assert.equal(f[4].calls[0].error,'InjectedFailure');
  assert.equal(f[4].pending_delivery,2);assert.equal(f[4].pending_reports,2);assert.deepEqual(f[4].core_positions,[0,0,0,0]);
  assert.deepEqual(f[4].peer_positions,[1,0,1,0]);assert.equal(f[5].calls.length,0);assert.equal(f[5].error,'Blocked');
  assert.equal(f[6].pending_delivery,0);assert.equal(f[6].pending_reports,0);assert.equal(f[6].query_blocked,true);
  assert.deepEqual(f[6].core_positions,[1,0,1,0]);assert.equal(f[7].error,'Blocked');assert.equal(f[7].calls.length,0);
  assert.equal(f[8].query_blocked,false);assert.equal(f[8].calls.length,2);assert.ok(f[8].calls.every(c=>c.error==='Duplicate'));
  let seq=0;for(const frame of f){assert.equal(frame.completed,true);assert.equal(frame.delivery_loss,false);
    for(const c of frame.calls){assert.ok(c.core_sequence>seq);seq=c.core_sequence;assert.ok(['None','Duplicate','InjectedFailure'].includes(c.error));}
    for(const o of frame.orders)assert.equal(o.parts.reduce((a,b)=>a+b,0),o.intent.quantity);}
  assert.equal(f[9].mode,1);assert.deepEqual(f[9].core_positions,[1,0,1,0]);assert.deepEqual(f[9].peer_positions,f[9].core_positions);
  retry.provenance={run,configuration:status.configuration,sha256:hash(retryFile),producer:'p03_export_quote_runner'};data.retry=retry;
}
if(status.quote_overflow_export){
  const overflowFile=path.join(dir,'quote-overflow.json'),overflow=JSON.parse(fs.readFileSync(overflowFile,'utf8')),f=overflow.frames;
  assert.equal(overflow.schema,data.schema);assert.equal(overflow.scenario,'evidence-overflow');assert.equal(overflow.passed,true);
  assert.equal(overflow.historical_trace,false);assert.equal(overflow.market_origin,'InterfaceFixture');assert.equal(f.length,10);
  assert.equal(f[2].error,'InjectedFailure');assert.equal(f[2].pending_delivery,1);assert.deepEqual(f[2].core_positions,[10,0,0,0]);assert.deepEqual(f[2].peer_positions,[9,0,0,0]);
  assert.equal(f[4].pending_delivery,3);assert.equal(f[5].error,'Capacity');assert.equal(f[5].pending_delivery,3);
  for(const i of [3,4,5])assert.equal(f[i].input.bypass_backpressure_for_test,true);
  assert.equal(f[6].error,'None');assert.equal(f[6].pending_delivery,0);assert.deepEqual(f[6].core_positions,[9,0,0,0]);
  assert.equal(f[7].calls.length,1);assert.equal(f[7].calls[0].error,'Duplicate');assert.equal(f[7].error,'Blocked');assert.equal(f[8].calls.length,0);
  let seq=0;for(const [i,frame] of f.entries()){
    assert.equal(frame.completed,true);if(i>=5){assert.equal(frame.delivery_loss,true);assert.equal(frame.query_blocked,true);}
    assert.ok(Number.isSafeInteger(frame.core_required)&&frame.core_required>=0);assert.ok(frame.peer_available>=0&&frame.peer_available<=overflow.peer_initial_budget);
    for(const c of frame.calls){assert.ok(c.core_sequence>seq);seq=c.core_sequence;assert.ok(['None','Duplicate','InjectedFailure'].includes(c.error));}
  }
  assert.equal(f[9].mode,1);assert.deepEqual(f[9].core_positions,f[9].peer_positions);
  overflow.provenance={run,configuration:status.configuration,sha256:hash(overflowFile),producer:'p03_export_quote_runner'};data.overflow=overflow;
}
if(status.quote_cleanup_export){
  const file=path.join(dir,'quote-cleanup.json'),cleanup=JSON.parse(fs.readFileSync(file,'utf8')),f=cleanup.frames;
  assert.equal(cleanup.schema,data.schema);assert.equal(cleanup.scenario,'capacity-cleanup');assert.equal(cleanup.passed,true);
  assert.equal(cleanup.historical_trace,false);assert.equal(cleanup.market_origin,'InterfaceFixture');assert.equal(f.length,6);
  assert.equal(cleanup.runner_stopped,true);assert.equal(cleanup.cleanup_exhausted,true);
  assert.equal(cleanup.experiment_replay_verified,true);assert.equal(cleanup.input_attempts.length,9);
  assert.deepEqual(cleanup.input_attempts.map(a=>a.frames_after),[1,2,3,3,4,5,6,6,6]);
  assert.deepEqual(cleanup.input_attempts.filter(a=>a.frames_before===a.frames_after).map(a=>a.error),['Capacity','Halted','Capacity']);
  assert.equal(cleanup.input_attempts[8].cleanup_exhausted,true);
  assert.deepEqual(f.map(frame=>frame.at),[1,2,3,5,6,7]);
  assert.equal(f[2].error,'InjectedFailure');assert.equal(f[2].pending_delivery,1);
  assert.equal(f[3].pending_delivery,0);assert.equal(f[3].core_positions[0],1);
  let seq=0;for(const [i,frame] of f.entries()){
    assert.equal(frame.completed,true);assert.equal(frame.cleanup_lane,i>=3);
    for(const call of frame.calls){assert.ok(call.core_sequence>seq);seq=call.core_sequence;assert.ok(['None','Duplicate','InjectedFailure'].includes(call.error));}
  }
  assert.equal(f[4].mode,1);assert.deepEqual(f[5].core_positions,[1,0,0,0]);assert.deepEqual(f[5].core_positions,f[5].peer_positions);
  assert.ok(f[5].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
  assert.deepEqual(cleanup.admission_checks.map(c=>[c.at,c.actual]),[[4,'Capacity'],[8,'Halted'],[9,'Capacity']]);
  for(const check of cleanup.admission_checks){assert.equal(check.actual,check.expected);assert.equal(check.frames_before,check.frames_after);assert.equal(check.core_unchanged,true);}
  cleanup.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_quote_runner'};data.cleanup=cleanup;
}
if(status.quote_late_fill_export){
  const file=path.join(dir,'quote-late-fill.json'),late=JSON.parse(fs.readFileSync(file,'utf8')),f=late.frames;
  assert.equal(late.schema,data.schema);assert.equal(late.scenario,'late-after-kill');assert.equal(late.passed,true);
  assert.equal(late.market_origin,'InterfaceFixture');assert.equal(late.historical_trace,false);assert.equal(f.length,7);
  assert.deepEqual(f[2].orders[0].parts,[0,1,0,2,0]);assert.equal(f[2].core_required,102);
  assert.ok(f[2].calls.some(c=>c.input.type==='TerminalReport'&&c.input.order==='7:1'&&c.input.exact_quantity===1));
  assert.equal(f[3].calls.length,1);assert.equal(f[3].calls[0].input.type,'KillRequested');
  assert.deepEqual(f[4].orders[0].parts,[1,0,0,2,0]);assert.equal(f[4].core_required,102);
  assert.equal(f[5].calls[0].error,'Duplicate');assert.equal(f[5].calls[0].input.channel,1);assert.equal(f[6].calls[0].error,'Duplicate');
  let seq=0;for(const [i,frame] of f.entries()){
    assert.equal(frame.completed,true);if(i>=2){assert.equal(frame.mode,1);assert.equal(frame.cleanup_lane,true);}
    for(const c of frame.calls){assert.ok(c.core_sequence>seq);seq=c.core_sequence;assert.ok(['None','Duplicate'].includes(c.error));}
  }
  assert.deepEqual(f[6].core_positions,[1,0,0,0]);assert.deepEqual(f[6].core_positions,f[6].peer_positions);
  late.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_quote_runner'};data.late_fill=late;
}
if(status.quote_inventory_export){
  const file=path.join(dir,'quote-inventory.json'),inventory=JSON.parse(fs.readFileSync(file,'utf8')),f=inventory.frames;
  assert.equal(inventory.schema,data.schema);assert.equal(inventory.scenario,'inventory-boundary');assert.equal(inventory.passed,true);
  assert.equal(inventory.market_origin,'InterfaceFixture');assert.equal(inventory.historical_trace,false);assert.equal(f.length,7);
  assert.deepEqual(f.map(x=>x.core_positions[0]),[9,10,10,9,9,9,9]);
  assert.deepEqual(f[0].orders.map(o=>o.intent.quantity),[1,3]);assert.equal(f[2].orders.length,2);
  assert.equal(f[4].orders.length,4);let submitted=false,canceled=false;
  for(const c of f[4].calls){if(c.input.type==='RequestCancel'){assert.equal(submitted,false);canceled=true;}if(c.input.type==='SubmitOrder')submitted=true;}
  assert.equal(canceled,true);assert.equal(submitted,true);assert.equal(f[5].query_aligned,true);
  for(const frame of f){assert.equal(frame.completed,true);assert.deepEqual(frame.core_positions,frame.peer_positions);}
  assert.equal(f[6].mode,1);assert.ok(f[6].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
  inventory.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_quote_runner'};data.inventory=inventory;
}
if(status.quote_feed_export){
  const file=path.join(dir,'quote-feed.json'),feed=JSON.parse(fs.readFileSync(file,'utf8')),f=feed.frames;
  assert.equal(feed.passed,true);assert.equal(feed.scenario,'feed-recovery');assert.equal(f.length,12);
  assert.deepEqual(f.map(x=>x.feed.status),['Valid','Recovering','Recovering','Recovering','Recovering','Valid','Invalid','Valid','Stale','Valid','Valid','Valid']);
  for(const i of [1,2,3,4,6,8,11])assert.ok(f[i].orders.every(o=>o.parts[1]+o.parts[2]+o.parts[4]===0));
  for(const i of [2,3,4])assert.equal(f[i].calls.length,0);
  for(const frame of f){assert.equal(frame.completed,true);assert.deepEqual(frame.core_positions,frame.peer_positions);}
  assert.equal(f[4].error,'WrongIdentity');assert.equal(f[5].feed.subscription_generation,2);assert.equal(f[5].feed.last_receive_sequence,1);
  assert.equal(f[6].error,'InvalidInput');assert.equal(f[8].error,'StaleMark');assert.equal(f[10].query_aligned,true);assert.equal(f[11].mode,1);
  assert.equal(f[0].input.valid_depth,2);assert.equal(f[0].input.bids.length,5);assert.equal(f[0].input.asks.length,5);
  feed.provenance={run,configuration:status.configuration,sha256:hash(file),producer:'p03_export_quote_runner'};data.feed=feed;
}
fs.writeFileSync(path.resolve(root,'..','demos','round02','quote-runner-data.js'),'window.P03_QUOTE_RUNNER = '+JSON.stringify(data,null,2)+';\n');
console.log('PASS: sealed quote-runner JSON, cancel-before-replace, withdrawal, final inventory and zero held quantity checked.');
