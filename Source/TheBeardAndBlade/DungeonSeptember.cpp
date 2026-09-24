#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "BossHealthWindows.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

bool ADungeonGameMode::CanCollectReward(const ADungeonHero* H) const
{
    if(!H||Reward.Phase!=ERewardPhase::Available||Reward.Choice<0) return false;
    const auto P=DungeonView::Project(H->GetActorLocation());
    // The same chest interaction spot remains valid after the item lands.
    return FVector2D::Distance(P,Reward.Landing)<110||FVector2D::Distance(P,ChestPosition(Reward.Choice))<100;
}
bool ADungeonGameMode::InteractReward(ADungeonHero* H)
{
    if(!H||!bChest||H->Health<=0||!Enemies.IsEmpty()||PendingSpawns>0) return false;
    const auto P=DungeonView::Project(H->GetActorLocation());
    if(Reward.Phase==ERewardPhase::Closed)
    {
        int Choice=INDEX_NONE; float Distance=100;
        for(int I=0;I<3;++I) { const float D=FVector2D::Distance(P,ChestPosition(I)); if(D<Distance) { Distance=D; Choice=I; } }
        if(Choice==INDEX_NONE) return false;
        if(!ChestRolled[Choice]) { ChestLoot[Choice]=RollChestLoot(IsBossRoom(),Room); ChestRolled[Choice]=true; }
        Loot=ChestLoot[Choice]; bLootRolled=true;
        Reward.Choice=Choice; Reward.Phase=ERewardPhase::Opening; Reward.Age=0;
        // The reward area is clear of props and arches. Clamp in logical floor space.
        Reward.Landing=DungeonView::Clamp(ChestPosition(Choice)+FVector2D(0,92));
        PlaySound(TEXT("Chest")); return true;
    }
    if(!CanCollectReward(H)) return false;
    if(!H->AddToInventory(Loot)) { Reward.BagFull=true; LootTimer=8; H->InventoryMessage=TEXT("Bag full. Make space, then collect this item."); return false; }
    // All once-only grants live behind the successful insertion, never animation events.
    Reward.Phase=ERewardPhase::Collected; Reward.CollectedAge=0;Reward.BagFull=false;
    H->Health=FMath::Min(H->MaxHealth,H->Health+35);
    bChest=false; bLootClaimed=true; LootTimer=8; TransitionCooldown=.8f;
    UE_LOG(LogTemp,Display,TEXT("ROOM_FLOW room=%d item collected; doors unlocked"),Room);
    PlaySound(TEXT("UI")); return true;
}
void ADungeonGameMode::UpdateReward(float Dt)
{
    if(Reward.Phase==ERewardPhase::Closed) return;
    if(Reward.Phase==ERewardPhase::Collected) { Reward.CollectedAge+=Dt; return; }
    Reward.Age+=FMath::Max(0.f,Dt);
    // Absolute thresholds are safe when a long frame crosses both transitions.
    if(Reward.Phase==ERewardPhase::Opening&&Reward.Age>=.5f) Reward.Phase=ERewardPhase::Ejecting;
    if(Reward.Phase==ERewardPhase::Ejecting&&Reward.Age>=.95f) { Reward.Phase=ERewardPhase::Available; PlaySound(TEXT("Step"),.35f); }
}
void ADungeonHUD::DrawReward(ADungeonGameMode* G,ADungeonHero* H)
{
    const auto& R=G->GetRewardPresentation();
    if(!G->HasChest()&&R.Phase!=ERewardPhase::Collected) return;
    if(G->HasChest()) for(int I=0;I<3;++I)
    {
        if(R.Choice!=INDEX_NONE&&I!=R.Choice) continue;
        const auto P=G->ChestPosition(I);
        const int Frame=R.Choice==I?FMath::Clamp(int(R.Age*16),0,11):0;
        Sprite(FString::Printf(TEXT("RewardChest_%d"),Frame),P.X-65,P.Y-130*(340.f/352.f),130,130);
    }
    if(R.Phase==ERewardPhase::Closed||R.Phase==ERewardPhase::Opening) return;
    const auto& Item=G->GetLoot();
    if(R.Phase==ERewardPhase::Collected)
    {
        if(R.CollectedAge<.67f) Sprite(FString::Printf(TEXT("LootEffect_%d"),16+FMath::Clamp(int(R.CollectedAge*12),0,7)),R.Landing.X-50,R.Landing.Y-80,100,100);
        return;
    }
    const auto Start=G->ChestPosition(R.Choice);
    const float T=FMath::Clamp((R.Age-.5f)/.45f,0.f,1.f);
    auto P=FMath::Lerp(Start,R.Landing,T);
    if(R.Age<1.15f) Sprite(FString::Printf(TEXT("LootEffect_%d"),FMath::Clamp(int((R.Age-.5f)*12),0,7)),Start.X-55,Start.Y-100,110,110);
    if(R.Phase==ERewardPhase::Available)
    {
        const float LandAge=R.Age-.95f;
        Sprite(FString::Printf(TEXT("RarityGlow_%d"),Item.Rarity*6+int(LandAge*6)%6),R.Landing.X-80,R.Landing.Y-154,160,160);
        if(LandAge<.67f) Sprite(FString::Printf(TEXT("LootEffect_%d"),8+FMath::Clamp(int(LandAge*12),0,7)),P.X-48,P.Y-45,96,96);
        P.Y-=FMath::Abs(FMath::Sin(LandAge*18))*8*FMath::Exp(-LandAge*6);
        if(G->CanCollectReward(H))
        {
            static const TCHAR* Rarity[]={TEXT("Common"),TEXT("Uncommon"),TEXT("Rare"),TEXT("Epic"),TEXT("Legendary")};
            Label(Item.Name+TEXT(" / ")+Rarity[Item.Rarity]+TEXT(" / E"),P.X-65,P.Y+25,FLinearColor(1,.86f,.5f),.72f);
        }
    }
    Shadow(P,15); P.Y-=55*(1-T)+FMath::Sin(T*PI)*65;
    Sprite(Item.Art(),P.X-28,P.Y-53,56,56);
    if(R.Phase==ERewardPhase::Available)
    {
        Box(340,650,600,38,FLinearColor(.015f,.02f,.025f,.94f));
        Label(R.BagFull?TEXT("BAG FULL - press I, discard an item, then press E to collect"):
            TEXT("Reward on the floor - press E beside the chest or item to collect"),355,663,FLinearColor(1,.86f,.5f),.85f);
    }
}

