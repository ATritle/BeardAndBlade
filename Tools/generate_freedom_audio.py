"""Original synthetic eagle-like screech, not a wildlife recording."""
from pathlib import Path
import numpy as np
import wave
out=Path(__file__).resolve().parents[1]/'Content/Audio'
sr=32000;t=np.arange(int(sr*1.8))/sr
freq=2100-1100*t/1.8+230*np.sin(t*36)
phase=2*np.pi*np.cumsum(freq)/sr
env=np.minimum(t/.04,1)*np.maximum(0,1-t/1.8)**.7
gate=.45+.55*np.sin(t*19)**2
rng=np.random.default_rng(17)
a=(np.sin(phase)+.3*np.sin(phase*2.02)+.1*rng.normal(size=len(t)))*env*gate
a=a/max(abs(a))*.7
with wave.open(str(out/'EagleScreech.wav'),'wb') as f:
    f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((a*32767).astype('<i2').tobytes())
