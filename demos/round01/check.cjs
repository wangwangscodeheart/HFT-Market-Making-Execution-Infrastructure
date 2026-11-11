// Smoke-check explanatory navigation without a browser; not a C++ business test.
const fs=require('node:fs');
const vm=require('node:vm');
const assert=require('node:assert/strict');
const nodes={};
const buttons=['','light','learn'].map(theme=>({dataset:{theme},setAttribute(){},classList:{contains:()=>theme==='',toggle(){}}}));
const document={body:{},getElementById(id){return nodes[id]??={};},querySelectorAll(){return buttons;}};
const context=vm.createContext({document});
vm.runInContext(fs.readFileSync(require('node:path').join(__dirname,'demo.js'),'utf8'),context);
assert.equal(nodes.counter.textContent,'1 / 8');
for(let step=1;step<8;step++){nodes.next.onclick();assert.match(nodes.invariant.textContent,/= 3/);}
assert.equal(nodes.long.textContent,2);
assert.equal(nodes.next.disabled,true);
nodes.prev.onclick();assert.equal(nodes.long.textContent,2);
buttons[1].onclick();assert.equal(document.body.className,'light');
buttons[2].onclick();assert.equal(document.body.className,'learn');
nodes.reset.onclick();assert.equal(nodes.long.textContent,0);assert.equal(nodes.prev.disabled,true);
console.log('PASS: 8 explanatory steps, quantity totals, dedup view, navigation, themes, reset. No browser rendering or C++ verification.');
