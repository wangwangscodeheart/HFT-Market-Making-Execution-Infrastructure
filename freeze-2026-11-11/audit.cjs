const fs=require('fs'),path=require('path'),crypto=require('crypto');
const root=path.resolve(__dirname,'..');
const hash=async p=>{const h=crypto.createHash('sha256');for await(const b of fs.createReadStream(p))h.update(b);return h.digest('hex');};
function walk(d){return fs.readdirSync(d,{withFileTypes:true}).flatMap(e=>{const p=path.join(d,e.name);if(e.isSymbolicLink())throw Error('Unreviewed link '+p);return e.isDirectory()?walk(p):[p];});}
async function main(){
 const files=['demos','handover_review','project03'].flatMap(d=>walk(path.join(root,d))).sort();
 const rows=[];for(const p of files)rows.push({path:path.relative(root,p).replaceAll('\\','/'),bytes:fs.statSync(p).size,sha256:await hash(p)});
 fs.writeFileSync(path.join(__dirname,'WORKSPACE_FILES.json'),JSON.stringify(rows,null,2));
 fs.writeFileSync(path.join(__dirname,'WORKSPACE_SHA256SUMS'),rows.map(r=>r.sha256+'  '+r.path+'\n').join(''));
 const base=path.join(root,'handover_review/PROJECT03_CODEX_HANDOVER');
 const old=JSON.parse(fs.readFileSync(path.join(base,'manifest/FILES.json'),'utf8')).files;
 const mismatches=[];for(const r of old){const p=path.join(base,r.path);if(!fs.existsSync(p)||await hash(p)!==r.sha256)mismatches.push(r.path);}
 const reviewed=files.filter(p=>/\.(md|hpp|cpp|cjs|ps1|json|txt)$/.test(p)&&!p.includes('evidence-local'));
 // Read every selected text input; this index supports inventory coverage, not a claim of formal semantic proof.
 const index=reviewed.map(p=>{const s=fs.readFileSync(p,'utf8');return {path:path.relative(root,p),bytes:Buffer.byteLength(s),headings:s.split(/\r?\n/).filter(s=>/^#{1,6} /.test(s)),markers:s.split(/\r?\n/).filter(s=>/NotImplemented|Blocked|TODO|FIXME/.test(s)).slice(0,50)};});
 fs.writeFileSync(path.join(__dirname,'AUDIT_TEXT_INDEX.json'),JSON.stringify(index,null,2));
 const summary={files:rows.length,bytes:rows.reduce((n,r)=>n+r.bytes,0),handover_manifest_entries:old.length,handover_mismatches:mismatches,git_commit:null,git_status:'No .git repository found; cannot establish tracked/untracked or clean/dirty state',text_files_indexed:index.length};
 fs.writeFileSync(path.join(__dirname,'AUDIT_RESULT.json'),JSON.stringify(summary,null,2));console.log(summary);
}
main().catch(e=>{console.error(e);process.exitCode=1});
