#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ADungeonHero::PowerMove()
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    if(!G||G->IsGameplayBlocked()||IsInventoryOpen()||Health<=0||IsRolling()||IsAttacking()||IsCasting()||PowerCooldown>0) return;
    const FVector2D P=DungeonView::Project(GetActorLocation());
    PowerTarget=P+Aim*300;
    if(auto* PC=Cast<APlayerController>(GetController()))
    {
        int W=0,H=0; float MX=0,MY=0; PC->GetViewportSize(W,H);
        const float S=FMath::Min(W/1280.f,H/800.f);
        if(S>0&&PC->GetMousePosition(MX,MY))
        {
            const auto Mouse=(FVector2D(MX,MY)-FVector2D((W-1280*S)/2,(H-800*S)/2))/S;
            if(Mouse.X>=0&&Mouse.X<=1280&&Mouse.Y>=0&&Mouse.Y<=800) PowerTarget=Mouse;
        }
    }
    PowerTarget=DungeonView::Clamp(P+(PowerTarget-P).GetClampedToMaxSize(450));
    PowerAim=(PowerTarget-P).GetSafeNormal(); if(PowerAim.IsNearlyZero()) PowerAim=Aim;
    PowerDirection=DungeonView::Direction(PowerAim); PowerCooldown=10.f; PowerCastTime=.48f; bTeaReleased=false;
}

void ADungeonGameMode::LaunchTea(ADungeonHero* H,FVector2D Target)
{
    if(!H||H->Health<=0) return;
    FDungeonShot S; S.bFriendly=true; S.Art=0; S.Style=8;
    S.Position=S.Origin=DungeonView::Project(H->GetActorLocation()); S.Target=DungeonView::Clamp(Target);
    S.FlightTime=FMath::Clamp((float)FVector2D::Distance(S.Origin,S.Target)/620.f,.35f,.75f);
    S.Life=S.FlightTime; S.BlastRadius=135; S.Damage=55+H->AttackPower*.8f;
    Shots.Add(S);
    PlaySound(TEXT("Throw"));
}

void ADungeonGameMode::ResolveProjectile(const FDungeonShot& S,FVector2D At)
{
    FDungeonSplash FX; FX.Position=At; FX.bFriendly=S.bFriendly; FX.Radius=FMath::Max(28.f,S.BlastRadius);
    FX.Art=S.bFriendly?8:S.Art==1||S.Art==2?9:S.Art==4?10:S.Art==5?11:S.Art==6?12:S.Art==7?13:15;
    Splashes.Add(FX);
    PlaySound(S.bFriendly?TEXT("TeaSplash"):S.Art==1||S.Art==2?TEXT("Paper"):S.Art==3?TEXT("Hit"):S.Art==6?TEXT("TeaSplash"):TEXT("Explosion"),S.bFriendly?1.f:.65f);
    if(S.bFriendly)
    {
        // Death callbacks remove enemies and may schedule a new wave. Use a snapshot.
        const auto Targets=Enemies;
        for(auto& E:Targets) if(IsValid(E)&&E->Health>0&&E->SpawnTime<=0)
        {
            auto D=DungeonView::Project(E->GetActorLocation())-At; D.Y/=.65;
            if(D.Size()<=S.BlastRadius+(E->bBoss?24:12)) E->TakeDungeonDamage(S.Damage);
        }
    }
    else if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
    {
        auto D=DungeonView::Project(H->GetActorLocation())-At; D.Y/=.65;
        if(D.Size()<=FMath::Max(S.Radius+20,S.BlastRadius)) H->ReceiveHit(S.Damage);
    }
}

void ADungeonGameMode::UpdateProjectiles(float Dt)
{
    for(auto& S:Splashes) S.Life-=Dt;
    Splashes.RemoveAll([](const FDungeonSplash& S){return S.Life<=0;});
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)); if(!H) return;
    const auto HeroP=DungeonView::Project(H->GetActorLocation());
    for(auto& S:Shots)
    {
        const float Step=FMath::Min(Dt,FMath::Max(0.f,S.Life));
        const auto Before=S.Position; S.Age+=Dt; S.Life-=Dt;
        if(S.bFriendly) S.Position=FMath::Lerp(S.Origin,S.Target,FMath::Clamp(S.Age/S.FlightTime,0.f,1.f));
        else S.Position+=S.Velocity*Step;
        bool Hit=false;
        if(!S.bFriendly)
        {
            const auto Segment=S.Position-Before;
            const float T=Segment.IsNearlyZero()?0:FMath::Clamp(FVector2D::DotProduct(HeroP-Before,Segment)/Segment.SizeSquared(),0.,1.);
            Hit=FVector2D::Distance(Before+Segment*T,HeroP)<S.Radius+17;
            if(Hit) S.Position=Before+Segment*T;
        }
        if(Hit||S.Life<=0)
        {
            ResolveProjectile(S,S.bFriendly?S.Target:S.Position); S.Life=0;
        }
    }
    Shots.RemoveAll([](const FDungeonShot& S){return S.Life<=0||S.Position.X<65||S.Position.X>1215||S.Position.Y<145||S.Position.Y>755;});
}
