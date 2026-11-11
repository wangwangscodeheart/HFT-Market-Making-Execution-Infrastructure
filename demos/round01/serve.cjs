const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const files = {'/':['index.html','text/html; charset=utf-8'],'/index.html':['index.html','text/html; charset=utf-8'],'/demo.js':['demo.js','text/javascript; charset=utf-8']};
http.createServer((req,res)=>{
  const item=files[new URL(req.url,'http://localhost').pathname];
  if(!item){res.writeHead(404);res.end('Not found');return;}
  fs.readFile(path.join(__dirname,item[0]),(error,data)=>{
    if(error){res.writeHead(500);res.end('Read failed');return;}
    res.writeHead(200,{'Content-Type':item[1]});res.end(data);
  });
}).listen(8033,'127.0.0.1',()=>console.log('PROJECT03 preview: http://127.0.0.1:8033'));
