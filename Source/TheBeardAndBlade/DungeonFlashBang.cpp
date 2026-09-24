#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"
#include "HAL/PlatformMisc.h"

namespace FlashBang
{
    // A 120 degree forward cone. Side-on and rear explosions are safely avoidable.
    bool Exposed(FVector2D Player,FVector2D Aim,FVector2D Explosion,float Radius)
    {
        const auto Delta=Explosion-Player;
        return Delta.SizeSquared()<=FMath::Square(Radius)&&
            (Delta.IsNearlyZero()||FVector2D::DotProduct(Aim.GetSafeNormal(),Delta.GetSafeNormal())>.5);
    }
}

bool ADungeonHero::ApplyFlashBang(FVector2D Explosion,float Radius)
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    if(Health<=0||!G||G->IsGameplayBlocked()||IsInventoryOpen()||G->IsFreedomActive()) return false;
    if(!FlashBang::Exposed(DungeonView::Project(GetActorLocation()),GetAim(),Explosion,Radius)) return false;
    // No chained stun extension while recovering from the same encounter's flash.
    if(FlashBlindTime>0) return false;
    CancelCombatActions(); StunTime=FMath::Max(StunTime,.6f);
    SlowTime=FMath::Max(SlowTime,2.5f); FlashBlindTime=2.2f;
    ReceiveHit(DungeonRoster::Get(30).Damage,true);
    return true;
}

void ADungeonHero::ReceiveFlashStab()
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    if(!G||G->IsGameplayBlocked()||G->IsFreedomActive()||Health<=0)return;
    // Percentage of current health at impact, not max health or armor-reduced base damage.
    const float Damage=Health*.25f;Health-=Damage;HurtTime=.18f;
    G->AddImpact(DungeonView::Project(GetActorLocation()),Damage,true);G->PlaySound(TEXT("Hurt"));
}

void ADungeonEnemy::BeginFlashAmbush(ADungeonHero* H,ADungeonGameMode* G)
{
    if(!H||!G||Species!=30||Health<=0||H->Health<=0||H->StunTime<=0||Action==2||G->IsGameplayBlocked())return;
    ActionFrom=DungeonView::Project(GetActorLocation());
    const auto Target=DungeonView::Project(H->GetActorLocation());
    ActionTo=DungeonView::Clamp(Target+FVector2D(Target.X>640?-55:55,0));
    Facing=Target.X<ActionTo.X?6:2;
    G->AddBossFX(ActionFrom,21);G->AddBossFX(ActionTo,21);
    SetActorLocation(DungeonView::Unproject(ActionTo));bWalking=false;
    Action=2;ActionTime=ActionDuration=.5f;G->PlaySound(TEXT("Portal"),.5f);
}

void ADungeonEnemy::TickFlashBoss(float Dt,ADungeonHero* H,ADungeonGameMode* G)
{
    if(SpawnTime>0) { SpawnTime=FMath::Max(0.f,SpawnTime-Dt);return; }
    const auto P=DungeonView::Project(GetActorLocation()),Target=DungeonView::Project(H->GetActorLocation());
    Facing=Target.X<P.X?6:2;
    if(Action==2)
    {
        const float Before=ActionTime;ActionTime=FMath::Max(0.f,ActionTime-Dt);
        if(Before>.32f&&ActionTime<=.32f&&H->Health>0)
        {
            H->ReceiveFlashStab();G->PlaySound(TEXT("Sword"),.75f);
        }
        if(ActionTime<=0)
        {
            G->AddBossFX(P,21);SetActorLocation(DungeonView::Unproject(ActionFrom));G->AddBossFX(ActionFrom,21);
            Action=0;Recovery=2.6f;
        }
        return;
    }
    if(ActionTime>0)
    {
        const float Before=ActionTime;
        ActionTime=FMath::Max(0.f,ActionTime-Dt);
        // Release exactly once, independently of animation frame rate.
        if(Before>.5f&&ActionTime<=.5f) G->ThrowFlashBang(this);
        if(ActionTime<=0) { Action=0;Recovery=2.6f; }
        return;
    }
    if(Recovery>0)
    {
        const float Step=FMath::Min(Dt,Recovery);Recovery=FMath::Max(0.f,Recovery-Dt);
        const auto Aim=(Target-P).GetSafeNormal();
        const float Distance=FVector2D::Distance(P,Target);
        auto Move=Distance<200?-Aim:Distance>360?Aim:FVector2D(-Aim.Y,Aim.X)*(AttackCount%2?1:-1);
        auto Next=DungeonView::Clamp(P+Move*220.f*Step*(SlowTime>0?.65f:1.f));
        Next.Y=FMath::Max(Next.Y,310.);
        bWalking=!Next.Equals(P,.01);WalkDistance+=FVector2D::Distance(Next,P);
        SetActorLocation(DungeonView::Unproject(Next));return;
    }
    ++AttackCount;Action=1;ActionTime=ActionDuration=1.2f;
    AttackTarget=DungeonView::Clamp(Target+H->ScreenVelocity*.15f);
}

