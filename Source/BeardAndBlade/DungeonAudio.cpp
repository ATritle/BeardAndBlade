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
