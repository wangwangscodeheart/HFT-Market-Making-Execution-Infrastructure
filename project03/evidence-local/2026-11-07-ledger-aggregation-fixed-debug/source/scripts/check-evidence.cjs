// Uses actual run records but mutates copies in memory; historical files remain untouched.
const path=require('node:path'),assert=require('node:assert/strict');
const {verifyRun,validateEvidence,validateUiEvidence}=require('./verify-run.cjs');
const r=verifyRun(path.resolve(__dirname,'..'),process.argv[2]||'');
const clone=value=>JSON.parse(JSON.stringify(value));
let controls=0;
function reject(change){const s=clone(r.status),expected=clone(r.sources),current=clone(r.current),artifacts=clone(r.artifacts);change(s,expected,current,artifacts);assert.throws(()=>validateEvidence(s,expected,current,artifacts));controls++;}
reject(s=>s.state='failed');reject(s=>s.exit_code=1);
reject((s,e,c)=>c[0].sha256='0'.repeat(64));
reject((s,e,c,a)=>a[0].sha256='0'.repeat(64));
reject(s=>s.artifacts=s.artifacts.filter(a=>a.path!=='build/p03_tests.exe'));
reject((s,e,c)=>c.push({...c[0]}));
reject((s,e,c)=>c.pop());
reject((s,e,c)=>c.push({path:'src/unbuilt-new-source.cpp',sha256:'a'.repeat(64)}));
if(r.ui){
  function rejectUi(change){const expected=clone(r.ui.expected),archive=clone(r.ui.archive),current=clone(r.ui.current);change(expected,archive,current);assert.throws(()=>validateUiEvidence(expected,archive,current));controls++;}
  rejectUi((e,a,c)=>c[0].sha256='0'.repeat(64));
  rejectUi((e,a)=>a[0].sha256='0'.repeat(64));
  rejectUi((e,a,c)=>c.pop());
  rejectUi((e,a,c)=>c.push({path:'unbuilt-view.js',sha256:'a'.repeat(64)}));
}
console.log(`PASS: ${controls} negative evidence controls rejected failed/stale/changed/incomplete records. No source or run files modified.`);
