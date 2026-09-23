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
        SpeechBubble(FVector2D(X,Y),FVector2D(W,54),P-FVector2D(0,114));
        Label(H->AttackQuip,X+16,Y+17,Ink,1.25f);
    }
    if(!G->IsBossDialogueActive()) return;
    const auto B=BossBubblePosition(G,H);
    const float X=B.X,Y=B.Y;
    FVector2D Speaker=DungeonView::Project(H->GetActorLocation())-FVector2D(0,114);
    if(G->GetDialogueIndex()%2) for(auto& E:G->GetEnemies()) if(IsValid(E)&&E->bBoss)
        { Speaker=DungeonView::Project(E->GetActorLocation())-FVector2D(0,DungeonRoster::RenderSize(E->Species)*.76f); break; }
    SpeechBubble(B,FVector2D(320,124),Speaker);
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

void ADungeonHUD::SpeechBubble(FVector2D P,FVector2D S,FVector2D Speaker)
{
    const FLinearColor Ink(.035f,.045f,.055f),Paper(.98f,.92f,.74f);
    auto Round=[&](FVector2D Q,FVector2D Size,float Radius,FLinearColor Color)
    {
        for(int Y=0;Y<FMath::CeilToInt(Size.Y);++Y)
        {
            const float Edge=FMath::Min(float(Y)+.5f,float(Size.Y)-Y-.5f);
            const float Inset=Edge<Radius?Radius-FMath::Sqrt(FMath::Max(0.f,Radius*Radius-FMath::Square(Radius-Edge))):0;
            Box(Q.X+Inset,Q.Y+Y,Size.X-2*Inset,1,Color);
        }
    };
    // Choose the nearest side so the pointer continues to track the actual speaker.
    const FVector2D Center=P+S*.5;
    FVector2D Direction=(Speaker-Center).GetSafeNormal();
    const double EdgeScale=FMath::Min((S.X*.5-15)/FMath::Max(.001,FMath::Abs(Direction.X)),(S.Y*.5-15)/FMath::Max(.001,FMath::Abs(Direction.Y)));
    const FVector2D Base=Center+Direction*EdgeScale;
    const FVector2D Tip=Speaker;
    const int Steps=FMath::Max(1,FMath::CeilToInt(FVector2D::Distance(Base,Tip)));
    Round(P+FVector2D(3,4),S,16,FLinearColor(0,0,0,.4f));
    for(int I=0;I<Steps;++I)
    {
        const float T=float(I)/Steps,R=10*(1-T);
        const auto Q=FMath::Lerp(Base,Tip,T);
        Box(Q.X-R,Q.Y-R,2*R+1,2*R+1,Ink);
    }
    for(int I=0;I<Steps-3;++I)
    {
        const float T=float(I)/Steps,R=FMath::Max(0.f,7*(1-T)-.5f);
        const auto Q=FMath::Lerp(Base,Tip,T);
        Box(Q.X-R,Q.Y-R,2*R+1,2*R+1,Paper);
    }
    Round(P,S,16,Ink); Round(P+FVector2D(3,3),S-FVector2D(6,6),13,Paper);
}