bool ADungeonHero::IsDamageImmune() const
{
    const auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    return Health>0&&(Invulnerable>0||(G&&G->IsFreedomActive()));
}
void ADungeonHero::ApplyBurgerStatus()
{
    if(Health<=0) return;
    StunTime=FMath::Max(StunTime,.75f); SlowTime=FMath::Max(SlowTime,3.f);
    AttackTime=PowerCastTime=RollTime=0; bWalking=false;
}
void ADungeonHUD::DrawStatus(FVector2D P,float Stun,float Slow,float Poison,float Bleed,bool Immune,float Clock)
{
    const bool Active[]={Stun>0,Slow>0,Poison>0,Bleed>0,Immune};
    int Count=0;for(bool B:Active) Count+=B?1:0;
    int Slot=0;
    for(int I=0;I<5;++I) if(Active[I])
    {
        Sprite(FString::Printf(TEXT("Status_%d"),I*8+int(Clock*8)%8),P.X-Count*15+Slot*30,P.Y,30,34);
        ++Slot;
    }
    if(Stun>0) for(int I=0;I<3;++I)
    {
        const float A=Clock*4+I*2*PI/3;
        const float X=P.X+FMath::Cos(A)*25,Y=P.Y-12+FMath::Sin(A)*7;
        // Individual five-point star pixels, mathematically uniform orbit; no ring.
        const FLinearColor Gold(1,.75f,.08f);
        Box(X-2,Y-5,4,10,Gold);Box(X-5,Y-2,10,4,Gold);Box(X-3,Y-3,6,6,FLinearColor(1,.93f,.45f));
    }
}
void ADungeonHUD::DrawBossUI(ADungeonEnemy* E)
{
    const int I=E->Species-24;
    const float Fraction=FMath::Clamp(E->Health/FMath::Max(1.f,E->MaxHealth),0.f,1.f);
    Sprite(FString::Printf(TEXT("BossPortrait_%d"),I),322,20,70,70);
    Sprite(FString::Printf(TEXT("BossName_%d"),I),397,15,166,76);
    if(I>=0&&I<6)
    {
        const auto& Window=BossHealthWindows::Windows[I];
        const float SX=333.f/Window.Width,SY=90.f/Window.Height;
        const float RedEnd=Window.Left+(Window.Right-Window.Left)*Fraction;
        const float LagEnd=Window.Left+(Window.Right-Window.Left)*FMath::Clamp(E->HealthLag/FMath::Max(1.f,E->MaxHealth),0.f,1.f);
        for(int Row=0;Row<Window.Count;++Row)
        {
            const auto& Span=Window.Spans[Row];
            const float X=565+Span.Left*SX,Y=12+Span.Y*SY,H=Span.Height*SY;
            Box(X,Y,(Span.Right-Span.Left)*SX,H,FLinearColor(.1f,.015f,.02f));
            const float LagWidth=FMath::Clamp(LagEnd-Span.Left,0.f,float(Span.Right-Span.Left));
            const float RedWidth=FMath::Clamp(RedEnd-Span.Left,0.f,float(Span.Right-Span.Left));
            if(LagWidth>0)Box(X,Y,LagWidth*SX,H,FLinearColor(.85f,.5f,.1f));
            if(RedWidth>0)Box(X,Y,RedWidth*SX,H,FLinearColor(.8f,.05f,.09f));
        }
        Sprite(FString::Printf(TEXT("BossBorder_%d"),I),565,12,333,90);
        return;
    }
    // Measured per-border openings from the reproducible art manifest.
    static const FVector4 Openings[]={FVector4(.190909f,.501805f,.654545f,.158845f),FVector4(.211688f,.455197f,.611688f,.146953f),FVector4(.207792f,.505576f,.62987f,.156134f),FVector4(.201299f,.420339f,.645455f,.149153f),FVector4(.168508f,.447917f,.644567f,.117188f),FVector4(.161142f,.430412f,.653775f,.110825f),FVector4(.106607f,.355556f,.786787f,.294444f)};
    const int Border=I;
    const auto O=Openings[Border]; const float X=565+O.X*333,Y=12+O.Y*90,W=O.Z*333,H=O.W*90;
    Box(X,Y,W,H,FLinearColor(.1f,.015f,.02f));
    Box(X,Y,W*FMath::Clamp(E->HealthLag/FMath::Max(1.f,E->MaxHealth),0.f,1.f),H,FLinearColor(.85f,.5f,.1f));
    Box(X,Y,W*Fraction,H,FLinearColor(.8f,.05f,.09f));
    Sprite(FString::Printf(TEXT("BossBorder_%d"),Border),565,12,333,90);
}

