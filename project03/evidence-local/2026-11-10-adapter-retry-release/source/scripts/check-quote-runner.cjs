const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const dir=process.argv[2]?path.resolve(process.argv[2]):path.resolve(__dirname,'..','..','demos','round02'),html=fs.readFileSync(path.join(dir,'runner.html'),'utf8');
class Element{constructor(){this.children=[];this.textContent='';}append(child){this.children.push(child);}replaceChildren(){this.children=[];}}
function boot(data){
  const nodes=new Map([...html.matchAll(/id="([^"]+)"/g)].map(m=>[m[1],new Element()]));
  const context=vm.createContext({window:{P03_QUOTE_RUNNER:data},document:{getElementById(id){assert.ok(nodes.has(id),id);return nodes.get(id);},createElement(){return new Element();}}});
  vm.runInContext(fs.readFileSync(path.join(dir,'runner-charts.js'),'utf8'),context);
  vm.runInContext(fs.readFileSync(path.join(dir,'quote-runner.js'),'utf8'),context);return id=>nodes.get(id);
}
const ctx=vm.createContext({window:{}});vm.runInContext(fs.readFileSync(path.join(dir,'quote-runner-data.js'),'utf8'),ctx);
const data=ctx.window.P03_QUOTE_RUNNER,$=boot(data);
vm.runInContext(fs.readFileSync(path.join(dir,'runner-charts.js'),'utf8'),ctx);
const charts=ctx.window.P03RunnerCharts;
assert.equal(charts([],0),'');assert.equal(charts(data.frames,-1),'');assert.equal(charts(data.frames,data.frames.length),'');
for(const scenario of [data,data.expiry,data.query_gap,data.retry,data.overflow,data.cleanup,data.late_fill,data.inventory,data.feed].filter(Boolean)){
  for(const [i,f] of scenario.frames.entries()){
    const output=charts(scenario.frames,i);assert.equal((output.match(/<svg /g)||[]).length,3);
    assert.ok(!/NaN|Infinity/.test(output));assert.ok(output.includes('data-selected="'+(i+1)+'"'));
    assert.ok(output.includes('核心 '+f.core_positions.reduce((a,b)=>a+b,0)));
    assert.ok(output.includes('核心占用 '+f.orders.reduce((n,o)=>n+o.parts[1]+o.parts[2]+o.parts[4],0)));
  }
}
assert.ok(!/NaN|Infinity/.test(charts([data.frames[0]],0)));
const invalid=JSON.parse(JSON.stringify(data.frames[0]));invalid.core_positions[0]=NaN;
assert.match(charts([invalid],0),/趋势数据无效/);
assert.equal($('runnerPrevious').disabled,true);assert.equal($('runnerFrames').children.length,5);
for(let i=0;i<5;i++){
  $('runnerFrames').children[i].children[5].children[0].onclick();const f=data.frames[i];
  assert.equal($('runnerCore').textContent,f.core_positions[0]);assert.equal($('runnerPeer').textContent,f.peer_positions[0]);
  assert.equal($('runnerCallRows').children.length,f.calls.length);assert.equal($('runnerOrders').children.length,f.orders.length);
  assert.equal(JSON.parse($('runnerInput').textContent).completed,true);assert.equal($('runnerCanonical').textContent,f.canonical);
  const chart=$('runnerCharts').innerHTML;
  assert.equal((chart.match(/<svg /g)||[]).length,3);assert.ok(!/NaN|Infinity/.test(chart));
  assert.ok(chart.includes('data-selected="'+(i+1)+'"'));
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
  $('runnerFrames').children[3].children[5].children[0].onclick();assert.match($('runnerFinding').textContent,/绕过正常查询背压/);
  assert.equal(JSON.parse($('runnerInput').textContent).input.bypass_backpressure_for_test,true);
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
if(data.inventory){
  $('runnerScenario').value='inventory';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,7);
  assert.equal($('runnerCore').textContent,9);assert.match($('runnerFinding').textContent,/买侧只报一手/);
  assert.match($('runnerCharts').innerHTML,/核心 9/);assert.match($('runnerCharts').innerHTML,/data-selected="1"/);
  assert.deepEqual(JSON.parse($('runnerCallPayload').textContent).filter(c=>c.input.type==='SubmitOrder').map(c=>c.input.intent.quantity),[1,3]);
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,10);
  assert.match($('runnerCharts').innerHTML,/核心 10/);assert.match($('runnerCharts').innerHTML,/data-selected="2"/);
  $('runnerNext').onclick();assert.equal($('runnerOrders').children.length,2);assert.match($('runnerFinding').textContent,/不再产生买单/);
  $('runnerFrames').children[4].children[5].children[0].onclick();assert.equal($('runnerOrders').children.length,4);
  assert.match($('runnerFinding').textContent,/先实际撤掉旧卖单/);
  $('runnerNext').onclick();assert.match($('runnerDeliveryState').textContent,/交付后视图对齐 是/);
  $('runnerNext').onclick();assert.equal($('runnerCore').textContent,9);assert.equal($('runnerHeld').textContent,0);
  assert.equal($('runnerNext').disabled,true);assert.match($('runnerProvenance').textContent,new RegExp(data.inventory.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,5);
  console.log('PASS: inventory sizing, limit suppression, cancel-before-replace, query alignment, kill and scenario navigation.');
}
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
  if(data.cleanup.input_attempts){
    assert.equal($('runnerAttemptRows').children.length,9);assert.match($('runnerAttemptSummary').textContent,/9 次输入尝试/);
    for(const [i,a] of data.cleanup.input_attempts.entries()){
      $('runnerAttemptRows').children[i].children[6].children[0].onclick();
      const payload=JSON.parse($('runnerAttemptPayload').textContent);assert.equal(payload.ordinal,a.ordinal);assert.equal(payload.error,a.error);
      assert.equal(payload.stopped,a.stopped);assert.equal(payload.cleanup_exhausted,a.cleanup_exhausted);
      assert.match($('runnerStatus').textContent,new RegExp('选中尝试 '+a.ordinal));
      assert.equal($('runnerCanonical').textContent,data.cleanup.frames[a.frames_after-1].canonical);
      if(a.frames_before===a.frames_after)assert.match($('runnerAttemptSelection').textContent,/没有新增轨迹帧/);
      else assert.match($('runnerAttemptSelection').textContent,/已定位本次产生/);
    }
    assert.equal(JSON.parse($('runnerAttemptPayload').textContent).cleanup_exhausted,true);
    $('runnerPrevious').onclick();assert.equal($('runnerAttemptSelection').textContent,'');assert.equal($('runnerAttemptPayload').textContent,'');
    console.log('PASS: nine input-attempt rows, three no-frame rejections, final exhaustion, payload/frame mapping and cleared selection.');
  }
  $('runnerFrames').children[2].children[5].children[0].onclick();assert.equal(JSON.parse($('runnerInput').textContent).cleanup_lane,false);
  $('runnerNext').onclick();assert.equal(JSON.parse($('runnerInput').textContent).cleanup_lane,true);assert.equal($('runnerCore').textContent,1);
  $('runnerNext').onclick();assert.equal($('runnerHeld').textContent,0);assert.match($('runnerStatus').textContent,/Stop/);
  $('runnerNext').onclick();assert.match($('runnerFinding').textContent,/不能解除容量停报/);
  assert.match($('runnerProvenance').textContent,new RegExp(data.cleanup.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.equal($('runnerAdmissionRows').children.length,0);
  assert.equal($('runnerAttemptRows').children.length,0);assert.equal($('runnerAttemptPayload').textContent,'');
  assert.match($('runnerAttemptSummary').textContent,/尚未采集/);
  console.log('PASS: cleanup scenario lanes, reserved Stop, rejected admission checks and no automatic reopening.');
}

if(data.feed){
  $('runnerScenario').value='feed';$('runnerScenario').onchange();assert.equal($('runnerFrames').children.length,12);
  for(let i=0;i<12;++i){assert.match($('runnerFeedState').textContent,new RegExp(data.feed.frames[i].feed.status));assert.ok(!$('runnerStatus').textContent.includes('undefined'));if(i<11)$('runnerNext').onclick();}
  assert.equal($('runnerNext').disabled,true);assert.match($('runnerProvenance').textContent,new RegExp(data.feed.provenance.sha256));
  $('runnerScenario').value='cycle';$('runnerScenario').onchange();assert.equal($('runnerFeedState').textContent,'Legacy top fixture');
  console.log('PASS: feed health, twelve-frame navigation and scenario reset; Node DOM only.');
}
