#include "DungeonActors.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

void ADungeonHero::SpendStamina(float Amount)
{
    Stamina=FMath::Max(0.f,Stamina-Amount); StaminaDelay=.8f;
    if(Stamina<=KINDA_SMALL_NUMBER) { Stamina=0; bExhausted=true; }
}
void ADungeonHero::UpdateStamina(float Dt,bool Sprinting)
{
    if(Sprinting) { SpendStamina(25.f*Dt); return; }
    const float RegenTime=FMath::Max(0.f,Dt-StaminaDelay);
    StaminaDelay=FMath::Max(0.f,StaminaDelay-Dt);
    Stamina=FMath::Min(MaxStamina,Stamina+25.f*RegenTime);
    if(Stamina>=MaxStamina) bExhausted=false;
}
void ADungeonGameMode::UpdatePotions(float Dt)
{
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!H||H->Health<=0||H->IsInventoryOpen()||IsGameplayBlocked()) return;
    for(int32 I=Potions.Num()-1;I>=0;--I)
    {
        auto& P=Potions[I]; P.Age+=Dt;
        // Leave full-health pickups available; ignore fresh drops briefly so they are visible.
        if(P.Age>.4f&&H->Health<H->MaxHealth&&FVector2D::Distance(P.Position,DungeonView::Project(H->GetActorLocation()))<34)
        {
            H->Health=FMath::Min(H->MaxHealth,H->Health+H->MaxHealth*.35f);
            PlaySound(TEXT("Equip"),.65f,1.3f); Potions.RemoveAt(I);
        }
    }
}
void ADungeonHUD::Orb(FVector2D C,float Fraction,FLinearColor Color)
{
    const float Size=96,X=C.X-Size*.5f,Y0=C.Y-Size*.48f,R=Size*.265f;
    Sprite(TEXT("HUD_EmptyOrb"),X,Y0,Size,Size);
    auto* Liquid=Texture(Color.R>Color.B?TEXT("HUD_HealthOrb"):TEXT("HUD_StaminaOrb"));
    if(!Liquid) return;
    const float Level=R-2*R*FMath::Clamp(Fraction,0.f,1.f);
    // Clip the detailed liquid texture, not the metal frame. Values and art share one fill level.
    for(int Y=FMath::CeilToInt(Level);Y<R;++Y)
    {
        const float Half=FMath::Sqrt(FMath::Max(0.f,R*R-Y*Y));
        DrawTexture(Liquid,Offset.X+(C.X-Half)*Scale,Offset.Y+(C.Y+Y)*Scale,Half*2*Scale,Scale,
            .5f-Half/Size,.48f+Y/Size,Half*2/Size,1.f/Size,FLinearColor::White,BLEND_Translucent);
        if(Y<Level+1&&Fraction<.99f) Box(C.X-Half,C.Y+Y,Half*2,1,Color);
    }
}
void ADungeonHUD::DrawVitals(ADungeonHero* H)
{
    const FLinearColor Gold(.94f,.69f,.3f),Pale(.85f,.9f,.86f);
    // Nine-slice the panel so intricate corners remain proportionate on a wide HUD.
    if(auto* Panel=Texture(TEXT("HUD_Panel")))
    {
        const float X[]={458,488,792,822},Y[]={712,735,766,792};
        const float U[]={0,.1f,.9f,1},V[]={0,.3f,.7f,1};
        for(int Row=0;Row<3;++Row) for(int Col=0;Col<3;++Col)
            DrawTexture(Panel,Offset.X+X[Col]*Scale,Offset.Y+Y[Row]*Scale,(X[Col+1]-X[Col])*Scale,(Y[Row+1]-Y[Row])*Scale,
                U[Col],V[Row],U[Col+1]-U[Col],V[Row+1]-V[Row],FLinearColor::White,BLEND_Translucent);
    }
    Orb(FVector2D(480,744),H->Health/H->MaxHealth,FLinearColor(.95f,.045f,.065f));
    Orb(FVector2D(800,744),H->Stamina/H->MaxStamina,FLinearColor(.045f,.38f,1.f));
    Label(TEXT("HEALTH"),457,693,Gold,.65f);
    Label(FString::Printf(TEXT("%.0f / %.0f"),H->Health,H->MaxHealth),457,779,Pale,.65f);
    Label(TEXT("STAMINA"),773,693,Gold,.65f);
    Label(FString::Printf(TEXT("%.0f / %.0f"),H->Stamina,H->MaxStamina),777,779,Pale,.65f);
    Label(H->GetPowerCooldown()>0?FString::Printf(TEXT("RMB TEA  %.1fs"),H->GetPowerCooldown()):TEXT("RMB TEA  READY"),548,728,Gold,.8f);
    Label(H->bExhausted?TEXT("EXHAUSTED - REFILLING"):H->Stamina<30?TEXT("DODGE: NEED STAMINA"):H->GetRollCooldown()>0?TEXT("DODGE RECHARGING"):TEXT("SPACE DODGE / SHIFT SPRINT"),548,748,H->bExhausted?FLinearColor(1,.4f,.2f):Pale,.7f);
    Label(TEXT("E INTERACT    I BAG    P MENU"),548,769,Pale,.65f);
}
void ADungeonHUD::DrawPotions(ADungeonGameMode* G)
{
    for(const auto& P:G->GetPotions())
    {
        const float X=P.Position.X,Y=P.Position.Y+FMath::Sin(P.Age*3)*2;
        Shadow(P.Position,20);
        Ring(P.Position,21+FMath::Sin(P.Age*3)*2,FLinearColor(1,.12f,.18f,.35f),2);
        Sprite(TEXT("HUD_Potion"),X-32,Y-60,64,64);
        Label(TEXT("+35% HEALTH"),X-34,Y+8,FLinearColor(1,.65f,.55f),.65f);
    }
}