void ADungeonEnemy::TickNewBoss(float Dt,ADungeonHero* H,ADungeonGameMode* G)
{
    if(SpawnTime>0) { SpawnTime=FMath::Max(0.f,SpawnTime-Dt); return; }
    FlashTime=FMath::Max(0.f,FlashTime-Dt);
    const auto P=DungeonView::Project(GetActorLocation()),Target=DungeonView::Project(H->GetActorLocation());
    if(ActionTime>0)
    {
        const float Before=ActionTime;
        ActionTime=FMath::Max(0.f,ActionTime-Dt);
        if(Species==28&&Action==1)
        {
            const float T=1-ActionTime/ActionDuration;
            SetActorLocation(DungeonView::Unproject(FMath::Lerp(ActionFrom,ActionTo,FMath::SmoothStep(.2f,.8f,T))));
            if(ActionTime<=0) G->AddBossFX(ActionTo,17);
        }
        else if(Species==28)
        {
            if(Before>.22f&&ActionTime<=.22f) G->EmitBossShot(this,true);
        }
        else
        {
            // Timeline crossings, not render frames, emit a bounded number of bullets.
            const float Elapsed=ActionDuration-ActionTime;
            while(ShotsRemaining>0&&ShotTimer<=Elapsed)
            {
                G->EmitBossShot(this,false); --ShotsRemaining; ShotTimer+=Action==3?.12f:10.f;
            }
        }
        if(ActionTime<=0) { Recovery=Species==28?.65f:Action==3?1.6f:.75f; Action=0; }
        return;
    }
    Facing=DungeonView::Direction(Target-P);
    if(Recovery>0)
    {
        const float Step=FMath::Min(Dt,Recovery); Recovery=FMath::Max(0.f,Recovery-Dt);
        if(Species==29)
        {
            const auto Aim=(Target-P).GetSafeNormal();
            const auto Move=FVector2D::Distance(P,Target)<260?FVector2D(-Aim.Y,Aim.X):Aim;
            auto Next=DungeonView::Clamp(P+Move*DungeonRoster::Get(Species).Speed*Step*(SlowTime>0?.65f:1.f));
            Next.Y=FMath::Max(Next.Y,360.); SetActorLocation(DungeonView::Unproject(Next)); bWalking=true;
        }
        return;
    }
    AttackTarget=Target; ++AttackCount;
    if(Species==28)
    {
        Action=AttackCount%2?1:2; ActionDuration=Action==1?.95f:.55f;
        ActionFrom=P; ActionTo=DungeonView::Clamp(P+(Target-P).GetClampedToMaxSize(235)); ActionTo.Y=FMath::Max(330.,ActionTo.Y);
    }
    else
    {
        Action=AttackCount%3?2:3; ActionDuration=Action==3?1.8f:.8f;
        ShotsRemaining=Action==3?10:1; ShotTimer=Action==3?.55f:.35f;
    }
    ActionTime=ActionDuration;
}
void ADungeonGameMode::EmitBossShot(ADungeonEnemy* E,bool Burger)
{
    if(!IsValid(E)||E->Health<=0||IsGameplayBlocked()) return;
    const auto P=DungeonView::Project(E->GetActorLocation());
    auto* Hero=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!Burger&&Hero)
    {
        E->AttackTarget=DungeonView::Project(Hero->GetActorLocation());
        E->Facing=DungeonView::Direction(E->AttackTarget-P);
    }
    const float A=E->Facing*PI/4;
    const FVector2D Aim(FMath::Sin(A),-FMath::Cos(A)),Side(-Aim.Y,Aim.X);
    FDungeonShot S; S.Art=Burger?16:18; S.Style=Burger?10:11;
    // Origins are shared with muzzle flash rendering and direction-specific sprite sockets.
    const FVector2D Muzzles[8][2]={{{-80,-176},{80,-176}},{{105,-168},{85,-142}},{{103,-101},{90,-70}},{{62,-22},{90,-63}},{{-66,-20},{55,-4}},{{-102,-56},{-68,-18}},{{-106,-103},{-93,-73}},{{-106,-172},{-105,-139}}};
    const int Rifle=E->ShotSerial++%2;
    S.Position=P+(Burger?Aim*75+FVector2D(0,-85):Muzzles[E->Facing][Rifle]);
    S.Origin=S.Position; S.Target=E->AttackTarget;
    // Re-aim every round, with bounded movement lead. Bullets never home after firing.
    if(!Burger&&Hero) S.Target=DungeonView::Clamp(S.Target+Hero->ScreenVelocity*FMath::Min(.22f,float(FVector2D::Distance(S.Target,S.Position)/2200.f)));
    S.Velocity=(S.Target-S.Position).GetSafeNormal()*(Burger?300.f:2200.f);
    S.Life=Burger?3.f:2.5f; S.Radius=Burger?18.f:6.f;
    S.Damage=DungeonRoster::Get(E->Species).Damage; S.BlastRadius=0;
    Shots.Add(S); E->FlashTime=.085f;
    AddBossFX(S.Position,Burger?19:18);
    PlaySound(Burger?TEXT("Throw"):FString::Printf(TEXT("Rifle%d"),E->ShotSerial%3),Burger?.4f:.85f,Burger?1.f:FMath::FRandRange(.97f,1.03f));
}
void ADungeonGameMode::AddBossFX(FVector2D P,int32 Art)
{
    FDungeonSplash FX;FX.Position=P;FX.Art=Art;FX.Radius=Art==17?85:Art==18?14:65;FX.Life=.65f;
    if(Splashes.Num()>100) Splashes.RemoveAt(0);
    Splashes.Add(FX);
}

