#pragma once
#include "CoreMinimal.h"

// Continuous deformation in the authored 128px frame; everything below the calves stays fixed.
// The body, held equipment and finger overlay must all use this same mapping.
namespace HeroBreathing
{
inline FVector2D Map(FVector2D P,float Phase,float Weight,float FacingX)
{
    const float Inhale=FMath::Pow(.5f-.5f*FMath::Cos(Phase),1.25f);
    const float Upper=FMath::SmoothStep(0.f,1.f,FMath::Clamp((100.f-float(P.Y))/56.f,0.f,1.f));
    const float Chest=FMath::Max(0.f,1.f-FMath::Abs(float(P.Y)-61.f)/32.f);
    const float Rise=Weight*(2.7f*Inhale+.28f*FMath::Sin(Phase*2));
    return FVector2D(64+(P.X-64)*(1+Weight*.045f*Inhale*Chest)+FacingX*.65f*Weight*Inhale*Upper,
        P.Y-Rise*Upper);
}
}
