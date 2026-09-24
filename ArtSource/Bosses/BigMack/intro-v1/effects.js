(()=>{
const scene=document.querySelector('.stage'),boss=scene.querySelector('.boss');
const back=document.createElement('canvas'),front=document.createElement('canvas');
for(const [c,z] of [[back,1],[front,2]]){c.width=640;c.height=360;c.style.cssText=`position:absolute;inset:0;width:100%;height:100%;pointer-events:none;image-rendering:pixelated;z-index:${z}`;c.setAttribute('aria-hidden','true');scene.appendChild(c);}
boss.style.zIndex=1;
scene.insertBefore(back,boss);
const bg=back.getContext('2d'),fg=front.getContext('2d');let raf=0,start=0;
function rand(n){const s=Math.sin(n*127.1+311.7)*43758.5453;return s-Math.floor(s);}
function dot(ctx,x,y,s,col,a){ctx.globalAlpha=Math.max(0,a);ctx.fillStyle=col;ctx.fillRect(Math.round(x),Math.round(y),s,s);}
function rock(ctx,x,y,s,angle,a){ctx.save();ctx.translate(Math.round(x),Math.round(y));ctx.rotate(angle);const kind=Math.floor(s*10)%3;if(kind===0){dot(ctx,-s/2,-s/2,s,'#417a19',a);dot(ctx,-s*.7,-s*.2,s*.6,'#68a825',a);dot(ctx,-s*.1,-s*.65,s*.5,'#88bd37',a);dot(ctx,-s*.15,-s*.4,1,'#c5d95a',a);}else if(kind===1){dot(ctx,-s/2,-s/2,s,'#eaa02b',a);dot(ctx,-s*.4,-s*.4,s*.7,'#ffda49',a);dot(ctx,s*.2,s*.3,s*.3,'#fbbb30',a);}else{dot(ctx,-s/2,-s/2,s*.75,'#713e24',a);dot(ctx,-s*.4,-s*.4,s*.4,'#b1783b',a);}ctx.restore();}
function burst(t,when,cx,cy,count,power,fade){const age=t-when;if(age<0||age>1.05)return;for(let i=0;i<count;i++){const a=rand(i+when)*Math.PI*2,v=(.4+rand(i+50))*power;const x=cx+Math.cos(a)*age*v,y=cy+Math.sin(a)*age*v*.28+age*age*35;rock(fg,x,y,2+rand(i+22)*4,a+age*5,(1-age/1.05)*fade);}}
function frame(now){const t=(now-start)/1000;bg.clearRect(0,0,640,360);fg.clearRect(0,0,640,360);if(t>6.05)return;
if(!scene.classList.contains('reduced')&&t>.3){
const r=boss.getBoundingClientRect(),s=scene.getBoundingClientRect();const cx=(r.left+r.width*.5-s.left)/s.width*640,cy=(r.top+r.height*.84-s.top)/s.height*360;
const fade=Math.min(1,(t-.3)*3,Math.max(0,(6-t)/.8));
for(let i=0;i<60;i++){const a=t*(2.6+rand(i)*1.6)+i*2.399;const radius=24+rand(i+100)*68;const y=cy+Math.sin(a)*radius*.22-(rand(i+70)*35);const x=cx+Math.cos(a)*radius;const ctx=Math.sin(a)<0?bg:fg;const size=3+Math.floor(rand(i+200)*8);dot(ctx,x,y,size,'#b49a75',fade*(.08+rand(i+10)*.2));dot(ctx,x+size*.6,y+size*.3,size*.7,'#6d6557',fade*.16);}
for(let i=0;i<14;i++){const a=t*(2+rand(i+300))+i*2.4,rad=45+rand(i+400)*48;const x=cx+Math.cos(a)*rad,y=cy-12+Math.sin(a)*rad*.3-((t*.22+rand(i+600))%1)*50;rock(Math.sin(a)<0?bg:fg,x,y,3+rand(i+500)*5,a,fade*.85);}
// Segmented curved wind arcs stay low, away from eyes and rifles.
for(let j=0;j<3;j++){const a=t*3+j*2.1;for(let k=0;k<20;k++){const q=a+k*.045,rad=48+j*16;dot(Math.sin(q)<0?bg:fg,cx+Math.cos(q)*rad,cy+Math.sin(q)*rad*.24-j*10,2,k%4===0?'#f6c878':'#ba7945',fade*.28*(k/20));}}
if(t<1.55){for(let i=0;i<18;i++){const y=cy-130+rand(i+800)*130,x=cx-35-rand(i+900)*150;dot(bg,x,y,15+rand(i+1000)*70,i%3===0?'#ffbc5c':'#ae542e',fade*.28);}}
burst(t,1.5,340,270,28,130,fade);burst(t,2.4,320,282,18,95,fade);
}bg.globalAlpha=fg.globalAlpha=1;raf=requestAnimationFrame(frame);}
new MutationObserver(()=>{if(scene.classList.contains('play')){cancelAnimationFrame(raf);start=performance.now();raf=requestAnimationFrame(frame);}else{cancelAnimationFrame(raf);bg.clearRect(0,0,640,360);fg.clearRect(0,0,640,360);}}).observe(scene,{attributes:true,attributeFilter:['class']});
})();
