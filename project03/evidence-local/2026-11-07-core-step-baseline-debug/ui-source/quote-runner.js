(()=>{
  const $=id=>document.getElementById(id),catalog=window.P03_QUOTE_RUNNER;let data=catalog;
  if(!data || data.schema!=='p03.quote-runner.v1' || !data.passed){$('runnerStatus').textContent='无有效 C++ 运行器快照';return;}
  let selected=0;const names=['初始双侧报价','盘口更新 · 先撤后建','买侧成交','异常盘口 · 撤回存量','停止报价 · 核心 Kill'];
  const row=(parent,values)=>{const tr=document.createElement('tr');values.forEach(value=>{const td=document.createElement('td');td.textContent=value;tr.append(td);});$(parent).append(tr);return tr;};
  function render(){
    $('runnerCharts').innerHTML=window.P03RunnerCharts(data.frames,selected);
    const f=data.frames[selected],expiry=data.scenario==='market-expiry',query=data.scenario==='query-gap',retry=data.scenario==='evidence-retry',overflow=data.scenario==='evidence-overflow';
    const cleanup=data.scenario==='capacity-cleanup';
    const late=data.scenario==='late-after-kill',inventory=data.scenario==='inventory-boundary';
    const label=cleanup?['普通额度 · 初始报价','普通额度 · 设置一次失败','普通额度 · 成交提交失败','收尾额度 · 重试已有回报','收尾额度 · Stop 与撤单','收尾额度 · 查询成交去重'][selected]:overflow?['初始十手多仓 · 卖平今报价','设置一次报告提交失败','平仓成交 · 核心提交失败','查询追加 · 队列二条','再次查询 · 队列三条','收件箱满 · 保留丢失标记','显式重试 · 队列排空','查询成功去重 · 仍保持停报','新盘口 · 仍禁止补单','停止报价 · 标记保留'][selected]:retry?['初始报价','买侧成交 · 暂扣','卖侧成交 · 暂扣','设置一次报告提交失败','查询首条失败 · 保留整批','队列未清 · 禁止补单','显式重试 · 补齐两笔','查询限制仍在 · 禁止补单','完整查询复核 · 两笔去重','停止报价 · 双向持仓保留'][selected]:query?['初始报价','对端成交 · 暂扣回报','有缺口 · 暂停补单并撤存量','缺行查询 · 不交付核心','完整查询 · 成交补齐','迟到推送 · 去重','新鲜盘口 · 重新规划','停止报价'][selected]:expiry?['初始报价','时效边界 · 仍有效','超过时效 · 实际撤单','重复到期检查 · 无新调用','新鲜盘口 · 重新报价','停止报价'][selected]:names[selected];
    const lateLabel=['三手双侧报价','买侧成交一手 · 暂扣','容量停报下 Stop · H 占用保留','重复 Stop · 不重撤终态单','Kill 后迟到推送 · 补记成交','Kill 后查询 · 经济成交去重','重复迟到推送 · 持仓不再增加'][selected];
    $('runnerStatus').textContent=`${selected+1} / ${data.frames.length} · ${late?lateLabel:label} · ${f.error}`;
    if(inventory)$('runnerStatus').textContent=`${selected+1} / ${data.frames.length} · ${['库存九手 · 买侧缩量','买入一手 · 达到限额','限额保持 · 不补买单','卖平一手 · 回到九手','先撤后建 · 恢复双侧目标','完整查询 · 两侧核对','Kill · 九手持仓保留'][selected]} · ${f.error}`;
    $('runnerCore').textContent=f.core_positions[0];$('runnerPeer').textContent=f.peer_positions[0];
    $('runnerHeld').textContent=f.orders.reduce((sum,o)=>sum+o.parts[1]+o.parts[2]+o.parts[4],0);$('runnerCalls').textContent=f.calls.length;
    $('runnerPending').textContent=f.pending_reports||0;
    $('runnerDeliveryState').textContent=`待交付队列 ${f.pending_delivery||0} · 查询限制 ${f.query_blocked?'保持':'无'} · 交付丢失标记 ${f.delivery_loss?'有':'无'} · 交付后视图对齐 ${f.query_aligned===true?'是':f.query_aligned===false?'否':'未核验'}`;
    $('runnerPositionBuckets').textContent=`四桶顺序：多今 / 多昨 / 空今 / 空昨 · 核心 ${f.core_positions.join('/')} · 对端 ${f.peer_positions.join('/')}`;
    $('runnerBudgetState').textContent=`本地模型单位，非真实保证金 · 核心资金需求 ${f.core_required??'—'} · 对端模型可用 ${f.peer_available??'—'} · 两者分别展示，不直接覆盖或相减做结算`;
    $('runnerAdmissionRows').replaceChildren();
    (data.admission_checks||[]).forEach(check=>row('runnerAdmissionRows',[check.at,check.expected,check.actual,`${check.frames_before} → ${check.frames_after}`,check.core_unchanged?'未改变':'已改变']));
    $('runnerAdmissionSummary').textContent=cleanup?'实验最终状态：容量停报保持，收尾预留也已耗尽。以下三项是 C++ 执行的拒绝尝试检查，不是完整输入审计日志。':'本案例没有单独记录拒绝输入检查。';
    $('runnerFinding').textContent=selected===1?'替代订单出现前，核心已收到两笔旧单的撤单终态。':selected===3?'异常盘口产生 Blocked 决策，同时实际撤掉剩余报价；Blocked 不等于操作失败。':selected===4?'核心已 Kill，订单占用为零；已成交的一手持仓保留，停止报价不等于清仓。':'本步结果来自 C++ 运行器，左侧为实际核心调用，右侧为核心订单观察值。';
    if(expiry)$('runnerFinding').textContent=[
      '盘口最大年龄为 '+data.max_book_age+' 个逻辑步。调用者显式投递定时事件，无后台计时器。',
      '恰好到最大年龄时仍有效；本步没有核心调用，也没有刷新盘口或参考价。',
      '盘口超过时效，实际经过两笔撤单请求、响应和终态；不凭时间到期直接释放资源。',
      '重复到期检查仍返回 StaleMark，没有新增核心调用或重复释放。',
      '只有序号更新且时效合格的新鲜盘口才重新报价；旧订单保留终态。',
      '核心已 Kill，当前订单无占用且持仓为零；此案例没有成交，不代表 Kill 会自动清仓。'][selected];
    if(query)$('runnerFinding').textContent=[
      '明确的接口故障实验。暂扣回报由运行器主动记录，不代表真实网络丢包检测能力。',
      '对端已成交一手，核心尚未收到回报；待补齐为 1，核心保留原订单风险。',
      '已知回报未记账，禁止新增报价并尝试撤存量；已成交订单可能拒绝撤单，不能据此释放风险。',
      '故意删掉查询成交行，检查为不完整：零次核心调用，缺口和核心状态均保留。',
      '完整查询通过检查后，成交进入核心，两侧今多均为一手；不签发完整恢复证明。',
      '原推送迟到，核心返回 Duplicate，持仓不再增加。',
      '新的合格盘口重新规划；卖侧优先平已有今多，旧报价终态保留。',
      '核心已 Kill，订单无占用，已成交的一手持仓仍保留。'][selected];
    if(retry)$('runnerFinding').textContent=[
      '固定的内存提交失败实验，不是生产自动恢复能力。',
      '对端买开成交一手，报告暂扣，核心保留原订单风险。',
      '对端卖开也成交一手。多今一手、空今一手，不自动互相净掉。',
      '此输入设置下一条回报在核心提交时失败，不改变已发生的对端成交。',
      '查询报告先整批进入收件箱；第一条提交失败，失败项与尚未尝试项均保留，核心尚未记账。',
      '待交付队列非空，市场输入没有产生新单或其他核心调用。',
      '显式重试使两条成交入账，队列清空；查询限制仍保留，不能仅凭队列空了重新报价。',
      '新的市场输入仍被阻断，没有产生新核心调用。',
      '再次通过完整查询核验；两笔成交返回 Duplicate，本次局部查询限制解除，不签发恢复证书。',
      '核心已 Kill，净仓为零，但多今一手和空今一手仍存在，总仓为两手。停止报价不清仓。'][selected];
    if(overflow)$('runnerFinding').textContent=[
      '显式初始多今十手，库存限制只允许卖侧减仓报价；收件箱上限为三条。',
      '设置下一条回报核心提交失败，不回滚对端已经发生的业务。',
      '对端已平仓一手，核心尚未记账；报告留在收件箱。',
      '故障注入显式绕过正常查询背压，继续追加重复报告；普通查询会阻断而不追加。',
      '压力注入使收件箱达到三条；重复经济成交仍需由核心去重。',
      '再次绕过背压注入，批次无法入箱，既有三条保留，交付丢失标记生效。',
      '重试后核心今多为九手，队列为空，但丢失标记不会清除。',
      '本次查询成交被核心识别为 Duplicate，局部调用成功仍不能清除此前溢出标记。',
      '新鲜盘口仍被阻断，零次核心调用；不因队列排空或查询成功自动重开。',
      '核心已 Kill，九手持仓保留；本版本没有实现解除溢出标记的流程。'][selected];
    if(cleanup)$('runnerFinding').textContent=[
      '普通输入额度为三条，另有三条收尾预留；两者独立且均有界。',
      '明确设置下一条回报核心提交失败，仍使用普通记录额度。',
      '对端成交一手，核心提交失败；报告仍保留，普通输入额度已用完。',
      '普通容量停报后，已有回报使用预留路径重试，核心补记一手，禁止恢复报价。',
      'Stop 使用收尾预留，Kill 核心并实际撤掉剩余报价，成交持仓仍保留。',
      '查询成功并识别重复成交，但不能解除容量停报。实验随后验证新行情被拒、额外重试因收尾额度耗尽被拒。'][selected];
    if(late)$('runnerFinding').textContent=[
      '专用多手接口测试，每侧三手，用于观察部分成交、撤单与迟到回报，不是实际行情轨迹。',
      '对端买开成交一手但推送暂扣，核心仍保留三手可能成交风险。',
      '两笔剩余报价实际撤掉。买单精确终态累计成交为一手，核心保留 H=1，另外两手确认未成交。',
      '重复 Stop 只记录 Kill，不再撤已知终态订单；H 是缺失成交证据占用，不是仍可撤的活动量。',
      '即使核心已 Kill，已在本地对端发生的成交仍入账。H 转为 B，资金需求仍为 102 个模型单位，不凭停机释放。',
      '查询成交与原推送使用相同经济身份，返回 Duplicate；核心仍是 Killed，不能恢复新报价。',
      '重复推送再次去重，两侧今多仍为一手；容量停报与 Kill 都保持。'][selected];
    if(inventory)$('runnerFinding').textContent=[
      '显式初始多今九手，策略限额十手、配置报价三手。买侧只报一手，卖侧平今三手；这是接口夹具，不是历史行情。',
      '买入一手后核心与对端均为十手。单侧报价数量受剩余库存空间约束，核心风控仍独立执行。',
      '新鲜盘口不再产生买单；卖平报价保持，旧买单已成交。',
      '卖平一手后多今回到九手，旧卖单还有两手未成交。',
      '卖侧剩余量需要恢复到目标三手，先实际撤掉旧卖单，再建立买一手、卖平三手的新报价。合法计划仍须逐笔准入，不保证双边原子接纳。',
      '完整查询核对订单、成交和持仓；已入账成交去重，局部视图对齐不等于恢复证书。',
      'Kill 实际撤掉剩余报价，订单占用为零，九手初始及成交后持仓保留；停机不清仓。'][selected];
    $('runnerFrames').replaceChildren();data.frames.forEach((frame,i)=>{const tr=row('runnerFrames',[frame.ordinal,frame.at,frame.input.type,frame.error,frame.calls.length]);
      const td=document.createElement('td'),button=document.createElement('button');button.textContent='查看';button.onclick=()=>{selected=i;render();};td.append(button);tr.append(td);});
    $('runnerCallRows').replaceChildren();f.calls.forEach(call=>{
      const input=call.input;
      const proof=input.cancel_state!==undefined?' / '+['None','Requested','Succeeded','Rejected','Unknown'][input.cancel_state]:input.terminal_kind!==undefined?' / '+['Filled','Canceled','Rejected','NotSent'][input.terminal_kind]+' F='+(input.exact_quantity??'未知'):'';
      row('runnerCallRows',[call.core_sequence,call.ingress,input.type,input.order||'—',call.error+proof]);
    });
    $('runnerCallPayload').textContent=JSON.stringify(f.calls,null,2);
    $('runnerOrders').replaceChildren();f.orders.forEach(o=>row('runnerOrders',[o.id,(o.intent.side===0?'买':'卖')+' / '+['开仓','平今','平昨'][o.intent.offset],o.intent.price,o.intent.quantity,o.parts.join('/'),['Created','Working','PartiallyFilled','Filled','Canceled','Rejected','NotSent'][o.execution]]));
    $('runnerInput').textContent=JSON.stringify({input:f.input,decision:f.decision,completed:f.completed,core_mode:f.mode,pending_delivery:f.pending_delivery,query_blocked:f.query_blocked,delivery_loss:f.delivery_loss,cleanup_lane:f.cleanup_lane,query_aligned:f.query_aligned},null,2);
    $('runnerCanonical').textContent=f.canonical;
    $('runnerPrevious').disabled=selected===0;$('runnerNext').disabled=selected===data.frames.length-1;
    $('runnerProvenance').textContent=`${data.provenance.producer} · ${data.provenance.run} / ${data.provenance.configuration} · JSON SHA256 ${data.provenance.sha256}`;
  }
  $('runnerScenario').onchange=()=>{
    const next=$('runnerScenario').value==='inventory'?catalog.inventory:$('runnerScenario').value==='late'?catalog.late_fill:$('runnerScenario').value==='cleanup'?catalog.cleanup:$('runnerScenario').value==='overflow'?catalog.overflow:$('runnerScenario').value==='retry'?catalog.retry:$('runnerScenario').value==='query'?catalog.query_gap:$('runnerScenario').value==='expiry'?catalog.expiry:catalog;
    if(!next){$('runnerStatus').textContent='所选场景未导出';return;}
    data=next;selected=0;render();
  };
  $('runnerPrevious').onclick=()=>{selected=Math.max(0,selected-1);render();};$('runnerNext').onclick=()=>{selected=Math.min(data.frames.length-1,selected+1);render();};
  $('runnerProvenance').textContent=`${data.provenance.producer} · ${data.provenance.run} / ${data.provenance.configuration} · JSON SHA256 ${data.provenance.sha256}`;render();
})();
