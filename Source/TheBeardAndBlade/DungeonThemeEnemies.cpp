#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformMisc.h"

void ADungeonGameMode::FireThemeAttack(ADungeonEnemy* E)
{
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!H||!IsValid(E)||IsGameplayBlocked())return;
    const auto& Spec=DungeonRoster::Get(E->Species);
    const auto P=DungeonView::Project(E->GetActorLocation());
    const auto Delta=E->AttackTarget-P;
    if(Spec.AttackStyle==2)
    {
        E->ChargeTime=E->Species==44?.6f:.45f;PlaySound(TEXT("Roll"),.55f);return;
    }
    if(Spec.AttackStyle==0||Spec.AttackStyle==4)
    {
        FDungeonSplash FX;FX.Position=P;FX.Radius=Spec.Range;
        FX.Art=E->Species==31?50:E->Species==47?51:49;Splashes.Add(FX);
        PlaySound(Spec.AttackStyle==0?TEXT("Sword"):TEXT("Explosion"),.6f);
        auto D=DungeonView::Project(H->GetActorLocation())-P;D.Y/=.7f;
        if(D.Size()<Spec.Range+20)H->ReceiveHit(Spec.Damage*1.8f);
        return;
    }
    const bool Grenade=E->Species==39,Arc=E->Species==33||E->Species==42;
    const bool Bullet=E->Species==37||E->Species==40;
    const bool Radial=E->Species==48;
    const int Count=Radial?6:E->Species==34||E->Species==40||E->Species==46?3:1;
    const int Art=E->Species==33?40:E->Species==34?41:E->Species==36?42:E->Species==42?43:E->Species==43?44:E->Species==46?45:47;
    const float BodyHeight=E->Species==40?55.f:E->Species==34||E->Species==43||E->Species==48?60.f:70.f;
    const auto Muzzle=P+FVector2D((E->Facing==3?-1:1)*DungeonRoster::RenderSize(E->Species)*.2f,-BodyHeight);
    const auto AimDelta=E->AttackTarget-FVector2D(0,65)-Muzzle;
    const float Base=FMath::Atan2(AimDelta.Y,AimDelta.X);
    for(int I=0;I<Count;++I)
    {
        FDungeonShot S;S.Origin=S.Position=Muzzle;S.Target=DungeonView::Clamp(E->AttackTarget);S.HitHeight=Grenade||Arc?0:65;
        S.Style=Grenade?13:Bullet?11:Arc?14:15;S.Art=Grenade?20:Bullet?18:Art;
        S.Damage=Spec.Damage*1.8f;S.Radius=Bullet?5:10;S.BlastRadius=Grenade?180:Arc?85:28;
        const float A=Radial?I*2*PI/Count:Base+(I-(Count-1)*.5f)*.18f;
        S.Velocity=FVector2D(FMath::Cos(A),FMath::Sin(A))*(Bullet?950.f:E->Species==43?360.f:275.f);
        if(Grenade||Arc){S.FlightTime=.85f;S.Life=Grenade?1.65f:1.05f;}
        Shots.Add(S);
    }
    PlaySound(Bullet?TEXT("Rifle0"):Grenade||Arc?TEXT("Throw"):TEXT("Magic"),.55f);
}