void ADungeonGameMode::VerifySeptember()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;
    auto Check=[&](bool OK,const TCHAR* What){if(!OK){++Errors;UE_LOG(LogTemp,Error,TEXT("SEPTEMBER_VERIFY: %s"),What);}};
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!H) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    StartGame(); PendingSpawns=0;
    for(int Rarity=0;Rarity<5;++Rarity) for(int Slot=0;Slot<3;++Slot)
    {
        RestartRun();PendingSpawns=0; bChest=true;H->Health=40;
        ChestRolled[1]=true;ChestLoot[1]=RollItem(Slot==0?0:Slot==1?24:36,Rarity,3);
        H->Inventory.Empty();for(int I=0;I<36;++I) H->AddToInventory(RollItem(36,0,1));
        H->SetActorLocation(DungeonView::Unproject(ChestPosition(1)));
        PlayerInteract(H); const auto Saved=Loot;
        Check(Reward.Choice==1&&Reward.Phase==ERewardPhase::Opening&&H->Inventory.Num()==36,TEXT("Opening locks choice without granting"));
        PlayerInteract(H);Check(Reward.Age==0&&Loot.Name==Saved.Name,TEXT("Repeated opening does not reroll"));
        ToggleMenu();Tick(10);ToggleMenu();Check(Reward.Age==0,TEXT("Menu pauses chest"));
        UpdateReward(10);Check(Reward.Phase==ERewardPhase::Available,TEXT("Long frame completes ejection"));
        H->SetActorLocation(DungeonView::Unproject(Reward.Landing));
        for(int I=0;I<3;++I) PlayerInteract(H);
        Check(!bLootClaimed&&H->Health==40&&Loot.Attack==Saved.Attack&&Loot.Effect==Saved.Effect,TEXT("Full bag preserves exact reward and no heal"));
        H->Inventory.Empty();PlayerInteract(H);
        Check(H->Inventory.Num()==1&&H->Inventory[0].Item.Stats()==Saved.Stats()&&H->Health==75&&bLootClaimed,TEXT("Pickup grants exact item and heal once"));
        PlayerInteract(H);UpdateReward(10);PlayerInteract(H);
        Check(H->Inventory.Num()==1&&H->Health==75,TEXT("No duplicate pickup"));
    }
    RestartRun();PendingSpawns=0;H->ApplyBurgerStatus();
    H->MoveRight(1);const auto Start=H->GetActorLocation();H->Tick(.5f);
    Check(H->GetActorLocation()==Start&&H->StunTime>0,TEXT("Stun blocks movement"));
    H->Attack();H->Dodge();H->PowerMove();Check(!H->IsAttacking()&&!H->IsRolling()&&!H->IsCasting(),TEXT("Stun blocks combat"));
    H->Tick(.3f);Check(H->StunTime==0&&H->SlowTime>0,TEXT("Stun expires before slow"));
    const auto P=DungeonView::Project(H->GetActorLocation());H->Tick(.1f);
    Check(FMath::IsNearlyEqual(float(DungeonView::Project(H->GetActorLocation()).X-P.X),11.4f,.02f),TEXT("Slow movement magnitude"));
    H->Tick(4);Check(H->SlowTime==0,TEXT("Slow expires"));H->Restart();
    for(int Species:{28,29})
    {
        FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* E=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),DungeonView::Unproject(FVector2D(600,420)),FRotator::ZeroRotator,Params);
        E->Species=Species;E->bBoss=true;E->SpawnTime=0;E->Health=E->MaxHealth=1000;
        for(int D=0;D<8;++D)
        {
            E->Facing=D;E->AttackTarget=FVector2D(640,520);Shots.Empty();EmitBossShot(E,Species==28);
            Check(Shots.Num()==1&&Shots[0].Origin!=DungeonView::Project(E->GetActorLocation()),TEXT("Directional projectile uses muzzle, not center"));
            if(Species==29&&Shots.Num()==1) Check(FMath::IsNearlyEqual(Shots[0].Damage,16.8f),TEXT("Twister bullet damage reduced thirty percent"));
            for(int F=0;F<(Species==28?16:24);++F)
            {
                const FString N=FString::Printf(TEXT("%s_%d_%d"),Species==28?TEXT("Mack"):TEXT("Twister"),D,F);
                Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/September/%s.%s"),*N,*N))!=nullptr,TEXT("All directional frames imported"));
            }
        }
        if(Species==29) for(float Dt:{.016f,.2f,2.f})
        {
            Shots.Empty();E->Action=3;E->ActionTime=E->ActionDuration=1.8f;E->ShotsRemaining=10;E->ShotTimer=.55f;E->Recovery=0;
            while(E->ActionTime>0) E->TickNewBoss(Dt,H,this);
            Check(Shots.Num()==10,TEXT("Burst is ten bullets independent of frame rate"));
            Shots.Empty();E->Action=2;E->ActionTime=E->ActionDuration=.8f;E->ShotsRemaining=1;E->ShotTimer=.35f;
            while(E->ActionTime>0) E->TickNewBoss(Dt,H,this);
            Check(Shots.Num()==1,TEXT("Single shot occurs once"));
        }
        E->Destroy();
    }
    Shots.Empty();H->Restart();
    // Every bullet counts, including within the ordinary melee hurt grace period.
    H->ReceiveHit(10); const float AfterMelee=H->Health;
    H->ReceiveHit(10,true); H->ReceiveHit(10,true);
    Check(H->Health<AfterMelee-14,TEXT("Consecutive bullets bypass hurt grace"));
    H->Restart();H->Dodge();const float BeforeDodge=H->Health;H->ReceiveHit(50,true);
    Check(H->Health==BeforeDodge,TEXT("Dodge immunity still blocks bullets"));H->Restart();
    FDungeonShot Fast;Fast.Style=11;Fast.Radius=6;Fast.Damage=20;Fast.Life=1;
    const auto HP=DungeonView::Project(H->GetActorLocation());
    Fast.Position=Fast.Origin=HP-FVector2D(220,0);Fast.Velocity=FVector2D(2200,0);
    Shots.Add(Fast);Shots.Add(Fast);UpdateProjectiles(.2f);
    Check(Shots.IsEmpty()&&H->Health<116,TEXT("Swept fast projectiles each hit once at low frame rate"));
    const float AfterBullets=H->Health;UpdateProjectiles(.2f);
    Check(H->Health==AfterBullets,TEXT("Resolved bullets cannot damage twice"));H->Restart();
    FDungeonShot Burger;Burger.Style=10;Burger.Art=16;Burger.Damage=30;Burger.Radius=18;
    const auto BurgerPoint=DungeonView::Project(H->GetActorLocation());ResolveProjectile(Burger,BurgerPoint);
    Check(H->StunTime>0&&H->SlowTime>0&&H->Health<150,TEXT("Valid burger hit applies damage and statuses"));
    H->Restart();H->Dodge();ResolveProjectile(Burger,BurgerPoint);Check(H->Health==150&&H->StunTime==0,TEXT("Immune burger hit applies no status"));
    RestartRun();Check(H->StunTime==0&&Reward.Phase==ERewardPhase::Closed,TEXT("Restart clears statuses and reward events"));
    const int Order[]={24,28,30,25,26,27,29};
    for(int I=0;I<7;++I) { Room=(I+1)*3;Check(GetBossSpecies()==Order[I],TEXT("Approved seven-boss campaign order")); }
    for(int I=0;I<7;++I) for(const TCHAR* Part:{TEXT("Portrait"),TEXT("Name"),TEXT("Border")})
    {
        const FString N=FString::Printf(TEXT("Boss%s_%d"),Part,I);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/September/%s.%s"),*N,*N))!=nullptr,TEXT("Boss UI imported"));
    }
    // Exercise real timed spawning, chest-side pickup and delayed transitions.
    auto ClearTimedRoom=[&]() {
        int Guard=0;
        while(!bChest&&++Guard<80) {
            Tick(2.1f);
            if(IsBossDialogueActive()) {DialogueWait=0;AdvanceBossDialogue(true);BossGrace=0;}
            const auto Batch=Enemies;
            for(auto& Enemy:Batch) if(IsValid(Enemy)){Enemy->SpawnTime=0;Enemy->TakeDungeonDamage(100000);}
        }
        Check(bChest&&PendingSpawns==0&&Enemies.IsEmpty(),TEXT("Timed waves end before rewards"));
    };
    StartPlaytestRoom(2);ClearTimedRoom();
    H->SetActorLocation(DungeonView::Unproject(ChestPosition(1)));PlayerInteract(H);Tick(2);
    Check(Enemies.IsEmpty()&&PendingSpawns==0,TEXT("Opening chest never spawns Finance Guy"));
    PlayerInteract(H);Check(bLootClaimed,TEXT("Collect reward without leaving original chest position"));
    Tick(1);H->SetActorLocation(DungeonView::Unproject(DoorPosition(1)));PlayerInteract(H);Tick(2.1f);
    Check(Room==3&&!bChest&&Reward.Phase==ERewardPhase::Closed&&Enemies.IsEmpty(),TEXT("Boss room clears all previous reward visuals"));
    Tick(2.1f);Check(Enemies.Num()==1&&Enemies[0]->Species==24&&!bChest,TEXT("Finance spawns once only after gate transition"));
    ClearTimedRoom();Check(bChest,TEXT("Finance reward follows boss death"));
    StartPlaytestRoom(13);ClearTimedRoom();
    H->Inventory.Empty();for(int I=0;I<36;++I)H->AddToInventory(RollItem(36,0,1));
    ChestRolled[0]=true;ChestLoot[0]=RollItem(36,2,13);
    H->SetActorLocation(DungeonView::Unproject(ChestPosition(0)));PlayerInteract(H);Tick(2);PlayerInteract(H);
    Check(Reward.BagFull&&!AreDoorsOpen(),TEXT("Ice full bag visibly explains locked portals"));
    H->Inventory.RemoveAt(0);PlayerInteract(H);
    Check(AreDoorsOpen()&&H->Inventory.Num()==36,TEXT("Ice pickup unlocks after making one valid slot"));
    Tick(1);H->SetActorLocation(DungeonView::Unproject(DoorPosition(0)));PlayerInteract(H);Tick(2.1f);
    Check(Room==14&&!IsTransitioning(),TEXT("First ice dungeon gate advances normally"));
    for(int I=0;I<7;++I)
    {
        StartPlaytestRoom((I+1)*3);Tick(2.1f);
        Check(Room==(I+1)*3&&Enemies.Num()==1&&Enemies[0]->Species==Order[I]&&!bChest,TEXT("Each direct boss playtest starts clean"));
    }
    UE_LOG(LogTemp,Display,TEXT("SEPTEMBER_VERIFY_COMPLETE errors=%d"),Errors);
    FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}

