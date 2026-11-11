(() => {
  'use strict';
  const $=id=>document.getElementById(id),catalog=window.P03_LOCAL_REPLAY;
  let data=catalog;
  if(!data || data.schema!=='p03.local-replay.v1' || !data.passed){$('replayStatus').textContent='无有效 C++ 回放快照';return;}
  const labels=['对端接单','核心收到 Working','对端成交 · 回报扣留','核心请求撤单','对端撤余单','核心收到撤单响应','核心收到终态 · 明细缺失','迟到成交补齐','重复交付','查询成交重放'];
  let selected=0;
  const label=i=>data.scenario==='query-first-cancel' && i>=7?['查询补齐缺失成交','查询后迟到推送','重复查询成交'][i-7]:labels[i];
  const cell=(row,value)=>{const td=document.createElement('td');td.textContent=value;row.append(td);};
  function render(){
    const f=data.frames[selected],p=f.parts;
    $('replayStatus').textContent=`${selected+1} / ${data.frames.length} · ${label(selected)} · ${f.passed?'检查通过':'检查失败'}`;
    $('replayPeer').textContent=f.peer_positions[0];$('replayCore').textContent=f.core_positions[0];
    $('replayMissing').textContent=p[1];$('replayReleased').textContent=p[3];
    $('replayDetail').textContent=JSON.stringify(f,null,2);
    $('replayExplanation').textContent=f.peer_positions[0]!==f.core_positions[0]
      ? (p[1]>0?'核心已知终态累计成交为 2 手，缺失的成交明细仍占用资源，等待补齐。':'对端已成交，核心尚未收到成交明细。两侧观察暂不一致。')
      : (f.error==='Duplicate'?'核心识别为重复成交，持仓不再增加。':'当前两侧今多持仓一致；这不构成完整恢复或资金对账证明。');
    if(data.blocked_queries)$('replayExplanation').textContent+=' 本次 C++ 运行还验证了 1 次缺项查询被阻断，完整查询通过检查后才补齐成交。';
    $('replayPrevious').disabled=selected===0;$('replayNext').disabled=selected===data.frames.length-1;
    $('replayRows').replaceChildren();
    data.frames.forEach((frame,i)=>{
      const row=document.createElement('tr');row.className=i===selected?'selected-row':'';
      cell(row,i+1);cell(row,label(i));cell(row,frame.peer_positions[0]);cell(row,frame.core_positions[0]);
      cell(row,frame.parts.join('/'));cell(row,frame.report_id||'—');cell(row,frame.ingress||'—');cell(row,frame.core_sequence||'—');cell(row,frame.error);
      const td=document.createElement('td'),button=document.createElement('button');button.textContent='查看';
      button.onclick=()=>{selected=i;render();};td.append(button);row.append(td);$('replayRows').append(row);
    });
    const points=key=>data.frames.map((frame,i)=>`${45+i*75},${125-frame[key][0]*45}`).join(' ');
    $('replayChart').innerHTML=`<svg viewBox="0 0 760 165" role="img" aria-label="对端与核心今多持仓，单位手，横轴观察步骤"><path d="M45 20V125H735" fill="none" stroke="#536172"/><text x="10" y="128" fill="#98a7b8">0</text><text x="10" y="38" fill="#98a7b8">2</text><polyline points="${points('peer_positions')}" fill="none" stroke="#4cc9b0" stroke-width="3"/><polyline points="${points('core_positions')}" fill="none" stroke="#e9b65c" stroke-width="3" stroke-dasharray="7 4"/><line x1="${45+selected*75}" x2="${45+selected*75}" y1="20" y2="132" stroke="#91b9f2"/><text x="45" y="154" fill="#98a7b8">观察步骤 1 → 10 · 逻辑顺序，非实测时间</text></svg>`;
    $('replayProvenance').textContent=`C++ 离线导出 · ${data.provenance.producer} · ${data.provenance.run} / ${data.provenance.configuration} · SHA256 ${data.provenance.sha256}`;
  }
  $('replayVariant').onchange=()=>{
    data=$('replayVariant').value==='query'?catalog.query_recovery:catalog;
    if(!data){$('replayStatus').textContent='查询场景未导出';return;}
    selected=0;render();
  };
  $('replayPrevious').onclick=()=>{selected=Math.max(0,selected-1);render();};
  $('replayNext').onclick=()=>{selected=Math.min(data.frames.length-1,selected+1);render();};
  $('replayProvenance').textContent=`C++ 离线导出 · ${data.provenance.producer} · ${data.provenance.run} / ${data.provenance.configuration} · SHA256 ${data.provenance.sha256}`;
  render();
})();
