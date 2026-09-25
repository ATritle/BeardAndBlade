#include "DungeonActors.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"

namespace
{
    // Species order differs from campaign order; keep art routing in one place.
    const TCHAR* IntroPrefix(int32 Species)
    {
        static const TCHAR* Names[]={TEXT("IntroFinance"),TEXT("IntroWebroot"),TEXT("IntroRime"),TEXT("IntroCinder"),TEXT("IntroMack"),TEXT("IntroTwister"),TEXT("IntroFlash")};
        return Names[FMath::Clamp(Species-24,0,6)];
    }
    FLinearColor IntroAccent(int32 Species,int32 Variant)
    {
        switch(Species)
        {
        case 24:return Variant%2?FLinearColor(.95f,.78f,.24f):FLinearColor(.72f,.86f,.59f);
        case 25:return Variant%2?FLinearColor(.26f,.64f,.16f):FLinearColor(.58f,.4f,.24f);
        case 26:return Variant%2?FLinearColor(.35f,.74f,1):FLinearColor(.8f,.94f,1);
        case 27:return Variant%2?FLinearColor(1,.24f,.035f):FLinearColor(1,.65f,.12f);
        case 28:return Variant%3==0?FLinearColor(.25f,.5f,.1f):Variant%3==1?FLinearColor(.95f,.65f,.12f):FLinearColor(.45f,.25f,.12f);
        case 30:return Variant%2?FLinearColor(.72f,.68f,.48f):FLinearColor(.5f,.55f,.42f);
        default:return FLinearColor(.55f,.49f,.4f);
        }
    }
}

void ADungeonGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
    CancelBossIntro();Super::EndPlay(Reason);
}

void ADungeonHero::SkipIntro()
{
    if(auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this)))
        if(!G->IsMenu()&&G->IsBossIntroActive()&&G->GetBossIntroTime()>.35f)G->FinishBossIntro();
}
void ADungeonHero::ToggleIntroMotion()
{
    if(auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this)))G->ToggleIntroMotion();
}
void ADungeonGameMode::ToggleIntroMotion()
{
    bIntroReducedMotion=!bIntroReducedMotion;
    GConfig->SetBool(TEXT("DungeonAccessibility"),TEXT("ReducedIntroMotion"),bIntroReducedMotion,GGameUserSettingsIni);
    GConfig->Flush(false,GGameUserSettingsIni);
}
void ADungeonGameMode::StartBossIntro()
{
    CancelBossIntro();
    if(!IsBossRoom()||GetBossSpecies()<24||GetBossSpecies()>30)return;
    // Complete texture initialization before starting the synchronized audio/timeline.
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0))
        if(auto* HUD=Cast<ADungeonHUD>(PC->GetHUD()))HUD->PreloadBossIntro(GetBossSpecies());
    BossIntroTime=0;
    GConfig->GetBool(TEXT("DungeonAccessibility"),TEXT("ReducedIntroMotion"),bIntroReducedMotion,GGameUserSettingsIni);
    if(auto* Cue=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/BossIntroCue.BossIntroCue")))
        IntroAudio=UGameplayStatics::SpawnSound2D(this,Cue,(bMusicMuted||bEffectsMuted)?0:.42f,1,0,nullptr,false,true);
    if(MusicComponent)MusicComponent->SetVolumeMultiplier(0);
}
void ADungeonGameMode::CancelBossIntro()
{
    BossIntroTime=-1;
    if(IntroAudio){IntroAudio->Stop();IntroAudio=nullptr;}
    if(MusicComponent)MusicComponent->SetVolumeMultiplier(bMusicMuted?0:.23f);
}
void ADungeonGameMode::FinishBossIntro()
{
    if(!IsBossIntroActive())return;
    CancelBossIntro();DialogueWait=.25f;
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))H->CancelCombatActions();
}
void ADungeonGameMode::UpdateBossIntro(float Dt)
{
    if(IsDead()){CancelBossIntro();DialogueLines.Empty();return;}
    if(IntroAudio){IntroAudio->SetPaused(bMenu);IntroAudio->SetVolumeMultiplier((bMusicMuted||bEffectsMuted)?0:.42f);}
    if(bMenu)return;
    if(MusicComponent)MusicComponent->SetVolumeMultiplier(0);
    BossIntroTime+=FMath::Max(0.f,Dt);DialogueWait=FMath::Max(0.f,DialogueWait-Dt);
    if(BossIntroTime>=6)FinishBossIntro();
}

