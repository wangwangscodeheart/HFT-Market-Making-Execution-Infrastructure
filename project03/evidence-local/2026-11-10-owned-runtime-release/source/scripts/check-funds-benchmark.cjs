const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),{verifyRun}=require('./verify-run.cjs');
function validate(d){
  assert.equal(d.schema,'p03.funds-view-benchmark.v1');assert.equal(d.passed,true);assert.equal(d.origin,'LocalFixture');assert.equal(d.unit,'nanoseconds');assert.equal(d.clock,'steady_clock');
  assert.equal(d.warmup_pairs_per_profile,50);assert.equal(d.measured_pairs_per_profile,500);assert.equal(d.measured_calls,4000);
  for(const field of ['setup_included','external_checks_included','timer_overhead_subtracted','gateway_included'])assert.equal(d[field],false);
  assert.equal(d.alternating_pair_order,true);assert.deepEqual(d.profiles.map(p=>p.resources),[0,8,32,64]);
  for(const p of d.profiles){
    const n=p.resources;assert.equal(p.proofs,n);assert.equal(p.local_required,n*100+n*(n+1)/2);assert.equal(p.proven_cover,Math.ceil(n/3)*60);
    assert.equal(p.uncovered,p.local_required-p.proven_cover);assert.equal(p.external_budget,100000-p.uncovered-7);
    for(const v of [p.optimized,p.nested_reference]){assert.equal(v.samples_ns.length,500);assert.ok(v.samples_ns.every(x=>Number.isSafeInteger(x)&&x>=0));const s=[...v.samples_ns].sort((a,b)=>a-b);assert.deepEqual([v.min_ns,v.p50_ns,v.p99_ns,v.max_ns],[s[0],s[249],s[494],s[499]]);}
  }
}
const {dir}=verifyRun(path.resolve(__dirname,'..'),process.argv[2]||''),d=JSON.parse(fs.readFileSync(path.join(dir,'funds-view-benchmark.json'),'utf8'));validate(d);
let controls=0;for(const change of [x=>x.profiles[0].proven_cover=1,x=>x.profiles[0].optimized.p50_ns=-1,x=>x.profiles[0].nested_reference.samples_ns.pop(),x=>x.gateway_included=true]){const c=JSON.parse(JSON.stringify(d));change(c);assert.throws(()=>validate(c));controls++;}
console.log(`PASS: four paired funds profiles, 4000 measured calls, exact nested-reference agreement and ${controls} funds benchmark negative controls. Algorithm comparison only.`);
