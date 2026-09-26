#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ADungeonHero::PowerMove()
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    if(!G||G->IsGameplayBlocked()||IsInventoryOpen()||Health<=0||StunTime>0||IsRolling()||IsAttacking()||IsCasting()||PowerCooldown>0) return;
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
    if(S.Style==14||S.Style==15)
    {
        FDungeonSplash FX;FX.Position=At;FX.Art=S.Art==40?48:S.Art==42?50:S.Art==43||S.Art==46?51:49;
        FX.Radius=FMath::Max(30.f,S.BlastRadius);Splashes.Add(FX);
        PlaySound(S.Style==14?TEXT("Explosion"):TEXT("Magic"),.45f);
        if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
        {
            auto D=DungeonView::Project(H->GetActorLocation())-FVector2D(0,S.HitHeight)-At;D.Y/=.65f;
            if(D.Size()<=FMath::Max(S.Radius+20,S.BlastRadius)) H->ReceiveHit(S.Damage);
        }
        return;
    }
    if(S.Style==13)
    {
        FDungeonSplash FX;FX.Position=At;FX.Art=20;FX.Radius=175;Splashes.Add(FX);
        PlaySound(TEXT("FlashBang"),.65f);
        if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
            if(H->ApplyFlashBang(At,S.BlastRadius)&&S.SourceEnemy.IsValid())S.SourceEnemy->BeginFlashAmbush(H,this);
        return;
    }
    if(S.Style==10||S.Style==11)
    {
        AddBossFX(At,S.Style==10?16:18);
        if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
        {
            if(FVector2D::Distance(DungeonView::Project(H->GetActorLocation())-FVector2D(0,S.HitHeight),At)<S.Radius+20)
            {
                const float Before=H->Health; H->ReceiveHit(S.Damage,S.Style==11);
                if(S.Style==10&&H->Health<Before) H->ApplyBurgerStatus();
            }
        }
        return;
    }
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
        auto D=DungeonView::Project(H->GetActorLocation())-(S.Style==12?FVector2D(0,65):FVector2D::ZeroVector)-At; D.Y/=.65;
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
        const bool LinearEnemy=!S.bFriendly&&S.Style!=13&&S.Style!=14&&!S.Velocity.IsNearlyZero();
        if(LinearEnemy&&!S.bMotionTuned)
        {
            S.bMotionTuned=true;
            const float OldSpeed=S.Velocity.Size(),NewSpeed=FMath::Min(650.f,OldSpeed*.8f);
            S.Velocity*=NewSpeed/OldSpeed;
            S.Life*=OldSpeed/NewSpeed; // Retain range, not the old time-to-impact.
        }
        if(LinearEnemy&&!S.bTrackingStopped)
        {
            const FVector2D ToHero=HeroP-FVector2D(0,S.Style==12?65:S.HitHeight)-S.Position;
            // Once dodged, passed, or out of the steering window, never reacquire.
            if(H->IsRolling()||S.Age>=.55f||FVector2D::DotProduct(ToHero,S.Velocity)<=0) S.bTrackingStopped=true;
            else if(S.Style!=6&&S.Style!=12) // Radial barrages and flame cones keep their readable pattern.
            {
                const float Heading=FMath::Atan2(S.Velocity.Y,S.Velocity.X);
                const float Desired=FMath::Atan2(ToHero.Y,ToHero.X);
                const float Turn=FMath::Clamp(FMath::FindDeltaAngleRadians(Heading,Desired),-.8f*FMath::Min(Dt,.55f-S.Age),.8f*FMath::Min(Dt,.55f-S.Age));
                S.Velocity=FVector2D(FMath::Cos(Heading+Turn),FMath::Sin(Heading+Turn))*S.Velocity.Size();
            }
        }
        const float Step=FMath::Min(Dt,FMath::Max(0.f,S.Life));
        const auto Before=S.Position; S.Age+=Dt; S.Life-=Dt;
        if(S.bFriendly||S.Style==13||S.Style==14) S.Position=FMath::Lerp(S.Origin,S.Target,FMath::Clamp(S.Age/S.FlightTime,0.f,1.f));
        else S.Position+=S.Velocity*Step;
        bool Hit=false;
        if(!S.bFriendly&&S.Style!=13&&S.Style!=14)
        {
            const auto Segment=S.Position-Before;
            const auto HitCenter=HeroP-FVector2D(0,S.Style==12?65:S.HitHeight);
            const float T=Segment.IsNearlyZero()?0:FMath::Clamp(FVector2D::DotProduct(HitCenter-Before,Segment)/Segment.SizeSquared(),0.,1.);
            Hit=FVector2D::Distance(Before+Segment*T,HitCenter)<S.Radius+17;
            if(Hit) S.Position=Before+Segment*T;
        }
        const bool Wall=S.Position.X<65||S.Position.X>1215||S.Position.Y<145||S.Position.Y>755;
        if(Hit||S.Life<=0||Wall)
        {
            ResolveProjectile(S,(S.bFriendly||S.Style==13||S.Style==14)?S.Target:S.Position); S.Life=0;
        }
    }
    Shots.RemoveAll([](const FDungeonShot& S){return S.Life<=0||S.Position.X<65||S.Position.X>1215||S.Position.Y<145||S.Position.Y>755;});
}