void ADungeonGameMode::VerifyProgression()
{
    int Failures=0;auto Check=[&](bool Good,const TCHAR* What){if(!Good)++Failures;UE_LOG(LogTemp,Display,TEXT("PROGRESSION %s %s"),Good?TEXT("PASS"):TEXT("FAIL"),What);};
    const int Order[]={24,28,30,25,26,27,29};const int Themes[]={0,6,5,1,2,3,4};
    for(int R=1;R<=21;++R)
    {
        StartPlaytestRoom(R);Check(GetBossSpecies()==Order[(R-1)/3]&&GetBiome()==Themes[(R-1)/3],TEXT("Room boss and theme mapping"));
        Check(IsBossRoom()==(R%3==0),TEXT("Two standard rooms before every boss"));
        Tick(2.1f);Check(!Enemies.IsEmpty(),TEXT("Room spawns enemies"));
        for(auto& E:Enemies)Check(IsBossRoom()?E->bBoss&&E->Species==GetBossSpecies():!E->bBoss&&E->Species>=DungeonProgression::RosterBase(GetBiome())&&E->Species<DungeonProgression::RosterBase(GetBiome())+6,TEXT("Correct room roster"));
    }
    StartPlaytestRoom(4);Tick(2.1f);auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    for(int S=31;S<=48;++S)
    {
        for(int F=0;F<8;++F)Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/Progression/ThemeEnemy_%d_%d.ThemeEnemy_%d_%d"),S,F,S,F))!=nullptr,TEXT("Enemy animation texture imported"));
        auto* E=GetWorld()->SpawnActor<ADungeonEnemy>();E->Species=S;E->SpawnTime=0;E->SetActorLocation(DungeonView::Unproject(FVector2D(500,450)));E->AttackTarget=FVector2D(640,450);
        Shots.Empty();Splashes.Empty();if(H)H->Health=H->MaxHealth;FireThemeAttack(E);
        const int Style=DungeonRoster::Get(S).AttackStyle;
        Check(Style==2?E->ChargeTime>0:Style==0||Style==4?!Splashes.IsEmpty():!Shots.IsEmpty(),TEXT("Every new enemy has an executable attack"));E->Destroy();
        if(H&&!Shots.IsEmpty()&&Shots[0].Style!=13)
        {
            H->Restart();H->SetActorLocation(DungeonView::Unproject(FVector2D(640,450)));
            const auto TestShot=Shots[0];const float Before=H->Health;
            ResolveProjectile(TestShot,FVector2D(640,450-TestShot.HitHeight));
            Check(H->Health<Before,TEXT("Themed projectile contact applies damage"));
        }
    }
    if(H)
    {
        H->Restart();H->SetActorLocation(DungeonView::Unproject(FVector2D(640,450)));const float Before=H->Health;
        Shots.Empty();FDungeonShot Bullet;Bullet.Style=11;Bullet.HitHeight=65;Bullet.Position=FVector2D(500,385);Bullet.Velocity=FVector2D(950,0);Bullet.Damage=10;Shots.Add(Bullet);
        UpdateProjectiles(.2f);Check(H->Health<Before&&H->Health>=Before-10&&Shots.IsEmpty(),TEXT("Fast troop bullet uses swept collision and one armor-adjusted hit"));
        H->Restart();H->SetActorLocation(DungeonView::Unproject(FVector2D(640,450)));Shots.Empty();
        FDungeonShot Mortar;Mortar.Style=14;Mortar.Art=43;Mortar.Origin=Mortar.Position=FVector2D(500,450);Mortar.Target=FVector2D(640,450);Mortar.FlightTime=.85f;Mortar.Life=1.05f;Mortar.BlastRadius=85;Shots.Add(Mortar);
        const float Full=H->Health;UpdateProjectiles(.9f);Check(H->Health==Full&&!Shots.IsEmpty(),TEXT("Mortar does not detonate before fuse"));UpdateProjectiles(.2f);
        Check(H->Health<Full&&Shots.IsEmpty(),TEXT("Mortar fuse detonates once"));
    }
    for(int I=0;I<12;++I)Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/Progression/ThemeFX_%d.ThemeFX_%d"),I,I))!=nullptr,TEXT("Theme projectile texture imported"));
    Check(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Art/V2/Arena6.Arena6"))!=nullptr,TEXT("Greaseworks dungeon imported"));
    UE_LOG(LogTemp,Display,TEXT("PROGRESSION_VERIFY failures=%d"),Failures);FPlatformMisc::RequestExitWithStatus(true,Failures?1:0);
}
