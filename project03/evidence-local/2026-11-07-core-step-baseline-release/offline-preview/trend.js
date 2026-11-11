// Resource observations come from exported C++ states; candles remain independent illustrations.
(() => {
  const $=id=>document.getElementById(id);
  const colors=['#82b7a4','#85aeca','#c8ad76'];
  const frames=window.P03_CORE_SNAPSHOT?.frames;
  if(!frames){$('trendNote').textContent='C++ 快照缺失，工程趋势不可用。';return;}
  const observations=frames.map(frame=>{const p=frame.order?.parts||[0,0,0,0,0];return {b:p[0],r:p[1]+p[2]+p[4],h:p[1]};});
  // open / high / low / close, in synthetic tick coordinates; separate from order events.
  const candles=[[496,499,495,498],[498,500,497,499],[499,500,496,497],[497,501,497,500],
    [500,503,499,502],[502,503,500,501],[501,504,500,503],[503,505,502,504],
    [504,505,501,502],[502,506,502,505],[505,508,504,507],[507,509,506,508]];
  let mode='resources',selected=1;
  const x=i=>62+i*(696/(observations.length-1));
  function shell(content,description){return `<svg viewBox="0 0 820 225" role="img" aria-label="${description}"><title>${description}</title>${content}</svg>`;}
  function render(){
    document.querySelectorAll('[data-chart]').forEach(button=>{const active=button.dataset.chart===mode;button.classList.toggle('active',active);button.setAttribute('aria-pressed',String(active));});
    let content='';
    if(mode==='resources'){
      const y=value=>172-value*44;
      for(let n=0;n<=3;n++)content+=`<path d="M62 ${y(n)}H758" class="chart-grid"/><text x="42" y="${y(n)+4}" text-anchor="end">${n}</text>`;
      observations.forEach((_,i)=>content+=`<text x="${x(i)}" y="197" text-anchor="middle">${String(i+1).padStart(2,'0')}</text>`);
      content+=`<rect x="${x(selected)-13}" y="27" width="26" height="146" fill="#85aeca" opacity=".09"/><path d="M${x(selected)} 27V172" stroke="#73899a" stroke-dasharray="3 4"/>`;
      ['b','r','h'].forEach((key,k)=>{
        let d=`M${x(0)} ${y(observations[0][key])}`;
        observations.slice(1).forEach((point,j)=>{d+=`H${x(j+1)}V${y(point[key])}`;});
        content+=`<path d="${d}" fill="none" stroke="${colors[k]}" stroke-width="2" ${k===2?'stroke-dasharray="5 4"':''}/>`;
        observations.forEach((point,i)=>content+=`<circle cx="${x(i)}" cy="${y(point[key])}" r="${i===selected?4:2.5}" fill="${colors[k]}"><title>事件 ${i+1} · ${['已记账','未完成订单风险','暂记缺口'][k]} ${point[key]} 手</title></circle>`);
      });
      $('trendLegend').innerHTML=['已记账开仓','未完成订单风险（含暂记）','暂记明细缺口'].map((label,i)=>`<span><i style="background:${colors[i]}"></i>${label}</span>`).join('');
      $('trendChart').innerHTML=shell(content,'八个真实 C++ 执行步骤的离线快照；输入来源为 LocalFixture。');
      $('trendUnit').textContent='单位：lots · 横轴：事件序号';
      const p=observations[selected];
      $('trendSummary').textContent=`当前选中事件 ${String(selected+1).padStart(2,'0')}：已记账 ${p.b} 手 / 未完成订单风险 ${p.r} 手 / 暂记缺口 ${p.h} 手`;
      $('trendNote').textContent='曲线读取 C++ 导出的完整八步状态；竖线对应左侧所选场景。撤单请求（04）不释放资源；终态（05）保留明细缺口；重复成交（07）不增加持仓；外发请求（08）返回 NotImplemented。暂记属于订单风险子集，两条不能相加。成交后仍有持仓风险。';
    }else{
      const y=value=>174-(value-494)*9;
      [494,499,504,509].forEach(n=>content+=`<path d="M62 ${y(n)}H758" class="chart-grid"/><text x="44" y="${y(n)+4}" text-anchor="end">${n}</text>`);
      candles.forEach(([open,high,low,close],i)=>{
        const cx=85+i*59,color=close>=open?colors[0]:'#c28e91';
        content+=`<g><title>构造时段 ${i+1}：开 ${open} / 高 ${high} / 低 ${low} / 收 ${close}</title><path d="M${cx} ${y(high)}V${y(low)}" stroke="${color}"/><rect x="${cx-11}" y="${y(Math.max(open,close))}" width="22" height="${Math.max(2,Math.abs(close-open)*9)}" fill="${color}" opacity=".85"/></g>`;
        if(i%3===0||i===11)content+=`<text x="${cx}" y="197" text-anchor="middle">${String(i+1).padStart(2,'0')}</text>`;
      });
      $('trendChart').innerHTML=shell(content,'十二个独立构造时段的开高低收 K 线，整体价格上升，期间有回落；不是订单事件对应行情。');
      $('trendLegend').innerHTML=`<span><i style="background:${colors[0]}"></i>收盘 ≥ 开盘</span><span><i style="background:#c28e91"></i>收盘 &lt; 开盘</span><span>细线：最高 / 最低；实体：开盘 / 收盘</span>`;
      $('trendUnit').textContent='单位：构造 tick 坐标 · 横轴：等长样本时段';
      $('trendSummary').textContent='固定样本：首时段收盘 498 → 末时段收盘 508；价格背景整体上行，中间存在回落。';
      $('trendNote').textContent='每根 K 线概括一段时间的开、高、低、收四个价格。此处用于阅读行情背景，未加入预测指标。价格样本独立构造，不对应左侧订单事件；不能用于解释示例成交价格或评估策略收益。';
    }
  }
  document.querySelectorAll('[data-chart]').forEach(button=>button.addEventListener('click',()=>{mode=button.dataset.chart;render();}));
  document.querySelectorAll('[data-step]').forEach(button=>button.addEventListener('click',()=>{selected=[1,2,4,6][Number(button.dataset.step)];render();}));
  render();
})();
