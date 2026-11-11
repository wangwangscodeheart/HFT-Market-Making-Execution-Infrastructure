const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {verifyRun}=require('./verify-run.cjs');
function validate(data){
  assert.equal(data.schema,'p03.core-step-benchmark.v1');assert.equal(data.passed,true);assert.equal(data.origin,'LocalFixture');
  assert.equal(data.warmup_cycles,50);assert.equal(data.measured_cycles,500);assert.equal(data.events_per_cycle,6);assert.equal(data.measured_steps,3000);
  assert.equal(data.single_thread,true);assert.equal(data.gateway_included,false);assert.equal(data.timer_overhead_subtracted,false);
  assert.equal(data.construction_included,false);assert.equal(data.core_internal_allocation_included,true);
  assert.equal(data.external_invariant_and_canonical_checks_included,false);assert.equal(data.nearest_rank_percentiles,true);
  assert.equal(data.clock,'steady_clock');assert.equal(data.unit,'nanoseconds');assert.ok(data.clock_period_num>0&&data.clock_period_den>0);
  assert.deepEqual(data.final_positions,[10,0,0,0]);
  assert.deepEqual(data.stages.map(s=>s.event),['ReferenceMark','SubmitOrder','WorkingReport','Fill2','Fill1','Fill7']);
  for(const stage of [...data.stages,data.timer_overhead]){
    assert.equal(stage.samples_ns.length,500);assert.ok(stage.samples_ns.every(v=>Number.isSafeInteger(v)&&v>=0));
    const sorted=[...stage.samples_ns].sort((a,b)=>a-b);
    assert.equal(stage.min_ns,sorted[0]);assert.equal(stage.p50_ns,sorted[249]);assert.equal(stage.p99_ns,sorted[494]);assert.equal(stage.max_ns,sorted[499]);
  }
}
const {dir}=verifyRun(path.resolve(__dirname,'..'),process.argv[2]||'');
const data=JSON.parse(fs.readFileSync(path.join(dir,'core-step-benchmark.json'),'utf8'));validate(data);
let controls=0;for(const change of [d=>d.stages[0].p99_ns=-1,d=>d.stages[0].samples_ns.pop(),d=>d.stages[0].samples_ns[0]=-1,d=>d.gateway_included=true]){
  const candidate=JSON.parse(JSON.stringify(data));change(candidate);assert.throws(()=>validate(candidate));controls++;
}
console.log(`PASS: 3000 measured steps, 50 warmup cycles, six per-event distributions and timer overhead; ${controls} benchmark negative controls. Local diagnostic only.`);
