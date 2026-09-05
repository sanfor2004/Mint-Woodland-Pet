// Deterministic extraction/packaging of generated artwork; does not draw poses.
const fs = require('node:fs');
const path = require('node:path');
const sharp = require(process.env.MINT_WOODLAND_PET_SHARP_PATH || 'sharp');
const root = path.resolve(__dirname, '../assets/mint-woodland-pet');
const ids = ['idle','look','wave','happy','sad','angry','sleep','eat','drink','thirsty','thunder'];
const SIZE = 96;
const raw = (data,w=SIZE,h=SIZE) => sharp(data,{raw:{width:w,height:h,channels:4}});
const manifests = [], qa = [], allFrames = [], allDelays = [];
const chroma = (r,g,b) => r>110 && b>110 && r-g>65 && b-g>65;

function bounds(data,w,h) {
  let x0=w,y0=h,x1=-1,y1=-1,count=0;
  for(let y=0;y<h;y++) for(let x=0;x<w;x++) if(data[(y*w+x)*4+3]) {
    x0=Math.min(x0,x);y0=Math.min(y0,y);x1=Math.max(x1,x);y1=Math.max(y1,y);count++;
  }
  return {x0,y0,x1,y1,count};
}
function bodyBounds(data,w,h) {
  const seen=new Uint8Array(w*h); let best=[];
  for(let n=0;n<w*h;n++) {
    if(seen[n] || !data[n*4+3]) continue;
    const q=[n];seen[n]=1;
    for(let i=0;i<q.length;i++) {
      const k=q[i],x=k%w,y=Math.floor(k/w);
      for(const [dx,dy] of [[-1,0],[1,0],[0,-1],[0,1]]) {
        const nx=x+dx,ny=y+dy,j=ny*w+nx;
        if(nx>=0&&nx<w&&ny>=0&&ny<h&&!seen[j]&&data[j*4+3]){seen[j]=1;q.push(j);}
      }
    }
    if(q.length>best.length) best=q;
  }
  let x0=w,y0=h,x1=-1,y1=-1;
  for(const n of best){const x=n%w,y=Math.floor(n/w);x0=Math.min(x0,x);x1=Math.max(x1,x);y0=Math.min(y0,y);y1=Math.max(y1,y);}
  return {x0,y0,x1,y1};
}
async function loop(frames,delays,file) {
  await sharp(Buffer.concat(frames),{raw:{width:SIZE,height:SIZE*frames.length,channels:4,pageHeight:SIZE}})
    .gif({loop:0,delay:delays,dither:0,effort:7}).toFile(file);
}
async function main() {
 for(const id of ids) {
  const source=`sources/${id}/woodland-${['idle','happy','thunder'].includes(id)?'02':'01'}.png`;
  const {data,info}=await sharp(path.join(root,source)).ensureAlpha().raw().toBuffer({resolveWithObject:true});
  if(info.width%2 || info.height%2) throw Error('Expected even 2x2 source dimensions: '+id);
  const halfW=info.width/2,h=info.height/2;
  const frames=[]; const target=path.join(root,'moods',id); fs.mkdirSync(target,{recursive:true});
  for(let f=0;f<4;f++) {
   const oy=Math.floor(f/2)*h;
   // Find an actually empty separator near the nominal grid center. Generators
   // can place a tail a few pixels across the nominal halfway line.
   let split=halfW,bestDistance=Infinity,runStart=-1;
   const endSearch=Math.floor(info.width*.58);
   for(let x=Math.floor(info.width*.42);x<=endSearch;x++) {
    let occupied=false;
    for(let y=oy;y<oy+h;y++){const a=(y*info.width+x)*4;if(!chroma(data[a],data[a+1],data[a+2])){occupied=true;break;}}
    if(!occupied&&x<endSearch){if(runStart<0)runStart=x;}
    else if(runStart>=0){
      const center=Math.floor((runStart+x-1)/2);
      if(x-runStart>=6&&Math.abs(center-halfW)<bestDistance){split=center;bestDistance=Math.abs(center-halfW);}
      runStart=-1;
    }
   }
   if(bestDistance===Infinity)throw Error('No clean separator in '+id);
   const ox=f%2?split:0,w=f%2?info.width-split:split;
   const cell=Buffer.alloc(w*h*4);
   for(let y=0;y<h;y++)for(let x=0;x<w;x++){
    const a=((y+oy)*info.width+x+ox)*4,b=(y*w+x)*4;
    const r=data[a],g=data[a+1],blue=data[a+2];
    // Chroma exists only in the staging background. Preserve dark plum eyes.
    if(chroma(r,g,blue)) continue;
    data.copy(cell,b,a,a+4);cell[b+3]=255;
   }
   const bb=bodyBounds(cell,w,h), complete=bounds(cell,w,h);
   if(complete.count<1000)throw Error('Blank source '+id);
   if(complete.x0===0||complete.y0===0||complete.x1===w-1||complete.y1===h-1)throw Error('Source touches a cell edge: '+id+' '+f+' '+JSON.stringify({complete,w,h,split}));
   // A fixed scale for every frame, translation-only registration. No stretching.
   const scale=80/Math.max(halfW,h),cx=(bb.x0+bb.x1)/2;
   const hop=(id==='happy'||id==='thunder')&&f===2?5:0;
   const out=Buffer.alloc(SIZE*SIZE*4);
   const left=48-cx*scale,top=88-bb.y1*scale-hop;
   for(let y=0;y<SIZE;y++)for(let x=0;x<SIZE;x++) {
    const sx=Math.floor((x-left)/scale),sy=Math.floor((y-top)/scale);
    if(sx>=0&&sx<w&&sy>=0&&sy<h) cell.copy(out,(y*SIZE+x)*4,(sy*w+sx)*4,(sy*w+sx)*4+4);
   }
   const b=bounds(out,SIZE,SIZE);
   if(b.count<300||b.x0<2||b.y0<2||b.x1>93||b.y1>93)throw Error('Final clipping/empty '+id+' '+f+JSON.stringify(b));
   frames.push(out); await raw(out).png().toFile(path.join(target,`frame-0${f}.png`));
   qa.push({id,frame:f,alphaPixels:b.count,bounds:b,sourceBody:bb});
  }
  const strip=Buffer.alloc(SIZE*4*SIZE*4);
  for(let f=0;f<4;f++)for(let y=0;y<SIZE;y++) frames[f].copy(strip,(y*SIZE*4+f*SIZE)*4,y*SIZE*4,(y+1)*SIZE*4);
  await raw(strip,SIZE*4,SIZE).png().toFile(path.join(target,'strip.png'));
  const delay=id==='sleep'?450:id==='thunder'?160:240;
  await loop(frames,[delay,delay,delay,delay],path.join(root,'qa',id+'-loop.gif'));
  for(let repeat=0;repeat<2;repeat++) {allFrames.push(...frames);allDelays.push(delay,delay,delay,delay);}
  manifests.push({id,strip:`moods/${id}/strip.png`,frames:frames.map((_,f)=>`moods/${id}/frame-0${f}.png`),frameWidth:SIZE,frameHeight:SIZE,durationMs:[delay,delay,delay,delay],loop:true,selectedSource:source,reviewStatus:'pending-visual-review'});
 }
 await loop(allFrames,allDelays,path.join(root,'qa/all-moods-loop.gif'));
 await sharp(path.join(root,'qa/all-moods-loop.gif'),{animated:true}).resize({width:288,kernel:'nearest'}).gif({loop:0}).toFile(path.join(root,'qa/all-moods-loop-large.gif'));
 // Contact sheet: all frames, with rasterized text labels outside sprite cells.
 const overlays=[];
 for(let i=0;i<ids.length;i++) {
  const label=await sharp({text:{text:ids[i].toUpperCase(),font:'Segoe UI 14',rgba:true}}).png().toBuffer();
  overlays.push({input:label,left:16,top:i*208+16});
  for(let f=0;f<4;f++)overlays.push({input:await sharp(path.join(root,`moods/${ids[i]}/frame-0${f}.png`)).resize(192,192,{kernel:'nearest'}).png().toBuffer(),left:130+f*198,top:i*208+4});
 }
 await sharp({create:{width:936,height:208*ids.length,channels:4,background:'#f6f3e8'}}).composite(overlays).png().toFile(path.join(root,'qa/all-frames.png'));
 const cards=[];
 for(let i=0;i<ids.length;i++) {
  const idx=ids[i]==='idle'?0:2;
  cards.push({input:await sharp(path.join(root,`moods/${ids[i]}/frame-0${idx}.png`)).resize(192,192,{kernel:'nearest'}).png().toBuffer(),left:(i%4)*240+24,top:Math.floor(i/4)*240+8});
  cards.push({input:await sharp({text:{text:ids[i].toUpperCase(),font:'Segoe UI 16',rgba:true}}).png().toBuffer(),left:(i%4)*240+72,top:Math.floor(i/4)*240+206});
 }
 await sharp({create:{width:960,height:720,channels:4,background:'#f6f3e8'}}).composite(cards).png().toFile(path.join(root,'qa/all-moods-contact-sheet.png'));
 const reference=await sharp(path.join(root,'reference/mint-woodland-pet-original.png')).resize(250,250,{kernel:'nearest'}).png().toBuffer();
 const idle=await sharp(path.join(root,'moods/idle/frame-00.png')).resize(288,288,{kernel:'nearest'}).png().toBuffer();
 await sharp({create:{width:640,height:350,channels:4,background:'#f6f3e8'}}).composite([
  {input:reference,left:30,top:50},{input:idle,left:338,top:8},
  {input:await sharp({text:{text:'REFERENCE',font:'Segoe UI 16',rgba:true}}).png().toBuffer(),left:90,top:318},
  {input:await sharp({text:{text:'RUNTIME IDLE',font:'Segoe UI 16',rgba:true}}).png().toBuffer(),left:410,top:318}
 ]).png().toFile(path.join(root,'qa/reference-comparison.png'));
 fs.writeFileSync(path.join(root,'qa/geometry.json'),JSON.stringify({frames:qa,checks:'44 nonempty frames, safe margins, binary alpha, fixed scale and translation-only registration'},null,2));
 fs.writeFileSync(path.join(root,'manifest.json'),JSON.stringify({schemaVersion:2,reference:'reference/mint-woodland-pet-original.png',character:'Mint Woodland Pet',status:'pending-visual-review',runtimeIntegration:'pending',moods:manifests},null,2));
 console.log('Prepared 44 transparent frames, 11 strips, 12 loops and QA sheets.');
}
main().catch(e=>{console.error(e);process.exitCode=1;});
