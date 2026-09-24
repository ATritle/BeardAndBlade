#include "DungeonActors.h"
#include "Kismet/GameplayStatics.h"

void ADungeonHero::Freedom()
{
    if(auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this))) G->ActivateFreedom(this);
}

bool ADungeonGameMode::ActivateFreedom(ADungeonHero* H)
{
    if(!H||H->Health<=0||H->StunTime>0||H->IsInventoryOpen()||IsGameplayBlocked()||IsFreedomActive()||FreedomKills<15||bChest||bLootClaimed) return false;
    FreedomKills=0; FreedomTime=3.f; bFreedomResolved=false;
    H->CancelCombatActions();
    Shots.Empty();
    PlaySound(TEXT("EagleScreech"),1.f);
    // Voiceover intentionally deferred: the local speech engine denied synthesis.
    return true;
}

void ADungeonGameMode::UpdateFreedom(float Dt)
{
    if(!IsFreedomActive()) return;
    const float Remaining=FMath::Max(0.f,FreedomTime-Dt);
    // Resolve once, during the sweep. Snapshot protects against death callbacks mutating Enemies.
    if(!bFreedomResolved&&Remaining<=1.65f)
    {
        bFreedomResolved=true; PendingSpawns=0; Wave=2; Shots.Empty();
        PlaySound(TEXT("Explosion"),1.f);
        const auto Victims=Enemies;
        for(auto& E:Victims) if(IsValid(E))
        {
            E->SpawnTime=0;
            E->TakeDungeonDamage(E->Health+1.f);
        }
        CompleteRoom();
    }
    FreedomTime=Remaining;
}

void ADungeonHUD::DrawCombatFX(ADungeonGameMode* G,bool Foreground)
{
    for(const auto& B:G->Blood)
    {
        if(!Foreground)
        {
            const float Alpha=FMath::Min(1.f,B.Age/.3f)*FMath::Clamp((14.f-B.Age)/4.f,0.f,1.f);
            const FString Art=B.bRemains?FString::Printf(TEXT("Remains_%d"),B.Variant):FString::Printf(TEXT("Blood_%d"),4+B.Variant);
            Sprite(Art,B.Position.X-B.Size*.5f,B.Position.Y-B.Size*.5f,B.Size,B.Size,FLinearColor(1,1,1,Alpha));
        }
        else if(!B.bRemains&&B.Age<.4f)
            Sprite(FString::Printf(TEXT("Blood_%d"),FMath::Min(3,int(B.Age*10))),B.Position.X-B.Size*.5f,B.Position.Y-B.Size,B.Size,B.Size,FLinearColor::White);
    }
    if(Foreground&&G->IsFreedomActive())
    {
        const float T=G->FreedomProgress(),X=FMath::Lerp(-220.f,1500.f,T),Y=280.f+FMath::Sin(T*PI*2)*55.f;
        Sprite(FString::Printf(TEXT("Eagle_%d"),int(T*36)%8),X-180,Y-150,360,300);
        for(int I=0;I<3;++I)
        {
            const float Phase=FMath::Fmod(T*2.f+I/3.f,1.f);
            Sprite(TEXT("Ornate_2"),X-120-Phase*180,Y-140-I*42,210,42,FLinearColor(1,1,1,1-Phase));
        }
    }
}