void ADungeonHUD::PreloadBossIntro(int32 Species)
{
    const FString Prefix=IntroPrefix(Species);
    Texture(Prefix+TEXT("Character"));Texture(Prefix+TEXT("Title"));
}

void ADungeonHUD::DrawBossIntro(ADungeonGameMode* G)
{
    const float T=G->GetBossIntroTime();const bool Reduced=G->IsIntroReducedMotion();
    const int32 Species=G->GetBossSpecies();
    const float Fade=FMath::Clamp((6-T)/.9f,0.f,1.f),Dim=FMath::Min(T/.3f,1.f)*Fade;
    Box(0,0,1280,800,FLinearColor(0,0,0,.58f*Dim));
    Box(0,0,1280,32*Dim,FLinearColor(.01f,.015f,.02f));Box(0,800-32*Dim,1280,32*Dim,FLinearColor(.01f,.015f,.02f));
    float X=640,Angle=0,Zoom=1;
    if(!Reduced)
    {
        if(T<.3f)X=-600;
        else if(T<1.5f){const float A=(T-.3f)/1.2f;X=FMath::Lerp(-600.f,680.f,1-FMath::Pow(1-A,1.6f));Angle=FMath::Lerp(-10.f,2.f,A);Zoom=FMath::Lerp(.9f,1.03f,A);}
        else if(T<1.68f){const float A=(T-1.5f)/.18f;X=FMath::Lerp(680.f,640.f,A);Angle=2*(1-A);Zoom=1+.03f*(1-A);}
        if(T>2.4f&&T<2.64f)X+=FMath::Sin((T-2.4f)*90)*4*(1-(T-2.4f)/.24f);
    }
    auto Noise=[](int I){return FMath::Frac(FMath::Abs(FMath::Sin(I*127.1f+311.7f)*43758.5453f));};
    auto Debris=[&](bool Front)
    {
        if(Reduced||T<.3f)return;
        for(int I=0;I<60;++I)
        {
            const float A=T*(2.6f+Noise(I)*1.6f)+I*2.399f;
            if((FMath::Sin(A)>=0)!=Front)continue;
            const float Rad=55+Noise(I+100)*120,Size=3+Noise(I+200)*9;
            const float PX=X+FMath::Cos(A)*Rad,PY=510+FMath::Sin(A)*Rad*.24f-Noise(I+70)*65-(Species==27?FMath::Fmod(T*35+I*7,65.f):0);
            const FLinearColor Color=IntroAccent(Species,I);
            const float W=Species==24?Size*1.4f:Size,H=Species==26?Size*1.5f:Size;
            Box(PX,PY,W,H,FLinearColor(Color.R,Color.G,Color.B,Fade*.65f));
            Box(PX,PY,W*.5f,H*.35f,FLinearColor(FMath::Min(1.f,Color.R+.2f),FMath::Min(1.f,Color.G+.2f),FMath::Min(1.f,Color.B+.2f),Fade*.7f));
            if(Species==24&&I%2==0)Box(PX+2,PY+H*.5f,W*.6f,1,FLinearColor(.15f,.28f,.12f,Fade*.6f));
        }
        if(!Front&&T<1.5f)for(int I=0;I<14;++I){auto C=IntroAccent(Species,I);C.A=Fade*.22f;Box(X-150-Noise(I)*220,200+Noise(I+11)*290,40+Noise(I+30)*90,2,C);}
        if(Front)for(float At:{1.5f,2.4f})
        {
            const float Age=T-At;if(Age<0||Age>1)continue;
            for(int I=0;I<22;++I){const float A=I*2.399f,V=80+Noise(I)*190;auto C=IntroAccent(Species,I);C.A=(1-Age)*Fade;Box(640+FMath::Cos(A)*Age*V,525+FMath::Sin(A)*Age*V*.3f+Age*Age*30,5,4,C);}
        }
    };
    Debris(false);
    const FString Prefix=IntroPrefix(Species);
    auto Fit=[&](const FString& Name,float CX,float CY,float MaxW,float MaxH,float Opacity,float Rotation)
    {
        if(auto* TextureAsset=Texture(Name))
        {
            const float Factor=FMath::Min(MaxW/TextureAsset->GetSizeX(),MaxH/TextureAsset->GetSizeY());
            const float W=TextureAsset->GetSizeX()*Factor,H=TextureAsset->GetSizeY()*Factor;
            Sprite(Name,CX-W/2,CY-H/2,W,H,FLinearColor(1,1,1,Opacity),Rotation);
        }
    };
    Fit(Prefix+TEXT("Character"),X,330,820*Zoom,540*Zoom,Fade*FMath::Clamp((T-.3f)/.15f,0.f,1.f),Angle);
    Debris(true);
    if(T>=2.1f)
    {
        const float A=FMath::Clamp((T-2.1f)/.3f,0.f,1.f),TX=Reduced?640:FMath::Lerp(-800.f,640.f,1-FMath::Pow(1-A,3.f));
        const float Pop=Reduced?1:1+.12f*(1-FMath::Clamp((T-2.4f)/.24f,0.f,1.f));
        Fit(Prefix+TEXT("Title"),TX,650,960*Pop,225*Pop,Fade*(Reduced?A:1),0);
    }
    if(!Reduced&&T>=2.4f&&T<2.55f){auto C=IntroAccent(Species,1);C.A=.10f*(1-(T-2.4f)/.15f);Box(0,0,1280,800,C);}
    Label(TEXT("CLICK / SPACE / ENTER TO SKIP"),38,758,FLinearColor(.9f,.85f,.7f,Fade),.7f);
    Label(Reduced?TEXT("R: REDUCED MOTION ON"):TEXT("R: REDUCED MOTION OFF"),982,758,FLinearColor(.9f,.85f,.7f,Fade),.7f);
}