void ADungeonGameMode::ThrowFlashBang(ADungeonEnemy* E)
{
    if(!IsValid(E)||E->Health<=0||IsGameplayBlocked()) return;
    FDungeonShot Shot;
    Shot.SourceEnemy=E;
    Shot.Style=13;Shot.Art=20;Shot.Damage=20;Shot.Radius=10;Shot.BlastRadius=300;
    Shot.Position=Shot.Origin=DungeonView::Project(E->GetActorLocation())+FVector2D(E->Facing==6?-35:35,-70);
    Shot.Target=DungeonView::Clamp(E->AttackTarget);
    Shot.FlightTime=.8f;Shot.Life=1.45f;Shot.Velocity=FVector2D::ZeroVector;
    Shots.Add(Shot);PlaySound(TEXT("Throw"),.6f);
}

void ADungeonHUD::UpdateFlashScreen()
{
    auto* Viewport=GetWorld()?GetWorld()->GetGameViewport():nullptr;
    if(!Viewport) return;
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    const float Remaining=H&&G&&H->Health>0&&!G->IsGameplayBlocked()&&!H->IsInventoryOpen()?H->FlashBlindTime:0;
    if(Remaining<=0)
    {
        if(FlashBlur.IsValid()) { Viewport->RemoveViewportWidgetContent(FlashBlur.ToSharedRef());FlashBlur.Reset();FlashWhite.Reset(); }
        return;
    }
    if(!FlashBlur.IsValid())
    {
        SAssignNew(FlashBlur,SBackgroundBlur).Padding(0).Visibility(EVisibility::HitTestInvisible)
            [SAssignNew(FlashWhite,SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).Padding(0)];
        Viewport->AddViewportWidgetContent(FlashBlur.ToSharedRef(),50);
    }
    // Slate blur includes the Canvas sprites, unlike camera post-processing.
    FlashBlur->SetBlurStrength(8.f*Remaining/2.2f);
    const float Age=2.2f-Remaining;
    FlashWhite->SetBorderBackgroundColor(FLinearColor(1,1,1,.82f*FMath::Exp(-Age*3.5f)));
}

void ADungeonHUD::EndPlay(const EEndPlayReason::Type Reason)
{
    if(FlashBlur.IsValid()) if(auto* V=GetWorld()->GetGameViewport()) V->RemoveViewportWidgetContent(FlashBlur.ToSharedRef());
    FlashBlur.Reset();FlashWhite.Reset();Super::EndPlay(Reason);
}

