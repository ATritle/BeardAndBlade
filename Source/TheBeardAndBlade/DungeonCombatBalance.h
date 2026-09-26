#pragma once
#include "CoreMinimal.h"

namespace DungeonCombatBalance
{
constexpr float DodgeCost=22.f;
constexpr float SprintCost=18.f;
// Screen-space hurt volume used ONLY for receiving a weapon strike, not contact damage.
inline bool MeleeHits(FVector2D Delta,FVector2D Aim,float SpriteSize,bool Boss)
{
    const float Radius=FMath::Clamp(SpriteSize*.25f,32.f,Boss?80.f:52.f);
    const float Along=FVector2D::DotProduct(Delta,Aim.GetSafeNormal());
    const float Across=FMath::Abs(Delta.X*Aim.Y-Delta.Y*Aim.X);
    return Delta.Size()<=100.f+Radius&&Along>=-Radius*.45f&&Across<=Radius+FMath::Max(0.f,Along)*.8f;
}
inline float SpawnHealth(float Base,int Room,bool Boss)
{return Base*1.15f*(Boss?1.f:1.25f)*(1.f+(Room-1)*.035f);}
inline float FreedomDamage(float Current,float Maximum,bool Boss)
{return Boss?0.f:Current<Maximum*.25f?Current:Current*.75f;}
}
