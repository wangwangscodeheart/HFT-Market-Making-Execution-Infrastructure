// Browser-independent interaction checks. This does not claim rendered-browser QA.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const folder=process.argv[2]?path.resolve(process.argv[2]):path.resolve(__dirname,'..','..','demos','round02');
const html=fs.readFileSync(path.join(folder,'index.html'),'utf8');
class Element{
  constructor(){this.children=[];this.textContent='';this.dataset={};this.classList={toggle(){}};}
  append(child){this.children.push(child);}
  replaceChildren(...children){this.children=children;}
  setAttribute(name,value){this[name]=value;}
}
const nodes=new Map([...html.matchAll(/id="([^"]+)"/g)].map(m=>[m[1],new Element()]));
const charts=[...html.matchAll(/data-chart="([^"]+)"/g)].map(m=>{const e=new Element();e.dataset.chart=m[1];return e;});
const events=new Map();let interval=null;
const document={hidden:false,getElementById(id){assert.ok(nodes.has(id),`Missing HTML node ${id}`);return nodes.get(id);},createElement(){return new Element();},querySelectorAll(query){assert.equal(query,'[data-chart]');return charts;},addEventListener(type,fn){if(!events.has(type))events.set(type,[]);events.get(type).push(fn);},dispatchEvent(event){for(const fn of events.get(event.type)||[])fn(event);}};
const context=vm.createContext({window:{},document,CustomEvent:class{constructor(type,options){this.type=type;this.detail=options.detail;}},setInterval(fn){interval=fn;return 1;},clearInterval(){interval=null;}});
for(const file of ['core-snapshot.js','terminal-cases.js','trend-cases.js'])vm.runInContext(fs.readFileSync(path.join(folder,file),'utf8'),context,{filename:file});
const get=id=>nodes.get(id);
assert.equal(get('orderQuantity').textContent,10);
assert.equal(get('clientIdentity').textContent,'fixture.standard / A1a.request');
assert.equal(get('scopeStrategy').textContent,'fixture.default.strategy');
assert.equal(get('scenarios').children.length,6);
get('scenarios').children[5].onclick();
assert.equal(get('booked').textContent,'10 / 10');
assert.equal(get('positionRows').children[0].children[3].textContent,10);
assert.match(get('external').textContent,/Filled/);
assert.match(get('payload').textContent,/"admission": "BusinessInput"/);
assert.match(get('payload').textContent,/"ingress_sequence": 6/);
assert.equal(get('events').children[5].children.length,6);
assert.match(get('trendSummary').textContent,/已记账 10 手/);
charts[1].onclick();assert.match(get('trendChart').innerHTML,/10000 \/ 10/);
get('caseSelect').value='race';get('caseSelect').onchange();
assert.equal(get('clientIdentity').textContent,'fixture.demo / cancel-race.request');
assert.equal(get('orderQuantity').textContent,3);assert.equal(get('scenarios').children.length,8);
get('scenarios').children[4].onclick();assert.match(get('external').textContent,/MissingTrades/);
get('scenarios').children[6].onclick();assert.equal(get('booked').textContent,'2 / 3');assert.match(get('payload').textContent,/Duplicate/);
get('scenarios').children[7].onclick();assert.match(get('payload').textContent,/NotImplemented/);
get('scenarios').children[0].onclick();assert.equal(get('orderId').textContent,'—');
get('caseSelect').value='normal';get('caseSelect').onchange();get('playFrames').onclick();
assert.ok(interval);for(let i=0;i<4;i++)interval();assert.equal(interval,null);assert.equal(get('booked').textContent,'10 / 10');
assert.equal(get('sourceLink').hidden,false);
assert.equal((html.match(/id="canonical"/g)||[]).length,1);
console.log('PASS: HTML node bindings, both cases, all critical steps, position table, chart scaling/source labels, no-order state, and autoplay stop. Not a browser layout test.');