void ADungeonGameMode::VerifyFlashBang()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;
    auto Check=[&](bool OK,const TCHAR* What){if(!OK){++Errors;UE_LOG(LogTemp,Error,TEXT("FLASH_VERIFY: %s"),What);}};
    const FVector2D P(640,520);
    for(int D=0;D<8;++D)
    {
        const float A=D*PI/4;const FVector2D Facing(FMath::Sin(A),-FMath::Cos(A));
        Check(FlashBang::Exposed(P,Facing,P+Facing*200,300),TEXT("All eight front directions exposed"));
        Check(!FlashBang::Exposed(P,-Facing,P+Facing*200,300),TEXT("Turning away avoids flash"));
        Check(!FlashBang::Exposed(P,FVector2D(-Facing.Y,Facing.X),P+Facing*200,300),TEXT("Side-on avoids flash"));
        Check(!FlashBang::Exposed(P,Facing,P+Facing*301,300),TEXT("Out of radius avoids flash"));
    }
    StartGame();PendingSpawns=0;
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    Check(H!=nullptr,TEXT("Hero exists"));if(!H){FPlatformMisc::RequestExitWithStatus(false,1);return;}
    Check(!H->ApplyFlashBang(P-FVector2D(0,100),300),TEXT("Real hero looking away remains unaffected"));
    Check(H->FlashBlindTime==0&&H->StunTime==0&&H->SlowTime==0,TEXT("No effects on avoided flash"));
    FDungeonShot Grenade;Grenade.Style=13;Grenade.BlastRadius=300;
    Grenade.Position=Grenade.Origin=P;Grenade.Target=P+FVector2D(0,100);Grenade.FlightTime=.8f;Grenade.Life=1.45f;
    Shots.Add(Grenade);UpdateProjectiles(.9f);
    Check(Shots.Num()==1&&H->FlashBlindTime==0,TEXT("No premature flash when crossing player or landing"));
    UpdateProjectiles(.6f);
    Check(Shots.IsEmpty()&&H->FlashBlindTime>0&&H->StunTime>0&&H->SlowTime>0,TEXT("Fuse triggers facing-only status"));
    const float After=H->Health;UpdateProjectiles(1);
    Check(H->Health==After,TEXT("Detonation occurs once"));
    H->MoveRight(1);const auto Start=H->GetActorLocation();H->Tick(.3f);
    Check(H->GetActorLocation()==Start,TEXT("Flash stun blocks movement"));
    H->Tick(3);H->Tick(.01f);Check(H->FlashBlindTime==0&&H->StunTime==0&&H->SlowTime==0,TEXT("All effects expire"));
    H->Restart();H->ApplyFlashBang(P+FVector2D(0,100),300);H->Restart();
    Check(H->FlashBlindTime==0,TEXT("Restart clears blur"));
    for(int Index=0;Index<16;++Index)
    {
        FString Name=FString::Printf(TEXT("FlashGuy_%d"),Index);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/September/%s.%s"),*Name,*Name))!=nullptr,TEXT("Soldier frames imported"));
    }
    Check(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Art/V2/Arena5.Arena5"))!=nullptr,TEXT("Bunker imported"));
    StartPlaytestRoom(9);Tick(2.1f);
    Check(GetBossSpecies()==30&&GetBiome()==5&&Enemies.Num()==1,TEXT("Room 9 is bunker boss"));
    DialogueWait=0;AdvanceBossDialogue(true);BossGrace=0;
    auto* E=Enemies.IsEmpty()?nullptr:Enemies[0].Get();
    if(E)for(float Dt:{.016f,.3f,2.f})
    {
        Shots.Empty();E->SpawnTime=0;E->Action=1;E->ActionTime=E->ActionDuration=1.2f;E->AttackTarget=P;
        while(E->ActionTime>0)E->TickFlashBoss(Dt,H,this);
        Check(Shots.Num()==1,TEXT("Exactly one grenade per animation across frame rates"));
    }
    if(E)for(float Dt:{.016f,.3f,2.f})
    {
        H->Restart();H->Health=120;H->SetActorLocation(DungeonView::Unproject(P));H->SetFlashReviewAim(FVector2D(0,1));
        E->Action=0;E->SetActorLocation(DungeonView::Unproject(FVector2D(380,380)));
        const auto Home=E->GetActorLocation();FDungeonShot Owned;Owned.Style=13;Owned.SourceEnemy=E;Owned.BlastRadius=300;
        ResolveProjectile(Owned,P+FVector2D(0,100));
        Check(E->Action==2&&!E->GetActorLocation().Equals(Home),TEXT("Successful owned flash teleports boss to player"));
        const float StunnedHP=H->Health;
        while(E->Action==2)E->TickFlashBoss(Dt,H,this);
        Check(FMath::IsNearlyEqual(H->Health,StunnedHP*.75f),TEXT("Ambush stabs exactly once for quarter current health at every frame rate"));
        Check(E->GetActorLocation().Equals(Home,.01),TEXT("Ambush returns to exact previous position"));
        H->Restart();H->SetActorLocation(DungeonView::Unproject(P));H->SetFlashReviewAim(FVector2D(0,-1));
        ResolveProjectile(Owned,P+FVector2D(0,100));Check(E->Action!=2,TEXT("Avoided flash never triggers ambush"));
        H->Restart();H->SetActorLocation(DungeonView::Unproject(P));H->SetFlashReviewAim(FVector2D(0,1));
        Owned.SourceEnemy.Reset();ResolveProjectile(Owned,P+FVector2D(0,100));Check(E->Action!=2,TEXT("Unowned cadet grenade never triggers boss ambush"));
    }
    StartPlaytestRoom(21);Check(GetBossSpecies()==29&&GetBiome()==4,TEXT("Twister final and storm theme preserved"));
    UE_LOG(LogTemp,Display,TEXT("FLASH_VERIFY_COMPLETE errors=%d"),Errors);
    FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
