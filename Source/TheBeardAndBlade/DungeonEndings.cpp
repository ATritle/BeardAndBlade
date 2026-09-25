#include "DungeonActors.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ADungeonGameMode::FinishRun(bool Victory)
{
    if(HasEnding())return;
    EndState=Victory&&!IsDead()?2:1;EndTime=0;bEndingCleaned=false;
    bMenu=false;bShowControls=false;
    CancelBossIntro();DialogueLines.Empty();DialogueWait=BossGrace=0;
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))
    {
        H->CancelCombatActions();H->StunTime=H->SlowTime=H->FlashBlindTime=0;
    }
    // This can be called inside projectile/damage iteration. Defer container cleanup
    // to the next GameMode tick, rather than invalidating the caller's iterator.
    UE_LOG(LogTemp,Display,TEXT("RUN_END outcome=%s room=%d"),IsVictory()?TEXT("victory"):TEXT("death"),Room);
}

void ADungeonGameMode::UpdateEnding(float Dt)
{
    if(!bEndingCleaned)
    {
        bEndingCleaned=true;PendingSpawns=0;SpawnTimer=0;
        Shots.Empty();Splashes.Empty();Impacts.Empty();Potions.Empty();Blood.Empty();
        FreedomTime=0;TransitionTime=TransitionCooldown=0;
        bChest=bLootClaimed=bLootRolled=false;Reward=FRewardPresentation();LootTimer=0;
        for(auto& E:Enemies)if(IsValid(E))E->Destroy();Enemies.Empty();
    }
    EndTime+=FMath::Max(0.f,Dt);
}

void ADungeonGameMode::RestartFromEnding()
{
    // Prevent the final attack click / held confirm from accidentally skipping art.
    if(HasEnding()&&EndTime>=1.f)StartGame();
}

void ADungeonHUD::DrawEnding(ADungeonGameMode* G)
{
    Sprite(G->IsVictory()?TEXT("EndingVictory"):TEXT("EndingDeath"),0,0,1280,800);
    const FLinearColor Parchment(.96f,.87f,.69f);
    auto CenterLine=[&](const FString& Text,float Y,float FontScale)
    {
        float W=0,H=0;GetTextSize(Text,W,H,GEngine->GetMediumFont(),FontScale);
        Label(Text,640-W*.5f,Y,Parchment,FontScale);
    };
    if(G->IsVictory())
    {
        CenterLine(TEXT("Seven guardians silenced. Twenty-one chambers conquered."),607,.95f);
        CenterLine(TEXT("Through frost, fire and flying paperwork, the bearded adventurer kept his blade sharp"),633,.9f);
        CenterLine(TEXT("and his tea hotter. At last, the storm fell silent. He raised his cup to the dawn:"),657,.9f);
        CenterLine(TEXT("\"Not a bad day's work. Now, who's putting the kettle on?\""),681,.9f);
    }
    else
    {
        CenterLine(TEXT("\"The tea went cold. The legend didn't.\""),617,1.15f);
        CenterLine(TEXT("The dungeon has claimed this descent, but not the last word."),650,.95f);
        CenterLine(FString::Printf(TEXT("You reached chamber %d. Rise again, adventurer."),G->GetRoom()),678,.9f);
    }
    float MX=-100,MY=-100;if(auto* PC=GetOwningPlayerController())PC->GetMousePosition(MX,MY);
    const FVector2D Mouse=(FVector2D(MX,MY)-Offset)/Scale;
    for(int I=0;I<2;++I)
    {
        const float X=275+400*I;
        const bool Hover=G->GetEndingTime()>=1&&Mouse.X>=X&&Mouse.X<=X+330&&Mouse.Y>=716&&Mouse.Y<=764;
        Sprite(I?TEXT("Menu_EXIT"):TEXT("Menu_NEW_RUN"),X-8,702,346,76,Hover?FLinearColor(1.2f,1.2f,1.1f):FLinearColor::White);
    }
    Box(0,0,1280,800,FLinearColor(0,0,0,1-FMath::Clamp(G->GetEndingTime()/.65f,0.f,1.f)));
}

void ADungeonHUD::EndingClick()
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    auto* PC=GetOwningPlayerController();float X=0,Y=0;
    if(!G||!G->HasEnding()||G->GetEndingTime()<1||!PC||!PC->GetMousePosition(X,Y)||Scale<=0)return;
    const FVector2D P=(FVector2D(X,Y)-Offset)/Scale;
    if(P.Y<716||P.Y>764)return;
    if(P.X>=275&&P.X<=605)G->RestartFromEnding();
    else if(P.X>=675&&P.X<=1005)UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);
}

