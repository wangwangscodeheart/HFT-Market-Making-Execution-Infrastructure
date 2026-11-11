(() => {
  const $=id=>document.getElementById(id),catalog=window.P03_CORE_CATALOG;
  if(!catalog||catalog.schema!=='p03.catalog.v1'){
    $('finding').textContent='未找到经过校验的案例快照，请重新导出。';return;
  }
  let key=catalog.default_case,selected=1,timer=null;
  const labels=[['B / BOOKED','成交已记账'],['H / SUSPENSE','累计证据待补明细'],['P / POSSIBLE','仍可能成交'],['L / RELEASED','确认未成交可释放'],['V / UNRESOLVED','未决数量']];
  const chinese={ReferenceMark:'建立参考价',SubmitOrder:'提交本地委托',WorkingReport:'注入接收确认',TradeReport:'成交明细',RequestCancel:'请求撤单',TerminalReport:'撤单终态',RequestFutureOperation:'请求未实现能力'};
  const row=values=>{const element=document.createElement('tr');for(const value of values){const cell=document.createElement('td');cell.textContent=value;element.append(cell);}return element;};
  const data=()=>catalog.cases[key];
  function stop(){if(timer!==null)clearInterval(timer);timer=null;$('playFrames').textContent='自动浏览快照';}
  function describe(frame){
    if(frame.error==='Duplicate')return '同一成交再次到达，核心返回 Duplicate。没有重复记账，资金需求也未再次增加。';
    if(frame.error==='NotImplemented')return '发送请求被明确拒绝：当前阶段没有外发实现。该结果不是交易所拒单。';
    if(frame.order?.consistency==='MissingTrades')return '累计证据大于已收到的逐笔明细。缺口进入暂记，对应风险资源继续保留。';
    if(!frame.order)return '先输入本地参考价，尚未创建订单。这是本地环境准备，不是官方案例中的行情记录。';
    if(frame.order.execution==='Filled')return '三笔成交全部记账，剩余订单数量为零。订单风险转为持仓风险，保证金需求不会因订单结束而自动归零。';
    if(frame.event==='WorkingReport')return '本地注入接收确认，订单处于 Working；未发生实际网络发送，不证明 CTP 已确认。';
    if(frame.event==='RequestCancel')return '撤单请求尚未获得终态证据，剩余可能成交的数量不能提前释放。';
    if(frame.event==='SubmitOrder')return '核心接受委托并原子预占风险资源；当前发送维度仍为 NotImplemented。';
    return `本次成交 ${frame.last_quantity} 手，账本累计 ${frame.order.parts[0]} 手；未完成订单风险继续保留。`;
  }
  function render(){
    const d=data(),f=d.frames[selected],o=f.order,p=o?.parts||[0,0,0,0,0],normal=key==='normal';
    window.P03_CORE_SNAPSHOT=d;
    const scope=d.business_scope;
    $('scopeAccount').textContent=scope.account;
    $('scopeContract').textContent=`${scope.exchange} / ${scope.instrument}`;
    $('scopeDay').textContent=scope.trading_day;
    $('scopeEpoch').textContent=scope.position_epoch;
    $('scopeStrategy').textContent=d.strategy;
    $('validationSummary').textContent=`${d.provenance.build} · ${d.provenance.test_cases} / ${d.provenance.test_cases} 通过`;
    $('validationAssertions').textContent=`${d.provenance.test_assertions.toLocaleString('en-US')} 次断言；当前订单、持仓、事件及资源曲线读取本机 C++ 导出结果。没有真实行情或在线核心连接。`;
    const maxQty=d.frames.find(frame=>frame.order)?.order.quantity||0;
    $('clientIdentity').textContent=o?.client_id?`${o.client_id.producer} / ${o.client_id.value}`:'—';
    for(const [id,value] of Object.entries({external:o?`${o.execution} / ${o.consistency}`:'未创建订单',booked:o?`${p[0]} / ${o.quantity}`:'—',cancel:o?.cancel||'—',orderId:o?.id||'—',orderInstrument:d.instrument,orderSide:o?'BUY / OPEN':'—',orderPrice:o?.price??'—',orderQuantity:o?.quantity??'—',riskBooked:p[0],riskPending:p[1]+p[2]+p[4],riskReleased:p[3],eventType:f.event,sequence:f.sequence,tradeId:f.trade_id||'—',payload:JSON.stringify({error:f.error,applied:f.applied,invariant:f.invariant,ledger_records:f.trade_records,funds_required:f.funds_required},null,2),finding:describe(f),dedup:`${f.trade_records} 笔独立成交${f.error==='Duplicate'?' / 重复输入未入账':''}`,footerScene:`${normal?'STANDARD A.1.a':'CONTRACT REGRESSION'} · SEQ ${f.sequence} · OFFLINE`,eventCount:`${f.sequence} / ${d.frames.length} EVENTS`,reviewState:o?.consistency||'NO ORDER',sum:o?`${o.quantity} = ${p.join(' + ')}`:'尚无委托',canonical:f.canonical_state,frameCounter:`${selected+1} / ${d.frames.length}`,caseTitle:normal?'标准正向 / 全部成交':'契约回归 / 撤单竞态',caseSource:normal?'FIX 官方规范 A.1.a · 2020-07':'本地架构契约 · 非历史事故复刻',sourceMapping:normal?'原例 10,000；成交 2,000 + 1,000 + 7,000 → 本地 10；成交 2 + 1 + 7（÷1,000）。':'委托 3 手；最终成交 2 手、释放 1 手。用于检验迟到、重复与证据缺口。',sourceBoundary:'价格、买开方向、客户端请求 ID、策略身份、账户、合约、日历、费用为本地环境设定；非真实交易记录。',snapshotProvenance:`${d.provenance.build} · ${d.provenance.test_cases} 项测试 / ${d.provenance.test_assertions} 次断言 · 导出 ${d.provenance.generated_utc} · SHA256 ${d.provenance.snapshot_sha256.slice(0,16)}…`}))$(id).textContent=value;
    $('sourceLink').hidden=!d.source_url;$('sourceLink').href=d.source_url||'#';
    $('parts').replaceChildren(...labels.map(([name,meaning],i)=>row([name,p[i],meaning])));
    $('positionRows').replaceChildren(...['LONG / TODAY','LONG / PREV','SHORT / TODAY','SHORT / PREV'].map((bucket,i)=>row([bucket,f.position_anchor[i],f.positions[i]-f.position_anchor[i],f.positions[i],f.frozen[i]])));
    $('events').replaceChildren(...d.frames.slice(0,selected+1).map(frame=>row([String(frame.sequence).padStart(2,'0'),frame.ingress_sequence,'LocalFixture',frame.event,frame.order?.id||'—',`${frame.admission} / ${frame.error} / applied=${frame.applied}${frame.trade_id?' / '+frame.trade_id:''}`])));
    $('payload').textContent=JSON.stringify({ingress_sequence:f.ingress_sequence,core_sequence:f.sequence,admission:f.admission,error:f.error,applied:f.applied,invariant:f.invariant,client_id:o?.client_id||null,ledger_records:f.trade_records,funds_required:f.funds_required},null,2);
    $('scenarios').replaceChildren(...d.frames.map((frame,i)=>{const button=document.createElement('button');button.className=i===selected?'active':'';button.textContent=`${String(i+1).padStart(2,'0')}  ${chinese[frame.event]||frame.event}${frame.last_quantity?' '+frame.last_quantity+' 手':''}${frame.error!=='None'?' / '+frame.error:''}`;button.setAttribute('aria-pressed',String(i===selected));button.onclick=()=>{stop();selected=i;render();};return button;}));
    $('previousFrame').disabled=selected===0;$('nextFrame').disabled=selected===d.frames.length-1;
    $('caseSelect').value=key;
    document.dispatchEvent(new CustomEvent('p03:frame',{detail:{data:d,selected,maxQty}}));
  }
  $('caseSelect').onchange=()=>{stop();key=$('caseSelect').value;selected=1;render();};
  $('previousFrame').onclick=()=>{stop();selected=Math.max(0,selected-1);render();};
  $('nextFrame').onclick=()=>{stop();selected=Math.min(data().frames.length-1,selected+1);render();};
  $('playFrames').onclick=()=>{if(timer!==null){stop();return;}if(selected===data().frames.length-1)selected=0;$('playFrames').textContent='暂停浏览';render();timer=setInterval(()=>{selected++;render();if(selected===data().frames.length-1)stop();},1200);};
  document.addEventListener('visibilitychange',()=>{if(document.hidden)stop();});
  render();
})();
