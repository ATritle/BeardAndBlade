#include "DungeonActors.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"

void ADungeonGameMode::PlaySound(const FString& Name,float Volume,float Pitch)
{
    if(bEffectsMuted||!GetWorld()) return;
    const double Now=GetWorld()->GetTimeSeconds();
    // Volley and AoE callbacks can fire many times in one frame. Limit each cue.
    if(const double* Last=LastSoundTime.Find(Name)) if(Now-*Last<.055) return;
    LastSoundTime.Add(Name,Now);
    auto& Sound=Sounds.FindOrAdd(Name);
    if(!Sound) Sound=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Audio/%s.%s"),*Name,*Name));
    if(Sound) UGameplayStatics::PlaySound2D(this,Sound,.48f*Volume,Pitch);
}
void ADungeonGameMode::UpdateAudio()
{
    const FString Next=bMenu?TEXT("MusicMenu"):IsBossRoom()?TEXT("MusicBoss"):TEXT("MusicDungeon");
    if(Next==MusicName) return;
    MusicName=Next;
    if(MusicComponent) { MusicComponent->FadeOut(.7f,0); MusicComponent=nullptr; }
    auto& Sound=Sounds.FindOrAdd(Next);
    if(!Sound) Sound=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Audio/%s.%s"),*Next,*Next));
    if(Sound)
    {
        MusicComponent=UGameplayStatics::CreateSound2D(this,Sound,1.f,1.f,0,nullptr,false,true);
        if(MusicComponent) MusicComponent->FadeIn(1.2f,bMusicMuted?0.f:.23f);
    }
}
void ADungeonGameMode::ToggleMusic()
{
    bMusicMuted=!bMusicMuted;
    if(MusicComponent) MusicComponent->AdjustVolume(.2f,bMusicMuted?0.f:.23f);
    GConfig->SetBool(TEXT("DungeonAudio"),TEXT("MuteMusic"),bMusicMuted,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);
}
void ADungeonGameMode::ToggleEffects()
{
    bEffectsMuted=!bEffectsMuted;
    GConfig->SetBool(TEXT("DungeonAudio"),TEXT("MuteEffects"),bEffectsMuted,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);
    if(!bEffectsMuted) PlaySound(TEXT("UI"));
}
void ADungeonHero::ToggleMusic() { if(auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this))) G->ToggleMusic(); }
void ADungeonHero::ToggleEffects() { if(auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this))) G->ToggleEffects(); }

