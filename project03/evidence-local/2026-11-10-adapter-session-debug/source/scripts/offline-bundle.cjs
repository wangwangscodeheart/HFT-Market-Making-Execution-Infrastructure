const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),vm=require('node:vm');
const {verifyRun,hash}=require('./verify-run.cjs');
const generated=['core-snapshot.js','local-replay-data.js','quote-runner-data.js'];
function validateBundle(manifest,records){
  assert.equal(manifest.schema,'p03.offline-bundle.v1');assert.match(manifest.run,/^[a-zA-Z0-9-]+$/);
  assert.match(manifest.ui_manifest_sha256,/^[a-f0-9]{64}$/);
  const expected=new Map(manifest.files.map(r=>[r.path,r.sha256])),actual=new Map(records.map(r=>[r.path,r.sha256]));
  assert.equal(expected.size,manifest.files.length);assert.equal(actual.size,records.length);assert.equal(expected.size,actual.size);
  for(const f of ['index.html','runner.html','runner-charts.js',...generated])assert.ok(expected.has(f),`Missing bundle file ${f}`);
  for(const [file,digest] of expected){assert.equal(path.basename(file),file);assert.ok(file!=='.'&&file!=='..');assert.match(digest,/^[a-f0-9]{64}$/);assert.equal(actual.get(file),digest,`Bundle mismatch: ${file}`);}
}
function verifyBundle(dir){
  const manifest=JSON.parse(fs.readFileSync(path.join(dir,'bundle-manifest.json'),'utf8'));
  const records=fs.readdirSync(dir,{withFileTypes:true}).filter(e=>e.name!=='bundle-manifest.json').map(e=>{
    assert.ok(e.isFile(),'Only flat regular bundle files are allowed');return {path:e.name,sha256:hash(path.join(dir,e.name))};
  });
  validateBundle(manifest,records);
  for(const file of ['index.html','runner.html']){
    const html=fs.readFileSync(path.join(dir,file),'utf8');
    for(const m of html.matchAll(/(?:src|href)="([^"]+)"/g)){
      const ref=m[1];if(ref.startsWith('#')||/^https?:/.test(ref))continue;
      assert.equal(path.basename(ref),ref,'Local link must stay in bundle');assert.ok(records.some(r=>r.path===ref),`Unbundled dependency: ${ref}`);
    }
  }
  const context=vm.createContext({window:{}});
  for(const file of generated)vm.runInContext(fs.readFileSync(path.join(dir,file),'utf8'),context,{timeout:1000});
  const w=context.window;
  const datasets=[...Object.values(w.P03_CORE_CATALOG.cases),w.P03_LOCAL_REPLAY,w.P03_LOCAL_REPLAY.query_recovery,w.P03_QUOTE_RUNNER,
    ...['expiry','query_gap','retry','overflow','cleanup','late_fill','inventory'].map(k=>w.P03_QUOTE_RUNNER[k])];
  if(w.P03_QUOTE_RUNNER.feed)datasets.push(w.P03_QUOTE_RUNNER.feed);
  for(const data of datasets){assert.ok(data,'Missing bundle scenario');assert.equal(data.provenance.run,manifest.run,'Mixed run in generated bundle data');}
  return {manifest,records};
}
function prepareBundle(root,run){
  const verified=verifyRun(root,run);assert.ok(verified.ui,'UI-sealed run required');
  const dir=path.join(verified.dir,'offline-preview');assert.ok(!fs.existsSync(dir),'Bundle already exists; use a fresh run');fs.mkdirSync(dir);
  const live=path.resolve(root,'..','demos','round02');
  for(const r of verified.ui.expected)fs.copyFileSync(path.join(verified.dir,'ui-source',r.path),path.join(dir,r.path),fs.constants.COPYFILE_EXCL);
  for(const file of generated)fs.copyFileSync(path.join(live,file),path.join(dir,file),fs.constants.COPYFILE_EXCL);
  const files=fs.readdirSync(dir).sort().map(file=>({path:file,sha256:hash(path.join(dir,file))}));
  const manifest={schema:'p03.offline-bundle.v1',run,configuration:verified.status.configuration,ui_manifest_sha256:verified.status.ui_manifest_sha256,
    files,raw_json:verified.status.artifacts.filter(r=>r.path.endsWith('.json')&&!r.path.includes('/'))};
  fs.writeFileSync(path.join(dir,'bundle-manifest.json'),JSON.stringify(manifest,null,2),{flag:'wx'});
  verifyBundle(dir);return dir;
}
module.exports={prepareBundle,verifyBundle,validateBundle};
if(require.main===module){
  const dir=process.argv[2]==='--verify'?path.resolve(process.argv[3]):prepareBundle(path.resolve(__dirname,'..'),process.argv[2]||'');
  const result=verifyBundle(dir);console.log(`PASS: standalone offline bundle ${result.manifest.run}, ${result.records.length} files, hashes/local dependencies/data run identities verified.`);
  const clone=x=>JSON.parse(JSON.stringify(x));let controls=0;
  for(const change of [r=>r[0].sha256='0'.repeat(64),r=>r.pop(),r=>r.push({path:'extra.js',sha256:'a'.repeat(64)}),r=>r.push({...r[0]})]){
    const records=clone(result.records);change(records);assert.throws(()=>validateBundle(result.manifest,records));controls++;
  }
  console.log(`PASS: ${controls} bundle negative controls rejected changed/missing/extra/duplicate file records.`);
}
