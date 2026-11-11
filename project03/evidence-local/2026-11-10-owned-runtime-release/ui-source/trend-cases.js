(() => {
  const $=id=>document.getElementById(id),colors=['#82b7a4','#85aeca','#c8ad76'];
  let data=window.P03_CORE_SNAPSHOT,selected=1,mode='resources';
  if(!data){$('trendNote').textContent='案例快照缺失，趋势不可用。';return;}
  function render(){
    const observations=data.frames.map(f=>{const p=f.order?.parts||[0,0,0,0,0];return {b:p[0],r:p[1]+p[2]+p[4],h:p[1]};});
    const max=Math.max(1,...data.frames.map(f=>f.order?.quantity||0));
    const x=i=>62+i*696/(observations.length-1),y=n=>172-n*132/max;
    let svg='';
    for(let j=0;j<=5;j++){const n=max*j/5;svg+=`<path d="M62 ${y(n)}H758" class="chart-grid"/><text x="42" y="${y(n)+4}" text-anchor="end">${Number(n.toFixed(1))}</text>`;}
    observations.forEach((_,i)=>svg+=`<text x="${x(i)}" y="197" text-anchor="middle">${i+1}</text>`);
    svg+=`<rect x="${x(selected)-12}" y="28" width="24" height="145" fill="#85aeca" opacity=".1"/><path d="M${x(selected)} 28V172" stroke="#73899a" stroke-dasharray="3 4"/>`;
    const series=mode==='resources'?['b','r','h']:['b'];
    series.forEach((key,k)=>{let d=`M${x(0)} ${y(observations[0][key])}`;observations.slice(1).forEach((v,i)=>d+=`H${x(i+1)}V${y(v[key])}`);svg+=`<path d="${d}" fill="none" stroke="${colors[k]}" stroke-width="2" ${key==='h'?'stroke-dasharray="5 4"':''}/>`;observations.forEach((v,i)=>svg+=`<circle cx="${x(i)}" cy="${y(v[key])}" r="${i===selected?4:2.5}" fill="${colors[k]}"><title>事件 ${i+1} / ${v[key]} 手</title></circle>`);});
    if(mode==='source'&&data.scenario==='normal'){
      svg+='<text x="70" y="18">原例累计 / 本地累计</text>';
      [3,4,5].forEach(i=>svg+=`<text x="${x(i)-5}" y="${Math.max(24,y(observations[i].b)-12)}" text-anchor="end">${observations[i].b*1000} / ${observations[i].b}</text>`);
    }
    $('trendChart').innerHTML=`<svg viewBox="0 0 820 225" role="img" aria-label="实际 C++ 离线快照的事件数量趋势"><title>实际 C++ 离线快照，横轴为逻辑事件，非真实时间或性能测量</title>${svg}</svg>`;
    const legends=mode==='resources'?['已记账开仓','未完成订单风险（含暂记）','暂记明细缺口']:['累计成交 / 本地手数'];
    $('trendLegend').innerHTML=legends.map((label,i)=>`<span><i style="background:${colors[i]}"></i>${label}</span>`).join('');
    $('trendUnit').textContent='单位：本地 lots · 横轴：逻辑事件序号';
    const p=observations[selected];
    $('trendSummary').textContent=`事件 ${selected+1}：已记账 ${p.b} 手 / 未完成订单风险 ${p.r} 手 / 暂记缺口 ${p.h} 手`;
    $('trendNote').textContent=mode==='source'?(data.scenario==='normal'?'标注为官方原例数量 / 本地数量；缩放系数 1,000。事件 1 为本地参考价准备，事件 2–6 对应原例接受分支。曲线是本地 C++ 成交账本结果，不是市场行情。':'该回归没有对应的公开原例；这里只显示本地成交累计，不伪造官方来源。'):'完整案例路径来自 C++ 运行快照；竖线为当前事件。暂记属于未完成订单风险子集，不能重复相加。订单结束后仍有持仓风险。没有真实时延或吞吐测量。';
    document.querySelectorAll('[data-chart]').forEach(b=>{const active=b.dataset.chart===mode;b.classList.toggle('active',active);b.setAttribute('aria-pressed',String(active));});
  }
  document.querySelectorAll('[data-chart]').forEach(b=>b.onclick=()=>{mode=b.dataset.chart;render();});
  document.addEventListener('p03:frame',event=>{data=event.detail.data;selected=event.detail.selected;render();});
  render();
})();