// Explicit unattended release check; absent this flag it never alters gameplay.
void ADungeonGameMode::RunPackagedSmokeTest()
{
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonFreedomPreview")))
    {
        static int Step=0; const float T=GetWorld()->GetTimeSeconds();
        auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
        if(Step==0&&T>2&&H)
        {
            StartGame(); PendingSpawns=0;
            for(int I=0;I<4;++I) { SpawnOneEnemy(); Enemies.Last()->SpawnTime=0; }
            FreedomKills=15; ActivateFreedom(H); ++Step;
        }
        if(Step==1&&T>3.45f) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Freedom.png"),false,false); ++Step; }
        if(Step==2&&T>5.2f) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/FreedomAftermath.png"),false,false); ++Step; }
        if(Step==3&&T>6.f) { FPlatformMisc::RequestExit(false); ++Step; }
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonVitalsPreview")))
    {
        static int Stage=0; const float T=GetWorld()->GetTimeSeconds();
        auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
        if(Stage==0&&T>2&&H)
        {
            StartGame(); PendingSpawns=0; H->Health=85; H->SpendStamina(65);
            FDungeonPotion P; P.Position=FVector2D(760,530); Potions.Add(P); ++Stage;
        }
        if(Stage==1&&T>2.3f) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/VitalsHUD.png"),false,false); ++Stage; }
        if(Stage==2&&T>4) { ++Stage; FPlatformMisc::RequestExit(false); }
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonDialogueSmokeTest")))
    {
        static int Step=0,Errors=0;
        const float Time=GetWorld()->GetTimeSeconds();
        auto* Hero=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
        if(Step==0&&Time>2&&Hero) { StartGame(); Room=3; SpawnWave(); ++Step; }
        if(Step==1&&Time>5)
        {
            if(!IsBossDialogueActive()||!IsGameplayBlocked()) ++Errors;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/BossDialogue.png"),false,false); ++Step;
        }
        if(Step==2&&Time>6) { AdvanceBossDialogue(); ++Step; }
        if(Step==3&&Time>7)
        {
            if(DialogueIndex!=1||!IsGameplayBlocked()) ++Errors;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/BossReply.png"),false,false); ++Step;
        }
        if(Step==4&&Time>8) { AdvanceBossDialogue(true); ++Step; }
        if(Step==5&&Time>9&&Hero)
        {
            if(IsGameplayBlocked()) ++Errors;
            Hero->Attack(); if(!Hero->IsAttacking()) ++Errors;
            Hero->AttackQuip=TEXT("Take this you C*NT!"); Hero->QuipTime=1.5f;
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/AttackQuip.png"),false,false); ++Step;
        }
        if(Step==6&&Time>11)
        {
            FFileHelper::SaveStringToFile(FString::Printf(TEXT("DIALOGUE_SMOKE errors=%d; introduction; boss reply; skip; combat resume; quip render\n"),Errors),*(FPaths::ProjectSavedDir()/TEXT("DialogueSmokeTest.txt")));
            ++Step; FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
        }
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonLootSmoke")))
    {
        static int Step=0,Errors=0; const float Time=GetWorld()->GetTimeSeconds();
        auto* Hero=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)); if(!Hero) return;
        if(Step==0&&Time>2) {
            StartGame();Hero->AddToInventory(RollItem(8,4,4));Hero->AddToInventory(RollItem(28,3,4));Hero->AddToInventory(RollItem(41,2,4));
            Hero->Equip(RollItem(8,4,4));Hero->ToggleInventory();Hero->SelectedItem=INDEX_NONE;
            if(auto* PC=Cast<APlayerController>(Hero->GetController())) {
                if(auto* HUD=Cast<ADungeonHUD>(PC->GetHUD())) Errors+=HUD->VerifyInventoryGestures(Hero);
                int W=0,H=0;PC->GetViewportSize(W,H);float Scale=FMath::Min(W/1280.f,H/800.f);PC->SetMouseLocation((W-1280*Scale)/2+289*Scale,(H-800*Scale)/2+250*Scale);}
            for(int I=0;I<48;++I) if(!LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/Loot_%d.Loot_%d"),I,I))) ++Errors;
            if(!LoadObject<UTexture2D>(nullptr,TEXT("/Game/Art/V2/InventoryFrame.InventoryFrame"))) ++Errors;
            ++Step;
        }
        if(Step==1&&Time>4) {FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/LootHover.png"),false,false);++Step;}
        if(Step==2&&Time>5) {
            Hero->ToggleInventory();while(PendingSpawns>0){SpawnOneEnemy();--PendingSpawns;}
            int I=0;for(auto& E:Enemies) {E->SpawnTime=0;E->Windup=0;E->Recovery=20;E->Health=E->MaxHealth=1000;E->SetActorLocation(DungeonView::Unproject(FVector2D(400+I*140,420)));if(I++%2){E->BleedTime=3;E->BleedDPS=10;}else{E->PoisonTime=4;E->PoisonDPS=10;}}
            ++Step;
        }
        if(Step==3&&Time>6.2f){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/LootEffects.png"),false,false);++Step;}
        if(Step==4&&Time>10) {
            for(auto& E:Enemies) if(E->BleedTime>0||E->PoisonTime>0||E->Health>=1000) ++Errors;
            FFileHelper::SaveStringToFile(FString::Printf(TEXT("LOOT_SMOKE errors=%d; 49 textures; unselected hover card; ailment rendering and expiration\n"),Errors),*(FPaths::ProjectSavedDir()/TEXT("LootSmokeTest.txt")));
            ++Step;FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
        }
        return;
    }
    if(!FParse::Param(FCommandLine::Get(),TEXT("DungeonSmokeTest"))) return;
    static int Stage=0; const float T=GetWorld()->GetTimeSeconds();
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(Stage==0&&T>2) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/SmokeMenu.png"),false,false); ++Stage; }
    if(Stage==1&&T>3&&H) { StartGame(); ++Stage; }
    if(Stage==2&&T>4&&H) { H->PowerMove(); ++Stage; }
    if(Stage==3&&T>4.8f) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/SmokeGame.png"),false,false); ++Stage; }
    if(Stage==4&&T>7)
    {
        int Errors=0;
        for(const TCHAR* Name:{TEXT("MusicMenu"),TEXT("MusicDungeon"),TEXT("MusicBoss"),TEXT("Sword"),TEXT("Roll"),TEXT("Hit"),TEXT("Explosion"),TEXT("TeaSplash"),TEXT("Throw"),TEXT("Paper"),TEXT("Magic"),TEXT("Step"),TEXT("Hurt"),TEXT("Death"),TEXT("Chest"),TEXT("Equip"),TEXT("Portal"),TEXT("UI"),TEXT("Spawn")})
            if(!LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Audio/%s.%s"),Name,Name))) ++Errors;
        if(!H||H->GetPowerCooldown()<=0||bMenu||!MusicComponent||!MusicComponent->IsPlaying()) ++Errors;
        FFileHelper::SaveStringToFile(FString::Printf(TEXT("PACKAGED_SMOKE errors=%d; audio assets=19; menu->game; tea cooldown; music component playing\n"),Errors),*(FPaths::ProjectSavedDir()/TEXT("SmokeTest.txt")));
        ++Stage; FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
    }
}
