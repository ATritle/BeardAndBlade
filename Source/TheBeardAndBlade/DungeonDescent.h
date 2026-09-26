#pragma once
#include "CoreMinimal.h"
namespace DungeonDescent
{
inline constexpr float Duration=4.f;
inline constexpr float StairDistance=32.f;
inline constexpr float StairGaitScale=8.f;
inline float Ease(float T,float Start,float End) { return FMath::SmoothStep(0.f,1.f,FMath::Clamp((T-Start)/(End-Start),0.f,1.f)); }
inline float Size(float T) { return FMath::Lerp(1.f,.80f,Ease(T,.25f,.82f)); }
inline float Opacity(float T) { return 1.f-Ease(T,.28f,.80f); }
inline float Blackout(float T) { return Ease(T,.82f,.98f); }
}
