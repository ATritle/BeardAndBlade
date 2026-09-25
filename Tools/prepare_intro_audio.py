"""Deterministic six-second PCM arrangement of the approved browser intro sketch."""
from pathlib import Path
import numpy as np
import wave
root=Path(__file__).resolve().parents[1];sr=44100
mix=np.zeros(sr*6,dtype=np.float64);rng=np.random.default_rng(17)
def tone(start,f,d,g,kind='sine',end=None):
    t=np.arange(round(sr*d))/sr;end=f if end is None else end
    freq=f*np.power(end/f,t/d);phase=np.cumsum(freq)/sr
    if kind=='triangle':v=2*np.abs(2*(phase%1)-1)-1
    elif kind=='saw':v=2*(phase%1)-1
    else:v=np.sin(2*np.pi*phase)
    env=np.minimum(t/.015,1)*np.exp(-7*t/d)*g
    at=round(start*sr);n=min(len(t),len(mix)-at);mix[at:at+n]+=v[:n]*env[:n]
def noise(start,d,g,rise):
    n=round(sr*d);v=rng.uniform(-1,1,n);v=np.convolve(v,np.ones(9)/9,mode='same')
    env=np.linspace(0,1,n) if rise else np.linspace(1,0,n)**2
    at=round(start*sr);mix[at:at+n]+=v*env*g
for i in range(6):
    tone(.45+i*.25,65.41,.2,.13,'triangle')
    if i%2==0:tone(.45+i*.25,110,.16,.22,end=38)
tone(.2,32.7,1.78,.15,'triangle',41.2);noise(.3,1.2,.3,True)
tone(1.5,95,.35,.24,end=35);noise(1.8,.6,.35,True)
tone(2.4,150,.8,.45,end=30);noise(2.4,1.25,.35,False)
for f in (65.41,77.78,98,130.81):tone(2.4,f,2.9,.09,'saw')
for i in range(6):tone(2.9+i*.4,110,.24,.2,end=40)
mix=np.tanh(mix*1.4);mix*=.8/max(.8,np.max(np.abs(mix)))
path=root/'Content/Audio/BossIntroCue.wav'
with wave.open(str(path),'wb') as w:
    w.setnchannels(1);w.setsampwidth(2);w.setframerate(sr);w.writeframes((mix*32767).astype('<i2').tobytes())
print(path)