void ADungeonGameMode::RunSeptemberSmoke()
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("DungeonSeptemberSmoke"))) return;
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));if(!H)return;
    static int Stage=0,Errors=0; static float Deadline=2;
    const float Time=GetWorld()->GetTimeSeconds();if(Time<Deadline)return;
    auto Capture=[&](const FString& Name){FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/")+Name+TEXT(".png"),false,false);};
    if(Stage<28)
    {
        const int Boss=Stage/4,Part=Stage%4;
        if(Part==0)
        {
            StartGame();Room=(Boss+1)*3;PendingSpawns=0;SpawnOneEnemy();
            H->Health=H->MaxHealth=10000;
            if(Enemies.Num()!=1||Enemies[0]->Species!=GetBossSpecies()) ++Errors;
            DialogueIndex=1;DialogueWait=0;
        }
        if(Part==1) Capture(FString::Printf(TEXT("SeptemberBoss%d"),Boss));
        if(Part==2) {AdvanceBossDialogue(true);BossGrace=0;}
        if(Part==3) Capture(FString::Printf(TEXT("SeptemberCombat%d"),Boss));
        ++Stage;Deadline=Time+(Part==2?3.f:1.f);return;
    }
    if(Stage==28)
    {
        StartGame();PendingSpawns=0;bChest=true;
        H->SetActorLocation(DungeonView::Unproject(ChestPosition(1)+FVector2D(130,50)));
        ChestLoot[1]=RollItem(8,4,6);ChestRolled[1]=true;
        Capture(TEXT("SeptemberChoices"));++Stage;Deadline=Time+1;return;
    }
    if(Stage==29)
    {
        H->SetActorLocation(DungeonView::Unproject(ChestPosition(1)));PlayerInteract(H);
        H->SetActorLocation(DungeonView::Unproject(ChestPosition(1)+FVector2D(130,50)));
        ++Stage;Deadline=Time+.32f;return;
    }
    if(Stage==30) {Capture(TEXT("SeptemberOpening"));++Stage;Deadline=Time+.3f;return;}
    if(Stage==31) {Capture(TEXT("SeptemberEjection"));++Stage;Deadline=Time+1;return;}
    if(Stage==32)
    {
        if(Reward.Phase!=ERewardPhase::Available||H->Inventory.Num()!=0||bLootClaimed)++Errors;
        Capture(TEXT("SeptemberFloorLoot"));H->Inventory.Empty();for(int I=0;I<36;++I)H->AddToInventory(RollItem(36,0,1));
        H->SetActorLocation(DungeonView::Unproject(Reward.Landing));PlayerInteract(H);
        if(bLootClaimed||Reward.Phase!=ERewardPhase::Available)++Errors;
        ++Stage;Deadline=Time+1;return;
    }
    if(Stage==33)
    {
        H->Inventory.Empty();PlayerInteract(H);PlayerInteract(H);
        if(H->Inventory.Num()!=1||!bLootClaimed||H->Inventory[0].Item.CatalogId!=8)++Errors;
        Capture(TEXT("SeptemberCollected"));++Stage;Deadline=Time+1;return;
    }
    if(Stage==34)
    {
        FFileHelper::SaveStringToFile(FString::Printf(TEXT("SEPTEMBER_SMOKE errors=%d; seven boss introductions/combat; chest choice/open/eject/land/full bag/pickup\n"),Errors),*(FPaths::ProjectSavedDir()/TEXT("SeptemberSmokeTest.txt")));
        ++Stage;FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
    }
}
