#include "DungeonActors.h"
#include "DungeonCombatBalance.h"
#include "DungeonRoster.h"
#include "AthleticHeroSockets.h"
#include "DungeonDescent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundWave.h"

void ADungeonGameMode::VerifyWeekend()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;auto Check=[&](bool OK,const TCHAR* Why){if(!OK){++Errors;UE_LOG(LogTemp,Error,TEXT("WEEKEND_FAIL %s"),Why);}};
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));if(!H)return;
    StartGame();PendingSpawns=1;H->SetActorLocation(DungeonView::Unproject(FVector2D(640,450)));
    const float HP=H->Health;
    Check(DungeonDescent::Size(0)==1&&DungeonDescent::Opacity(.25f)==1,TEXT("Door approach keeps full size and visibility"));
    Check(DungeonDescent::Opacity(.85f)==0&&DungeonDescent::Blackout(.8f)==0,TEXT("Hero disappears before screen fades"));
    Check(DungeonDescent::Size(.8f)>.79f&&DungeonDescent::StairDistance<=32.f&&DungeonDescent::Duration>=4.f,TEXT("Slow descent fades before extreme shrink or arch-height travel"));
    for(int I=1;I<=100;++I)
    {
        const float T=I/100.f;
        Check(DungeonDescent::Size(T)<=DungeonDescent::Size(T-.01f)+.0001f&&DungeonDescent::Opacity(T)<=DungeonDescent::Opacity(T-.01f)+.0001f,TEXT("Door shrink and fade are monotonic"));
    }
    for(int D=0;D<8;++D)
    {
        const FVector2D Aim(FMath::Cos(D*PI/4),FMath::Sin(D*PI/4));
        Check(DungeonCombatBalance::MeleeHits(Aim*125,Aim,128,false),TEXT("Every direction hits at 125px without overlap"));
        Check(!DungeonCombatBalance::MeleeHits(Aim*200,Aim,128,false),TEXT("Melee rejects beyond reach"));
        Check(!DungeonCombatBalance::MeleeHits(-Aim*125,Aim,128,false),TEXT("Melee rejects behind player"));
    }
    auto* Target=GetWorld()->SpawnActor<ADungeonEnemy>();Target->Species=0;Target->SpawnTime=0;
    Target->Health=Target->MaxHealth=1000;Target->SetActorLocation(DungeonView::Unproject(FVector2D(640,450)+H->GetAim()*125));Enemies.Add(Target);
    PlayerAttack(H);Check(Target->Health<1000&&H->Health==HP,TEXT("Real melee deals damage outside contact range"));
    Target->Destroy();Enemies.Empty();
    const float Before[]={100,80,25,20};const float Expected[]={25,20,6.25f,0};
    TArray<TObjectPtr<ADungeonEnemy>> Cases;
    for(int I=0;I<4;++I){auto* E=GetWorld()->SpawnActor<ADungeonEnemy>();E->Species=0;E->SpawnTime=0;E->MaxHealth=100;E->Health=Before[I];Enemies.Add(E);Cases.Add(E);}
    auto* Boss=GetWorld()->SpawnActor<ADungeonEnemy>();Boss->Species=24;Boss->bBoss=true;Boss->SpawnTime=0;Boss->Health=Boss->MaxHealth=100;Enemies.Add(Boss);
    FreedomKills=15;Check(ActivateFreedom(H),TEXT("Charged FREEDOM activates"));UpdateFreedom(1.5f);
    for(int I=0;I<4;++I)Check(FMath::IsNearlyEqual(Cases[I]->Health,Expected[I]),TEXT("Exact FREEDOM current-health / strict threshold result"));
    Check(Boss->Health==100&&Boss->FreedomImmuneTime>0,TEXT("Boss immune with status indicator"));
    Check(PendingSpawns==1&&Wave==1&&!bChest,TEXT("FREEDOM does not skip waves or clear surviving enemies"));
    UpdateFreedom(.1f);Check(Cases[0]->Health==25,TEXT("FREEDOM resolves once"));
    FreedomTime=0;
    FDungeonShot S;S.Position=FVector2D(300,400);S.Velocity=FVector2D(1000,0);S.Style=1;S.Life=3;Shots.Empty();Shots.Add(S);
    UpdateProjectiles(.05f);Check(Shots.Num()==1&&Shots[0].Velocity.Size()<=650.1&&Shots[0].Velocity.Y>0,TEXT("Slower bolt steers toward player"));
    H->Dodge();Check(FMath::IsNearlyEqual(H->Stamina,78.f),TEXT("Dodge costs 22"));UpdateProjectiles(.05f);
    Check(Shots[0].bTrackingStopped,TEXT("Dodge permanently cancels steering"));const auto Direction=Shots[0].Velocity;
    H->SetActorLocation(DungeonView::Unproject(FVector2D(200,600)));UpdateProjectiles(.05f);
    Check(Shots[0].Velocity.Equals(Direction,.01f),TEXT("Projectile never turns around after dodge"));
    Shots.Empty();S.Position=FVector2D(300,400);S.Velocity=FVector2D(200,0);S.Age=.6f;S.bMotionTuned=true;Shots.Add(S);UpdateProjectiles(.05f);
    Check(Shots[0].bTrackingStopped,TEXT("Expired tracking cannot reacquire"));
    for(float Dt:{1.f/30,1.f/60,.1f})
    {
        H->Restart();Shots.Empty();const auto P=DungeonView::Project(H->GetActorLocation());
        FDungeonShot Incoming;Incoming.Style=11;Incoming.Damage=50;Incoming.Position=P-FVector2D(140,0);Incoming.Velocity=FVector2D(650,0);Shots.Add(Incoming);
        H->MoveForward(1);H->Dodge();const float BeforeHit=H->Health;
        for(float T=0;T<1.f;T+=Dt){H->Tick(Dt);UpdateProjectiles(Dt);}
        Check(H->Health==BeforeHit,TEXT("Timed sideways dodge avoids tracked bullet at 10/30/60fps"));
    }
    H->Restart();H->UpdateStamina(1,true);Check(FMath::IsNearlyEqual(H->Stamina,82.f),TEXT("Sprint costs 18 per second"));
    for(int RoomNumber:{1,2,10,20})Check(FMath::IsNearlyEqual(DungeonCombatBalance::SpawnHealth(50,RoomNumber,false),50*1.15f*1.25f*(1+(RoomNumber-1)*.035f)),TEXT("Regular health retains room scaling"));
    Check(FMath::IsNearlyEqual(DungeonCombatBalance::SpawnHealth(760,3,true),760*1.15f*1.07f),TEXT("Boss health unchanged"));
    for(int I=0;I<8;++I){auto* T=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/Finance_%d.Finance_%d"),I,I));Check(T&&T->Source.GetSizeX()==384&&T->Filter==TF_Nearest&&T->NeverStream,TEXT("Finance high-resolution point-filtered nonstreaming frames"));}
    for(int Species=25;Species<=27;++Species)for(int Frame=0;Frame<8;++Frame)
    {
        auto* T=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/Creature_%d_%d.Creature_%d_%d"),Species,Frame,Species,Frame));
        Check(T&&T->Source.GetSizeX()==384&&T->Source.GetSizeY()==384&&T->Filter==TF_Nearest&&T->NeverStream,TEXT("All Webroot/Rime/Cinder frames are sharp 384px nonstreaming textures"));
    }
    for(int I=0;I<16;++I)
    {
        auto* T=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/TeaFX_%d.TeaFX_%d"),I,I));
        Check(T&&T->Source.GetSizeX()==384&&T->Filter==TF_Nearest&&T->NeverStream,TEXT("Tea FX preserve detail without streaming blur"));
    }
    auto CheckHeroTexture=[&](const FString& Name)
    {
        auto* T=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*Name,*Name));
        Check(T&&T->Source.GetSizeX()==384&&T->Source.GetSizeY()==384&&T->Filter==TF_Nearest&&T->NeverStream&&T->MipGenSettings==TMGS_NoMipmaps,TEXT("Athletic hero frame exists, 384px, nearest, no mips or streaming"));
    };
    for(int D=0;D<8;++D)
    {
        Check(AthleticHeroSockets::IdleFrame[D]>=0&&AthleticHeroSockets::IdleFrame[D]<6,TEXT("Idle uses a valid matching movement pose"));
        CheckHeroTexture(FString::Printf(TEXT("Athletic_Walk%s_%d_%d"),D%2?TEXT("Diagonal"):TEXT("Cardinal"),D/2,AthleticHeroSockets::IdleFrame[D]));
        for(int F=0;F<6;++F)
        {
            const int V=AthleticHeroSockets::AttackView(D,F),Pose=AthleticHeroSockets::AttackPose[D][F];
            Check(V>=0&&V<8&&Pose>=0&&Pose<6,TEXT("Right-handed attack remap stays within authored poses"));
            Check(!(V==1&&Pose==4)&&!(V==3&&Pose==3)&&!(V==5&&(Pose==2||Pose==5))&&!(V>=6&&(Pose==2||Pose==3)),TEXT("Left-arm attack poses are never selected"));
            for(const TCHAR* State:{TEXT("Walk"),TEXT("Attack")})CheckHeroTexture(FString::Printf(TEXT("Athletic_%s%s_%d_%d"),State,D%2?TEXT("Diagonal"):TEXT("Cardinal"),D/2,F));
            for(auto Socket:{AthleticHeroSockets::Walk[D][F],AthleticHeroSockets::Attack[D][F],AthleticHeroSockets::IdleHand(D)})
                Check(Socket.X>0&&Socket.X<128&&Socket.Y>0&&Socket.Y<128,TEXT("Hand anchors inside normalized hero frame"));
        }
    }
    Check(AthleticHeroSockets::IdleHand(7).X>64&&AthleticHeroSockets::IdleHand(4).X<64,TEXT("Rear-left right hand stays on far side; front right hand stays screen-left"));
    for(int D=0;D<4;++D)for(int F=0;F<8;++F)CheckHeroTexture(FString::Printf(TEXT("Athletic_Roll_%d_%d"),D,F));
    const float SavedVolume=GetMasterVolume();
    SetMasterVolume(-1,false);Check(GetMasterVolume()==0,TEXT("Volume clamps at silent"));
    SetMasterVolume(2,false);Check(GetMasterVolume()==1,TEXT("Volume clamps at full"));
    SetMasterVolume(.35f,false);Check(FMath::IsNearlyEqual(GetMasterVolume(),.35f),TEXT("Volume supports intermediate levels"));
    SetMasterVolume(SavedVolume,false);
    auto* Menu=LoadObject<USoundWave>(nullptr,TEXT("/Game/Audio/MusicMenu.MusicMenu"));
    Check(Menu&&Menu->Duration>130&&Menu->bLooping,TEXT("Full new menu track loops"));
    Check(LoadObject<USoundWave>(nullptr,TEXT("/Game/Audio/MusicEnding.MusicEnding"))!=nullptr,TEXT("Original ending music preserved"));
    UE_LOG(LogTemp,Display,TEXT("WEEKEND_VERIFY_COMPLETE errors=%d"),Errors);FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
