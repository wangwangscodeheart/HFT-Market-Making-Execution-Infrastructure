const fs=require('fs'),path=require('path'),crypto=require('crypto');
async function hash(p){const h=crypto.createHash('sha256');for await(const b of fs.createReadStream(p))h.update(b);return h.digest('hex');}
async function main(){
 const [restore,zip]=process.argv.slice(2);if(!restore||!zip)throw Error('restore directory and zip required');
 const source=path.resolve(__dirname,'..'),frozen=path.join(restore,'freeze-2026-11-11');
 const records=JSON.parse(fs.readFileSync(path.join(frozen,'WORKSPACE_FILES.json'))).concat(JSON.parse(fs.readFileSync(path.join(frozen,'PACKAGE_FILES.json'))));
 const failures=[];for(const r of records){const p=path.resolve(restore,r.path);if(!p.startsWith(path.resolve(restore)+path.sep))throw Error('Unsafe path');
   if(!fs.existsSync(p)||fs.statSync(p).size!==r.bytes||await hash(p)!==r.sha256)failures.push({path:r.path,kind:'extracted mismatch'});
   const current=path.join(source,r.path);if(!fs.existsSync(current)||await hash(current)!==r.sha256)failures.push({path:r.path,kind:'source changed after inventory'});
 }
 function list(d){return fs.readdirSync(d,{withFileTypes:true}).flatMap(e=>e.isDirectory()?list(path.join(d,e.name)):[path.relative(restore,path.join(d,e.name)).replaceAll('\\','/')]);}
 const extracted=list(restore),expected=new Set(records.map(r=>r.path).concat(['freeze-2026-11-11/PACKAGE_FILES.json']));
 for(const p of extracted)if(!expected.has(p))failures.push({path:p,kind:'unexpected extracted file'});
 if(extracted.length!==expected.size)failures.push({kind:'file count mismatch',expected:expected.size,actual:extracted.length});
 const manifest='freeze-2026-11-11/PACKAGE_FILES.json';if(await hash(path.join(source,manifest))!==await hash(path.join(restore,manifest)))failures.push({path:manifest,kind:'manifest mismatch'});
 const sha256=await hash(zip);const result={date:'2026-11-11 Asia/Tokyo',zip:path.basename(zip),bytes:fs.statSync(zip).size,sha256,extracted_files:extracted.length,hashed_records:records.length,failures,passed:failures.length===0,restore_directory:restore,manifest_self_hash_policy:'PACKAGE_FILES.json excluded from own list; identical extracted/source bytes checked; external ZIP SHA256 seals manifest'};
 fs.writeFileSync(path.join(source,'ARCHIVE_VERIFICATION.json'),JSON.stringify(result,null,2));
 fs.writeFileSync(path.join(source,'PROJECT03_FREEZE_2026-11-11.zip.sha256'),sha256+'  '+path.basename(zip)+'\n');
 console.log(JSON.stringify(result,null,2));if(failures.length)process.exitCode=1;
}
main().catch(e=>{console.error(e);process.exitCode=1});
