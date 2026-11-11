const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const folder=path.resolve(__dirname,'..','..','demos','round02'),html=fs.readFileSync(path.join(folder,'index.html'),'utf8');
class Element{constructor(){this.children=[];this.textContent='';}append(child){this.children.push(child);}replaceChildren(){this.children=[];}}
function boot(data){
  const nodes=new Map([...html.matchAll(/id="([^"]+)"/g)].map(m=>[m[1],new Element()]));
  const window={};if(data)window.P03_LOCAL_REPLAY=data;
  const context=vm.createContext({window,document:{getElementById(id){assert.ok(nodes.has(id),id);return nodes.get(id);},createElement(){return new Element();}}});
  vm.runInContext(fs.readFileSync(path.join(folder,'local-replay.js'),'utf8'),context);
  return id=>nodes.get(id);
}
const context=vm.createContext({window:{}});vm.runInContext(fs.readFileSync(path.join(folder,'local-replay-data.js'),'utf8'),context);
const data=context.window.P03_LOCAL_REPLAY,$=boot(data);
assert.equal($('replayRows').children.length,10);assert.equal($('replayPrevious').disabled,true);
for(let i=0;i<10;i++){
  $('replayRows').children[i].children[9].children[0].onclick();
  assert.equal($('replayPeer').textContent,data.frames[i].peer_positions[0]);
  assert.equal($('replayCore').textContent,data.frames[i].core_positions[0]);
  assert.equal($('replayMissing').textContent,data.frames[i].parts[1]);
  assert.deepEqual(JSON.parse($('replayDetail').textContent),JSON.parse(JSON.stringify(data.frames[i])));
}
assert.equal($('replayNext').disabled,true);$('replayNext').onclick();assert.match($('replayStatus').textContent,/10 \/ 10/);
$('replayRows').children[6].children[9].children[0].onclick();assert.equal($('replayMissing').textContent,2);
assert.equal($('replayReleased').textContent,8);assert.match($('replayExplanation').textContent,/仍占用资源/);
$('replayNext').onclick();assert.equal($('replayMissing').textContent,0);assert.equal($('replayCore').textContent,2);
$('replayNext').onclick();assert.match($('replayExplanation').textContent,/持仓不再增加/);
for(let i=0;i<12;i++)$('replayPrevious').onclick();assert.match($('replayStatus').textContent,/1 \/ 10/);
assert.match($('replayProvenance').textContent,new RegExp(data.provenance.sha256));
assert.match($('replayChart').innerHTML,/非实测时间/);assert.match($('replayChart').innerHTML,/polyline/);
if(data.query_recovery){
  $('replayVariant').value='query';$('replayVariant').onchange();
  $('replayRows').children[7].children[9].children[0].onclick();
  assert.match($('replayStatus').textContent,/查询补齐缺失成交/);assert.equal($('replayCore').textContent,2);
  assert.equal(JSON.parse($('replayDetail').textContent).error,'None');
  assert.match($('replayProvenance').textContent,new RegExp(data.query_recovery.provenance.sha256));
  $('replayNext').onclick();assert.match($('replayStatus').textContent,/查询后迟到推送/);
  assert.equal(JSON.parse($('replayDetail').textContent).error,'Duplicate');
  $('replayVariant').value='push';$('replayVariant').onchange();assert.match($('replayStatus').textContent,/1 \/ 10/);
}
assert.equal(boot(null)('replayStatus').textContent,'无有效 C++ 回放快照');
assert.equal(boot({...data,passed:false})('replayStatus').textContent,'无有效 C++ 回放快照');
for(const file of ['local-replay.css','local-replay-data.js','local-replay.js'])assert.ok(html.includes(file));
console.log('PASS: 10 replay frames, selected details, divergence, missing trades, convergence, duplicate handling, bounds, provenance, absent/failed data. Node DOM logic only; no browser layout claim.');
