// SPDX-FileCopyrightText: 2026 sanfor2004
// SPDX-License-Identifier: MIT

const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const sharp=require(process.env.MINT_WOODLAND_PET_SHARP_PATH||'sharp');
const root=path.resolve(__dirname,'../assets/mint-woodland-pet');
async function main(){
 const manifest=JSON.parse(fs.readFileSync(path.join(root,'manifest.json'),'utf8'));
 if(manifest.moods.length!==11)throw Error('Expected eleven moods');
 for(const mood of manifest.moods){
  if(mood.frames.length!==4||!fs.existsSync(path.join(root,mood.selectedSource)))throw Error('Incomplete mood '+mood.id);
  const unique=new Set();
  for(const filename of mood.frames){
   const {data,info}=await sharp(path.join(root,filename)).ensureAlpha().raw().toBuffer({resolveWithObject:true});
   if(info.width!==96||info.height!==96)throw Error('Frame dimensions '+filename);
   let filled=0,empty=0;
   for(let i=0;i<data.length;i+=4){
    const a=data[i+3];if(a===0){empty++;continue;}if(a!==255)throw Error('Nonbinary alpha');filled++;
    if(data[i]>110&&data[i+2]>110&&data[i]-data[i+1]>65&&data[i+2]-data[i+1]>65)throw Error('Chroma remains');
    const pixel=i/4,x=pixel%96,y=Math.floor(pixel/96);if(x<2||y<2||x>93||y>93)throw Error('Clipped frame');
   }
   if(filled<300||empty<300)throw Error('Empty or opaque rectangle '+filename);
   unique.add(crypto.createHash('sha256').update(data).digest('hex'));
  }
  if(unique.size<3)throw Error('Insufficient distinct frames '+mood.id);
  const strip=await sharp(path.join(root,mood.strip)).metadata();
  if(strip.width!==384||strip.height!==96||!strip.hasAlpha)throw Error('Invalid strip');
  const gif=await sharp(path.join(root,'qa',mood.id+'-loop.gif'),{animated:true}).metadata();
  if(gif.pages!==4||gif.loop!==0||gif.delay.length!==4)throw Error('Invalid loop '+mood.id);
 }
 const combined=await sharp(path.join(root,'qa/all-moods-loop-large.gif'),{animated:true}).metadata();
 if(combined.pages!==88||combined.loop!==0||combined.pageHeight!==288)throw Error('Combined loop invalid');
 const graph=JSON.parse(fs.readFileSync(path.resolve(root,'../../data/conversations.json'),'utf8')).nodes;
 if(graph.greeting.lines.length!==20)throw Error('Greeting count');
 const validMoods=new Set(manifest.moods.map(m=>m.id)),seen=new Set(),todo=['greeting'];
 while(todo.length){const id=todo.pop();if(seen.has(id))continue;const n=graph[id];if(!n||!validMoods.has(n.mood)||n.answers.length!==2||!n.lines.length)throw Error('Conversation '+id);seen.add(id);for(const a of n.answers)todo.push(a.next);}
 if(seen.size!==Object.keys(graph).length)throw Error('Unreachable conversation nodes');
 console.log('PASS: 44 RGBA frames, 11 strips, 11 four-frame loops, 88-frame combined loop, 20 greetings and '+seen.size+' reachable conversation nodes.');
}
main().catch(e=>{console.error(e);process.exitCode=1;});
