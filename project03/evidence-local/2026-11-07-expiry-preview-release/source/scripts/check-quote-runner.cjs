const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const dir=path.resolve(__dirname,'..','..','demos','round02'),html=fs.readFileSync(path.join(dir,'runner.html'),'utf8');
class Element{constructor(){this.children=[];this.textContent='';}append(child){this.children.push(child);}replaceChildren(){this.children=[];}}
function boot(data){
  const nodes=new Map([...html.matchAll(/id="([^"]+)"/g)].map(m=>[m[1],new Element()]));
  const context=vm.createContext({window:{P03_QUOTE_RUNNER:data},document:{getElementById(id){assert.ok(nodes.has(id),id);return nodes.get(id);},createElement(){return new Element();}}});
  vm.runInContext(fs.readFileSync(path.join(dir,'quote-runner.js'),'utf8'),context);return id=>nodes.get(id);
}
const ctx=vm.createContext({window:{}});vm.runInContext(fs.readFileSync(path.join(dir,'quote-runner-data.js'),'utf8'),ctx);
const data=ctx.window.P03_QUOTE_RUNNER,$=boot(data);
assert.equal($('runnerPrevious').disabled,true);assert.equal($('runnerFrames').children.length,5);
for(let i=0;i<5;i++){
  $('runnerFrames').children[i].children[5].children[0].onclick();const f=data.frames[i];
  assert.equal($('runnerCore').textContent,f.core_positions[0]);assert.equal($('runnerPeer').textContent,f.peer_positions[0]);
  assert.equal($('runnerCallRows').children.length,f.calls.length);assert.equal($('runnerOrders').children.length,f.orders.length);
  assert.equal(JSON.parse($('runnerInput').textContent).completed,true);assert.equal($('runnerCanonical').textContent,f.canonical);
}
assert.equal($('runnerHeld').textContent,0);assert.equal($('runnerCore').textContent,1);assert.match($('runnerFinding').textContent,/不等于清仓/);
assert.equal($('runnerNext').disabled,true);$('runnerNext').onclick();assert.match($('runnerStatus').textContent,/5 \/ 5/);
for(let i=0;i<8;i++)$('runnerPrevious').onclick();assert.match($('runnerStatus').textContent,/1 \/ 5/);
assert.match($('runnerProvenance').textContent,new RegExp(data.provenance.sha256));
if(data.expiry){
  $('runnerScenario').value='expiry';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,6);
  $('runnerFrames').children[1].children[5].children[0].onclick();assert.equal($('runnerCalls').textContent,0);assert.match($('runnerFinding').textContent,/恰好/);
  $('runnerNext').onclick();assert.equal($('runnerCalls').textContent,6);assert.equal($('runnerHeld').textContent,0);
  $('runnerNext').onclick();assert.equal($('runnerCalls').textContent,0);assert.match($('runnerStatus').textContent,/StaleMark/);
  $('runnerNext').onclick();assert.equal($('runnerHeld').textContent,2);
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,0);assert.equal($('runnerNext').disabled,true);
  assert.match($('runnerProvenance').textContent,new RegExp(data.expiry.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.match($('runnerStatus').textContent,/1 \/ 5/);
}
assert.equal(boot(null)('runnerStatus').textContent,'无有效 C++ 运行器快照');
assert.equal(boot({...data,passed:false})('runnerStatus').textContent,'无有效 C++ 运行器快照');
assert.ok(fs.readFileSync(path.join(dir,'index.html'),'utf8').includes('href="runner.html"'));
console.log('PASS: five runner frames, calls/orders, preserved inventory after kill, bounds, provenance and missing/failed data; Node DOM logic only.');