void ADungeonGameMode::RunEndingPreview()
{
    // Explicit packaged-release diagnostic. Inert during normal play.
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonEndingSmoke")))
    {
        static int Stage=0,Errors=0;const float Time=GetWorld()->GetTimeSeconds();
        auto* Hero=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
        auto Check=[&](bool OK){if(!OK)++Errors;};
        if(Stage==0&&Time>2){StartGame();Room=21;PendingSpawns=0;SpawnOneEnemy();Check(IsBossIntroActive());++Stage;}
        if(Stage==1&&Time>3)
        {
            DialogueWait=0;AdvanceBossDialogue(true);BossGrace=0;
            const auto Batch=Enemies;for(auto& E:Batch)if(IsValid(E)){E->SpawnTime=0;E->TakeDungeonDamage(100000);}
            Check(IsVictory()&&!bChest);++Stage;
        }
        if(Stage==2&&Time>4.5f){Check(IsVictory()&&Shots.IsEmpty()&&PendingSpawns==0);FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/ReleaseVictory.png"),false,false);++Stage;}
        if(Stage==3&&Time>6){RestartFromEnding();Check(!HasEnding()&&Room==1);Hero->ReceiveHit(100000);++Stage;}
        if(Stage==4&&Time>7.5f){Check(HasEnding()&&!IsVictory()&&IsGameplayBlocked());FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/ReleaseDeath.png"),false,false);++Stage;}
        if(Stage==5&&Time>9)
        {
            Hero->Confirm();Check(!HasEnding()&&Hero->Health==Hero->MaxHealth&&Room==1);
            for(auto* N:{TEXT("EndingDeath"),TEXT("EndingVictory")})Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/Endings/%s.%s"),N,N))!=nullptr);
            FFileHelper::SaveStringToFile(FString::Printf(TEXT("ENDING_SMOKE errors=%d; final boss kill; victory; death; fresh restarts\n"),Errors),*(FPaths::ProjectSavedDir()/TEXT("EndingSmokeTest.txt")));
            ++Stage;FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
        }
        return;
    }
#if !UE_BUILD_SHIPPING
    const bool Death=FParse::Param(FCommandLine::Get(),TEXT("DungeonDeathPreview"));
    const bool Victory=FParse::Param(FCommandLine::Get(),TEXT("DungeonVictoryPreview"));
    if(!Death&&!Victory)return;
    static int Stage=0;const float Time=GetWorld()->GetTimeSeconds();
    if(Stage==0&&Time>2)
    {
        StartPlaytestRoom(Victory?21:8);
        if(Death)if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)))H->Health=0;
        FinishRun(Victory);++Stage;
    }
    if(Stage==1&&Time>4.5f)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("ArtSource")/(Victory?TEXT("VictoryEndingReview.png"):TEXT("DeathEndingReview.png")),false,false);++Stage;
    }
    if(Stage==2&&Time>6)FPlatformMisc::RequestExit(false);
#endif
}

void ADungeonGameMode::VerifyEndings()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;auto Check=[&](bool OK,const TCHAR* S){if(!OK){++Errors;UE_LOG(LogTemp,Error,TEXT("ENDING_VERIFY %s"),S);}};
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!H){FPlatformMisc::RequestExitWithStatus(false,1);return;}
    StartGame();H->Health=0;Tick(.1f);
    Check(HasEnding()&&!IsVictory()&&IsGameplayBlocked(),TEXT("Death displays ending and blocks combat"));
    Check(PendingSpawns==0&&Shots.IsEmpty()&&Enemies.IsEmpty(),TEXT("Death cleans combat safely"));
    RestartFromEnding();Check(HasEnding(),TEXT("Input guard prevents accidental restart"));
    ToggleMenu();Check(!bMenu&&HasEnding(),TEXT("Pause cannot hide ending"));
    Tick(1);H->Confirm();Check(!HasEnding()&&H->Health==H->MaxHealth&&Room==1&&PendingSpawns>0,TEXT("Enter starts fresh run"));
    for(int R:{3,18,21})
    {
        StartPlaytestRoom(R);Tick(2.1f);DialogueWait=0;AdvanceBossDialogue(true);BossGrace=0;
        const auto Batch=Enemies;for(auto& E:Batch)if(IsValid(E)){E->SpawnTime=0;E->TakeDungeonDamage(100000);}
        if(R<21)Check(!HasEnding()&&bChest,TEXT("Earlier bosses still award chests"));
        else
        {
            Check(IsVictory()&&IsGameplayBlocked()&&!bChest,TEXT("Real final boss kill triggers victory"));
            FinishRun(false);Check(IsVictory(),TEXT("Outcome is latched exactly once"));
            Tick(1.1f);NextRoom();Check(Room==21&&!IsTransitioning(),TEXT("No room 22 after campaign victory"));
            const float Health=H->Health;H->ReceiveHit(10000);H->Attack();Check(H->Health==Health&&!H->IsAttacking(),TEXT("Ending blocks attacks and incoming damage"));
            RestartFromEnding();Check(!HasEnding()&&Room==1&&H->Inventory.IsEmpty(),TEXT("Victory restart clears previous run"));
        }
    }
    for(auto* N:{TEXT("EndingDeath"),TEXT("EndingVictory")})Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/Endings/%s.%s"),N,N))!=nullptr,TEXT("Ending artwork imported"));
    UE_LOG(LogTemp,Display,TEXT("ENDING_VERIFY_COMPLETE errors=%d"),Errors);FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