void ADungeonGameMode::VerifyBossIntro()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;auto Check=[&](bool OK,const TCHAR* S){if(!OK){++Errors;UE_LOG(LogTemp,Error,TEXT("INTRO_VERIFY %s"),S);}};
    for(int R:{3,6,9,12,15,18,21})for(float Dt:{.016f,.3f,2.f})
    {
        StartPlaytestRoom(R);Tick(2.1f);Check(IsBossIntroActive()&&IsGameplayBlocked(),TEXT("Intro blocks combat"));
        const float Before=BossIntroTime;ToggleMenu();Tick(1);Check(BossIntroTime==Before,TEXT("Pause freezes timeline"));ToggleMenu();
        int Guard=0;while(IsBossIntroActive()&&++Guard<500)Tick(Dt);
        Check(!IsBossIntroActive()&&IsBossDialogueActive()&&IsGameplayBlocked(),TEXT("Completion restores dialogue, not premature combat"));
        FinishBossIntro();Check(IsBossDialogueActive(),TEXT("Finish is idempotent"));
        DialogueWait=0;AdvanceBossDialogue(true);Tick(1);Check(!IsGameplayBlocked(),TEXT("Dialogue ends into combat"));
        StartPlaytestRoom(R);Tick(2.1f);FinishBossIntro();Check(!IsBossIntroActive()&&IsBossDialogueActive(),TEXT("Skip preserves dialogue"));
        RestartRun();Check(!IsBossIntroActive()&&!IntroAudio,TEXT("Restart cancels intro and audio"));
    }
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
    {
        StartPlaytestRoom(6);Tick(2.1f);Tick(.5f);ToggleMenu();
        H->Confirm();Check(!bMenu&&IsBossIntroActive(),TEXT("Enter resumes paused intro without skipping"));
        H->SkipIntro();Check(!IsBossIntroActive()&&IsBossDialogueActive(),TEXT("Player skip hands off to dialogue"));
        StartPlaytestRoom(21);Tick(2.1f);H->Health=0;Tick(.1f);
        Check(!IsBossIntroActive()&&!IntroAudio&&!IsBossDialogueActive(),TEXT("Death cancels intro"));
    }
    StartPlaytestRoom(1);Tick(2.1f);Check(!IsBossIntroActive()&&!IsBossDialogueActive(),TEXT("Standard rooms do not trigger intros"));
    for(int32 Species=24;Species<=30;++Species)for(auto* Layer:{TEXT("Character"),TEXT("Title")})
    {
        const FString N=FString(IntroPrefix(Species))+Layer;
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/Intros/%s.%s"),*N,*N))!=nullptr,TEXT("Intro art loaded"));
    }
    Check(LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/BossIntroCue.BossIntroCue"))!=nullptr,TEXT("Intro cue loaded"));
    UE_LOG(LogTemp,Display,TEXT("INTRO_VERIFY_COMPLETE errors=%d"),Errors);FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
