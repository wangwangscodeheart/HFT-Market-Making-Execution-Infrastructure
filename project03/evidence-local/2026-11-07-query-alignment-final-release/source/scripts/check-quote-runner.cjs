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
if(data.query_gap){
  $('runnerScenario').value='query';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,8);
  $('runnerFrames').children[1].children[5].children[0].onclick();assert.equal($('runnerCore').textContent,0);assert.equal($('runnerPeer').textContent,1);assert.equal($('runnerPending').textContent,1);
  $('runnerFrames').children[3].children[5].children[0].onclick();assert.equal($('runnerCalls').textContent,0);assert.equal($('runnerPending').textContent,1);assert.match($('runnerFinding').textContent,/不完整/);
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,1);assert.equal($('runnerPending').textContent,0);
  $('runnerNext').onclick();assert.match($('runnerStatus').textContent,/Duplicate/);assert.equal($('runnerCore').textContent,1);
  $('runnerFrames').children[7].children[5].children[0].onclick();assert.equal($('runnerHeld').textContent,0);assert.equal($('runnerCore').textContent,1);
  assert.match($('runnerProvenance').textContent,new RegExp(data.query_gap.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.match($('runnerStatus').textContent,/1 \/ 5/);
}
assert.equal(boot({...data,passed:false})('runnerStatus').textContent,'无有效 C++ 运行器快照');
if(data.retry){
  $('runnerScenario').value='retry';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,10);
  $('runnerFrames').children[4].children[5].children[0].onclick();assert.match($('runnerStatus').textContent,/InjectedFailure/);
  assert.match($('runnerDeliveryState').textContent,/待交付队列 2/);assert.equal($('runnerPending').textContent,2);
  $('runnerFrames').children[6].children[5].children[0].onclick();assert.match($('runnerDeliveryState').textContent,/待交付队列 0 · 查询限制 保持/);
  assert.equal($('runnerPending').textContent,0);assert.match($('runnerPositionBuckets').textContent,/核心 1\/0\/1\/0/);
  $('runnerNext').onclick();assert.equal($('runnerCalls').textContent,0);assert.match($('runnerStatus').textContent,/Blocked/);
  $('runnerNext').onclick();assert.match($('runnerDeliveryState').textContent,/查询限制 无/);
  assert.ok($('runnerCallRows').children.every(row=>row.children[4].textContent==='Duplicate'));
  $('runnerNext').onclick();assert.match($('runnerFinding').textContent,/总仓为两手/);assert.equal($('runnerNext').disabled,true);
  assert.match($('runnerProvenance').textContent,new RegExp(data.retry.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.match($('runnerStatus').textContent,/1 \/ 5/);
}
assert.ok(fs.readFileSync(path.join(dir,'index.html'),'utf8').includes('href="runner.html"'));
if(data.overflow){
  $('runnerScenario').value='overflow';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,10);
  $('runnerFrames').children[5].children[5].children[0].onclick();assert.match($('runnerStatus').textContent,/Capacity/);
  assert.match($('runnerDeliveryState').textContent,/待交付队列 3/);assert.match($('runnerDeliveryState').textContent,/交付丢失标记 有/);
  $('runnerNext').onclick();assert.match($('runnerDeliveryState').textContent,/待交付队列 0/);assert.equal($('runnerCore').textContent,9);
  assert.match($('runnerDeliveryState').textContent,/交付丢失标记 有/);
  $('runnerNext').onclick();assert.match($('runnerStatus').textContent,/Blocked/);assert.equal($('runnerCallRows').children[0].children[4].textContent,'Duplicate');
  assert.match($('runnerDeliveryState').textContent,/交付后视图对齐 是/);
  assert.match($('runnerDeliveryState').textContent,/交付丢失标记 有/);
  $('runnerNext').onclick();assert.equal($('runnerCalls').textContent,0);assert.match($('runnerDeliveryState').textContent,/查询限制 保持/);
  assert.match($('runnerBudgetState').textContent,/非真实保证金/);assert.match($('runnerBudgetState').textContent,new RegExp(String(data.overflow.frames[8].peer_available)));
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,9);assert.match($('runnerFinding').textContent,/没有实现解除/);
  assert.match($('runnerProvenance').textContent,new RegExp(data.overflow.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.match($('runnerStatus').textContent,/1 \/ 5/);
}
console.log('PASS: five runner frames, calls/orders, preserved inventory after kill, bounds, provenance and missing/failed data; Node DOM logic only.');
if(data.late_fill){
  $('runnerScenario').value='late';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,7);
  $('runnerFrames').children[2].children[5].children[0].onclick();assert.equal($('runnerHeld').textContent,1);
  assert.match($('runnerFinding').textContent,/H=1/);assert.ok($('runnerCallRows').children.some(row=>String(row.children[4].textContent).includes('Canceled F=1')));
  $('runnerNext').onclick();assert.equal($('runnerCalls').textContent,1);assert.match($('runnerFinding').textContent,/不再撤/);
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,1);assert.equal($('runnerHeld').textContent,0);
  $('runnerNext').onclick();assert.equal(JSON.parse($('runnerCallPayload').textContent)[0].input.channel,1);
  assert.equal($('runnerCallRows').children[0].children[4].textContent,'Duplicate');
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,1);assert.equal($('runnerNext').disabled,true);
  assert.match($('runnerProvenance').textContent,new RegExp(data.late_fill.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();
  console.log('PASS: post-kill late fill, retained H, repeat Stop, exact terminal proof and query trade identity.');
}
if(data.cleanup){
  $('runnerScenario').value='cleanup';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,6);
  assert.equal($('runnerAdmissionRows').children.length,3);assert.match($('runnerAdmissionSummary').textContent,/不是完整输入审计/);
  $('runnerFrames').children[2].children[5].children[0].onclick();assert.equal(JSON.parse($('runnerInput').textContent).cleanup_lane,false);
  $('runnerNext').onclick();assert.equal(JSON.parse($('runnerInput').textContent).cleanup_lane,true);assert.equal($('runnerCore').textContent,1);
  $('runnerNext').onclick();assert.equal($('runnerHeld').textContent,0);assert.match($('runnerStatus').textContent,/Stop/);
  $('runnerNext').onclick();assert.match($('runnerFinding').textContent,/不能解除容量停报/);
  assert.match($('runnerProvenance').textContent,new RegExp(data.cleanup.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.equal($('runnerAdmissionRows').children.length,0);
  console.log('PASS: cleanup scenario lanes, reserved Stop, rejected admission checks and no automatic reopening.');
}
