// Independent explanatory fixture. This code never invokes the C++ candidate.
const frames = [
  ['READY','建立参考价','输入参考价 500，尚未提交委托。界面中的价格使用 tick 坐标。','ReferenceMark{500}',0,[0,0,0,0,0]],
  ['RESERVED','买开 3 手，等待外部证据','本地接受买开请求并预占风险资源。此时没有外发，也没有真实成交。','SubmitOrder{Buy, Open, price=499, quantity=3}',0,[0,0,3,0,0]],
  ['PARTIAL','第一笔成交记账','注入成交 t1，数量 1 手。多今增加 1，剩余 2 手仍可能成交。','TradeReport{trade_id=t1, quantity=1}',1,[1,0,2,0,0]],
  ['CANCEL REQUEST','请求撤单，继续保留风险','撤单请求不等于撤单成功。尚未取得终态证据时，剩余资源不能直接释放。','RequestCancel{order=7:1}',1,[1,0,2,0,0]],
  ['TERMINAL EVIDENCE','撤单终态，仍有迟到成交待核实','注入累计成交量为 2 的撤单证据；已知明细只有 1 手。另一手进入暂记，确认未成交的 1 手释放。','TerminalReport{Canceled, cumulative=2}',1,[1,1,0,1,0]],
  ['LATE TRADE','迟到成交补齐证据','注入成交 t2，将暂记的一手转为正式记账。多今为 2 手，释放量仍为 1 手。','TradeReport{trade_id=t2, quantity=1}',2,[2,0,0,1,0]],
  ['DEDUPLICATED','重复成交，状态保持','再次注入完全相同的 t2。成交去重保证不会再增加持仓，也不会重复消费资源。','TradeReport{trade_id=t2, quantity=1} // duplicate',2,[2,0,0,1,0]],
  ['BOUNDARY','外发能力明确不可用','请求后续阶段的发送操作，应得到不可用结果。M1A 不具备任何交易连接或外发能力。','RequestFutureOperation{SendOrder} // unavailable',2,[2,0,0,1,0]]
];
let current=0;
const $=id=>document.getElementById(id);
function render(){const [phase,title,why,event,booked,parts]=frames[current];$('phase').textContent=phase;$('stepTitle').textContent=title;$('explanation').textContent=why;$('event').textContent=event;$('filled').textContent=booked;$('long').textContent=booked;$('parts').innerHTML=parts.map(n=>`<td>${n}</td>`).join('');$('counter').textContent=`${current+1} / ${frames.length}`;$('steps').innerHTML=frames.map((_,i)=>`<span class="dot ${i<=current?'on':''}"></span>`).join('');$('invariant').textContent=current===0?'尚未创建订单。':`数量守恒：B + H + P + L + V = ${parts.reduce((a,b)=>a+b,0)} / 委托数量 3`;$('prev').disabled=current===0;$('next').disabled=current===frames.length-1;}
$('prev').onclick=()=>{if(current>0)current--;render();};$('next').onclick=()=>{if(current<frames.length-1)current++;render();};$('reset').onclick=()=>{current=0;render();};
const descriptions={'':['让每一次状态变化，都有据可查。','交易终端型 / 聚焦报价、订单和库存，保留清晰的风险边界。'],light:['把执行链路，变成可检查的证据。','工程监控型 / 浅色工作区，强调状态、事件与风险资源。'],learn:['从一笔订单，读懂交易核心。','教学解释型 / 逐步推进成交证据、持仓和资源的变化。']};
document.querySelectorAll('[data-theme]').forEach(button=>{button.onclick=()=>{document.body.className=button.dataset.theme;document.querySelectorAll('[data-theme]').forEach(b=>{b.classList.toggle('active',b===button);b.setAttribute('aria-pressed',String(b===button));});const [title,intro]=descriptions[button.dataset.theme];$('title').textContent=title;$('intro').textContent=intro;};button.setAttribute('aria-pressed',String(button.classList.contains('active')));});render();
