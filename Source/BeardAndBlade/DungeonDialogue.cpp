#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

namespace
{
    FVector2D BossBubblePosition(ADungeonGameMode* G,ADungeonHero* H)
    {
        FVector2D P=H?DungeonView::Project(H->GetActorLocation()):FVector2D(640,520);
        if(G->GetDialogueIndex()%2) for(auto& E:G->GetEnemies()) if(IsValid(E)&&E->bBoss) { P=DungeonView::Project(E->GetActorLocation()); break; }
        return FVector2D(FMath::Clamp(P.X+55.,20.,940.),FMath::Clamp(P.Y-200.,120.,550.));
    }
}

void ADungeonHero::CancelCombatActions()
{
    AttackTime=PowerCastTime=RollTime=0;
    bWalking=false; bAttackHit=true; bTeaReleased=true;
    QuipTime=QuipCooldown=0; AttackQuip.Empty();
    bInventoryOpen=false;
}

void ADungeonGameMode::BeginBossDialogue()
{
    static const TCHAR* Conversations[4][6]={
        {TEXT("Is this a dungeon or an open-plan office?"),
         TEXT("Finance Guy. Your survival isn't in this quarter's forecast."),
         TEXT("You've got twelve browser tabs open and not one exit plan."),
         TEXT("I'm diversifying. Stocks, bonds... blunt-force paperwork."),
         TEXT("Lovely. I've got a balanced portfolio: sword and hot tea."),
         TEXT("Then let's see how you handle a market crash.")},
        {TEXT("Someone's let the cobwebs get out of hand."),
         TEXT("Every thread leads to me, little wanderer."),
         TEXT("Any chance one leads to the kettle?"),
         TEXT("Your bones will hang beside it."),
         TEXT("Right. No biscuits for you, then."),
         TEXT("Come closer. The web is waiting.")},
        {TEXT("Bit chilly. Have you tried turning the heating on?"),
         TEXT("I am the Rime Empress. Even time freezes before me."),
         TEXT("That explains the service. My tea's gone cold."),
         TEXT("Kneel, and I shall preserve you forever."),
         TEXT("I prefer my adventures fresh, thanks."),
         TEXT("Then shatter beneath my crown.")},
        {TEXT("You're scorching the carpet."),
         TEXT("I am the Cinder Warden. All intruders become ash."),
         TEXT("All that fire, and you still can't make a decent brew."),
         TEXT("Your insolence will feed my furnace."),
         TEXT("Careful. This cup's hotter than it looks."),
         TEXT("Then let the final embers fall!")}
    };
    DialogueLines.Empty(); DialogueIndex=0; DialogueWait=.8f; BossGrace=0;
    for(const TCHAR* Line:Conversations[GetBiome()]) DialogueLines.Add(Line);
    Shots.Empty(); Splashes.Empty(); Impacts.Empty();
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0))) H->CancelCombatActions();
}
FString ADungeonGameMode::GetDialogueSpeaker() const
{
    return DialogueIndex%2?FString(DungeonRoster::Get(24+GetBiome()).Name):TEXT("THE ADVENTURER");
}
void ADungeonGameMode::AdvanceBossDialogue(bool Skip)
{
    if(bMenu||!IsBossDialogueActive()||!CanAdvanceDialogue()) return;
    DialogueIndex=Skip?DialogueLines.Num():DialogueIndex+1;
    DialogueWait=.18f;
    PlaySound(TEXT("UI"),.55f);
    if(!IsBossDialogueActive()) BossGrace=.75f;
}
void ADungeonHUD::DialogueClick()
{
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    auto* PC=GetOwningPlayerController(); float X,Y;
    if(!G||!PC||!PC->GetMousePosition(X,Y)||Scale<=0) return;
    const FVector2D P=(FVector2D(X,Y)-Offset)/Scale;
    if(P.X<0||P.Y<0||P.X>1280||P.Y>800) return;
    const auto B=BossBubblePosition(G,Cast<ADungeonHero>(PC->GetPawn()));
    G->AdvanceBossDialogue(P.X>=B.X+222&&P.X<=B.X+308&&P.Y>=B.Y+98&&P.Y<=B.Y+118);
}
void ADungeonHUD::DrawDialogue(ADungeonGameMode* G,ADungeonHero* H)
{
    const FLinearColor Ink(.035f,.045f,.055f),Paper(.98f,.92f,.74f),Gold(.9f,.65f,.25f);
    if(H->QuipTime>0&&!G->IsGameplayBlocked()&&H->Health>0)
    {
        const FVector2D P=DungeonView::Project(H->GetActorLocation());
        const float W=H->AttackQuip.Len()>10?260.f:130.f;
        const float X=FMath::Clamp(float(P.X-W*.5f),20.f,1260.f-W),Y=FMath::Max(100.f,float(P.Y)-182.f);
        Box(X+4,Y+5,W,54,Ink); Box(X,Y,W,54,Ink); Box(X+3,Y+3,W-6,48,Paper);
        for(int I=0;I<12;++I) Box(X+W*.5f+I*.4f,Y+53+I,17-I,1,Ink);
        Label(H->AttackQuip,X+16,Y+17,Ink,1.25f);
    }
    if(!G->IsBossDialogueActive()) return;
    const auto B=BossBubblePosition(G,H);
    const float X=B.X,Y=B.Y;
    Box(X+4,Y+5,320,124,FLinearColor(0,0,0,.45f));
    Box(X,Y,320,124,Ink); Box(X+3,Y+3,314,118,Paper);
    for(int I=0;I<14;++I) Box(X+12-I,Y+123+I,15-I,1,Ink);
    Label(G->GetDialogueSpeaker().ToUpper(),X+12,Y+12,Ink,.85f);
    TArray<FString> Words; G->GetDialogueLine().ParseIntoArrayWS(Words);
    FString Line; float LineY=Y+34;
    for(const auto& Word:Words)
    {
        const FString Candidate=Line.IsEmpty()?Word:Line+TEXT(" ")+Word;
        float Width=0,Height=0; GetTextSize(Candidate,Width,Height,GEngine->GetMediumFont(),.95f);
        if(Width>292&&!Line.IsEmpty()) { Label(Line,X+12,LineY,Ink,.95f); LineY+=17; Line=Word; }
        else Line=Candidate;
    }
    if(!Line.IsEmpty()) Label(Line,X+12,LineY,Ink,.95f);
    Label(FString::Printf(TEXT("%d/%d  %s"),G->GetDialogueIndex()+1,G->GetDialogueCount(),
        G->CanAdvanceDialogue()?(G->GetDialogueIndex()+1==G->GetDialogueCount()?TEXT("CLICK TO FIGHT"):TEXT("CLICK TO CONTINUE")):TEXT("...")),X+12,Y+103,Ink,.65f);
    Box(X+222,Y+98,86,20,Ink); Label(TEXT("SKIP / FIGHT"),X+230,Y+103,Paper,.65f);
}
