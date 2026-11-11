// Offline observation charts: samples from sealed C++ frames, no interpolation
// of market prices, latency, or unobserved events.
window.P03RunnerCharts=(frames,selected)=>{
  const definitions=[
    {title:'总持仓 · 手',series:[['核心','#e9b65c',f=>f.core_positions.reduce((a,b)=>a+b,0)],['对端','#4cc9b0',f=>f.peer_positions.reduce((a,b)=>a+b,0)]]},
    {title:'订单风险占用 · 手（H + P + V）',series:[['核心占用','#91b9f2',f=>f.orders.reduce((n,o)=>n+o.parts[1]+o.parts[2]+o.parts[4],0)]]},
    {title:'回报状态 · 条（两项可能重叠，不相加）',series:[['待交付队列','#e9b65c',f=>f.pending_delivery||0],['已知待补齐','#d499d9',f=>f.pending_reports||0]]}
  ];
  if(!frames.length || selected<0 || selected>=frames.length)return '';
  return definitions.map(def=>{
    const series=def.series.map(([name,color,value])=>({name,color,values:frames.map(value)}));
    if(series.some(s=>s.values.some(v=>!Number.isFinite(v)||v<0)))return '<p>趋势数据无效</p>';
    const max=Math.max(1,...series.flatMap(s=>s.values)),left=42,right=480,top=15,bottom=125;
    const x=i=>left+(right-left)*i/Math.max(1,frames.length-1),y=v=>bottom-(bottom-top)*v/max;
    const lines=series.map((s,index)=>{
      const points=s.values.flatMap((v,i)=>i?[[x(i),y(s.values[i-1])],[x(i),y(v)]]:[[x(i),y(v)]]).map(p=>p.join(',')).join(' ');
      return `<polyline data-series="${index}" points="${points}" fill="none" stroke="${s.color}" stroke-width="2.5" ${index?'stroke-dasharray="6 4"':''}/><circle cx="${x(selected)}" cy="${y(s.values[selected])}" r="4" fill="${s.color}"/>`;
    }).join('');
    return `<article><h3>${def.title}</h3><svg viewBox="0 0 500 158" role="img" aria-label="${def.title}，横轴为观察步骤"><path d="M${left} ${top}V${bottom}H${right}" fill="none" stroke="#536172"/><text x="8" y="20" fill="#aab8c9">${max}</text><text x="20" y="129" fill="#aab8c9">0</text>${lines}<line data-selected="${selected+1}" x1="${x(selected)}" x2="${x(selected)}" y1="${top}" y2="${bottom}" stroke="#718096"/><text x="42" y="151" fill="#aab8c9">观察步骤 1 → ${frames.length} · 当前 ${selected+1}</text></svg><p>${series.map(s=>`<span style="color:${s.color}">${s.name} ${s.values[selected]}</span>`).join(' · ')}</p></article>`;
  }).join('');
};
