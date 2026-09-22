#include "DungeonActors.h"
#include "HeroSockets.h"
#include "DungeonRoster.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#if WITH_EDITOR
#include "TextureCompiler.h"
#endif

namespace
{
    const FLinearColor Gold(.94f,.69f,.30f), Pale(.85f,.9f,.86f), Ink(.012f,.021f,.025f,.94f);
    const TCHAR* RarityNames[]={TEXT("COMMON"),TEXT("UNCOMMON"),TEXT("RARE"),TEXT("EPIC"),TEXT("LEGENDARY")};
    FLinearColor RarityColor(int32 R)
    {
        const FLinearColor Colors[]={FLinearColor(.65f,.69f,.72f),FLinearColor(.3f,.88f,.53f),FLinearColor(.3f,.62f,1.f),FLinearColor(.8f,.38f,1.f),FLinearColor(1.f,.6f,.16f)};
        return Colors[FMath::Clamp(R,0,4)];
    }
    ADungeonHero* Player(UObject* Context) { return Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(Context,0)); }
    ADungeonGameMode* Mode(UObject* Context) { return Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(Context)); }
    FString ItemArt(int32 Icon) { return FString::Printf(TEXT("Item_%d_%d"),Icon/3,Icon%3); }
}
FString FDungeonItem::Stats() const
{
    FString S;
    if(Attack>0) S+=FString::Printf(TEXT("+%d ATTACK  "),(int32)Attack);
    if(Defense>0) S+=FString::Printf(TEXT("+%d ARMOR  "),(int32)Defense);
    if(Vitality>0) S+=FString::Printf(TEXT("+%d HEALTH"),(int32)Vitality);
    return S;
}
ADungeonHero::ADungeonHero()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("HeroRoot"));
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("FixedCamera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetAbsolute(true,true,true);
    Camera->SetRelativeLocation(FVector(0,0,1200));
    Camera->SetRelativeRotation(FRotator(-90,0,0));
    AutoPossessPlayer=EAutoReceiveInput::Player0;
}
void ADungeonHero::BeginPlay()
{
    Super::BeginPlay();
    Restart();
    if(auto* PC=Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor=true;
        FInputModeGameAndUI Input;
        Input.SetHideCursorDuringCapture(false);
        Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Input);
    }
}
void ADungeonHero::Restart()
{
    Equipment.SetNum(3);
    Equipment[0]=ADungeonGameMode::MakeItem(0,0); Equipment[0].Attack=0; Equipment[0].Name=TEXT("Old Iron Sword");
    Equipment[1]=ADungeonGameMode::MakeItem(3,0); Equipment[1].Defense=0; Equipment[1].Vitality=0; Equipment[1].Name=TEXT("Traveler's Leathers");
    Equipment[2]=FDungeonItem(); Equipment[2].Slot=2; Equipment[2].Icon=6;
    Inventory.Empty(); SelectedItem=INDEX_NONE; bInventoryOpen=false; InventoryMessage.Empty();
    Health=MaxHealth=150; AttackPower=24; Armor=8; HurtTime=Invulnerable=AttackTime=0;
    InputX=InputY=WalkDistance=0; bSprinting=bWalking=false; Aim=FVector2D(0,1); Facing=4;
    RollTime=RollCooldown=PowerCooldown=PowerCastTime=0; bTeaReleased=false;
    SetActorLocation(DungeonView::Unproject(FVector2D(640,520)));
}
void ADungeonHero::Tick(float Dt)
{
    Super::Tick(Dt);
    if(auto* G=Mode(this)) if(G->IsGameplayBlocked()) return;
    if(bInventoryOpen) { bWalking=false; return; }
    RollCooldown=FMath::Max(0.f,RollCooldown-Dt);
    PowerCooldown=FMath::Max(0.f,PowerCooldown-Dt);
    if(PowerCastTime>0)
    {
        PowerCastTime=FMath::Max(0.f,PowerCastTime-Dt);
        if(!bTeaReleased&&PowerCastTime<=.26f)
        {
            bTeaReleased=true; if(auto* G=Mode(this)) G->LaunchTea(this,PowerTarget);
        }
    }
    HurtTime=FMath::Max(0.f,HurtTime-Dt); Invulnerable=FMath::Max(0.f,Invulnerable-Dt);
    if(Health<=0) { bWalking=false; AttackTime=PowerCastTime=0; return; }
    if(RollTime>0)
    {
        const float Step=FMath::Min(Dt,RollTime); RollTime=FMath::Max(0.f,RollTime-Dt);
        SetActorLocation(DungeonView::Unproject(DungeonView::Clamp(DungeonView::Project(GetActorLocation())+RollAim*510.f*Step)));
        bWalking=false; return;
    }
    FVector2D P=DungeonView::Project(GetActorLocation());
    FVector2D Move(InputX,InputY);
    Move=Move.GetClampedToMaxSize(1);
    const FVector2D Next=DungeonView::Clamp(P+Move*(bSprinting?380.f:190.f)*Dt*(IsAttacking()?.5f:1.f));
    bWalking=FVector2D::Distance(Next,P)>.01f;
    WalkDistance+=FVector2D::Distance(Next,P);
    FootstepDistance+=FVector2D::Distance(Next,P);
    if(bWalking&&FootstepDistance>58) { FootstepDistance=0; if(auto* G=Mode(this)) G->PlaySound(TEXT("Step"),.35f,FMath::FRandRange(.9f,1.1f)); }
    SetActorLocation(DungeonView::Unproject(Next));
    if(auto* PC=Cast<APlayerController>(GetController()))
    {
        int32 W=0,H=0; float MX=0,MY=0; PC->GetViewportSize(W,H);
        const float S=FMath::Min(W/1280.f,H/800.f);
        if(S>0&&PC->GetMousePosition(MX,MY))
        {
            FVector2D Mouse=(FVector2D(MX,MY)-FVector2D((W-1280*S)/2,(H-800*S)/2))/S;
            if(Mouse.X>=0&&Mouse.X<=1280&&Mouse.Y>=0&&Mouse.Y<=800)
            {
                const FVector2D Delta=Mouse-Next;
                if(Delta.SizeSquared()>144)
                {
                    Aim=Delta.GetSafeNormal();
                    Facing=DungeonView::Direction(Aim);
                }
            }
        }
    }
    if(AttackTime>0)
    {
        AttackTime=FMath::Max(0.f,AttackTime-Dt);
        // Strike once on the impact pose, not during the wind-up.
        if(!bAttackHit&&AttackTime<=.28f)
        {
            bAttackHit=true;
            if(auto* G=Mode(this)) G->PlayerAttack(this);
        }
    }
}
int32 ADungeonHero::GetAnimationFrame() const
{
    if(IsCasting()) return FMath::Clamp((int32)(GetCastProgress()*6),0,5);
    if(IsAttacking()) return FMath::Clamp((int32)(GetAttackProgress()*6),0,5);
    return bWalking?((int32)(WalkDistance/11.f)%6):0;
}
void ADungeonHero::SetupPlayerInputComponent(UInputComponent* I)
{
    I->BindAxis("MoveForward",this,&ADungeonHero::MoveForward);
    I->BindAxis("MoveRight",this,&ADungeonHero::MoveRight);
    I->BindAction("Attack",IE_Pressed,this,&ADungeonHero::Attack);
    I->BindAction("PowerMove",IE_Pressed,this,&ADungeonHero::PowerMove);
    I->BindAction("ToggleMusic",IE_Pressed,this,&ADungeonHero::ToggleMusic);
    I->BindAction("ToggleEffects",IE_Pressed,this,&ADungeonHero::ToggleEffects);
    I->BindAction("Interact",IE_Pressed,this,&ADungeonHero::Interact);
    I->BindAction("Inventory",IE_Pressed,this,&ADungeonHero::ToggleInventory);
    I->BindAction("Dodge",IE_Pressed,this,&ADungeonHero::Dodge);
    I->BindAction("Menu",IE_Pressed,this,&ADungeonHero::Menu);
    I->BindAction("Confirm",IE_Pressed,this,&ADungeonHero::Confirm);
    I->BindAction("Sprint",IE_Pressed,this,&ADungeonHero::SprintPressed);
    I->BindAction("Sprint",IE_Released,this,&ADungeonHero::SprintReleased);
}
void ADungeonHero::Attack()
{
    if(auto* G=Mode(this))
    {
        if(G->IsMenu()) { if(auto* PC=Cast<APlayerController>(GetController())) if(auto* HUD=Cast<ADungeonHUD>(PC->GetHUD())) HUD->InventoryClick(); return; }
        if(G->IsTransitioning()) return;
    }
    if(bInventoryOpen)
    {
        if(auto* PC=Cast<APlayerController>(GetController()))
            if(auto* HUD=Cast<ADungeonHUD>(PC->GetHUD())) HUD->InventoryClick();
        return;
    }
    if(Health<=0||IsAttacking()||IsRolling()||IsCasting()) return;
    AttackTime=.48f; bAttackHit=false; AttackAim=Aim; AttackDirection=Facing;
    if(auto* G=Mode(this)) G->PlaySound(TEXT("Sword"),.8f,FMath::FRandRange(.92f,1.08f));
}
void ADungeonHero::Interact() { if(!bInventoryOpen) if(auto* G=Mode(this)) G->PlayerInteract(this); }
void ADungeonHero::ToggleInventory()
{
    if(auto* G=Mode(this)) if(G->IsGameplayBlocked()) return;
    if(IsRolling()) return;
    if(Health<=0) return;
    bInventoryOpen=!bInventoryOpen; InventoryMessage.Empty();
    if(auto* G=Mode(this)) G->PlaySound(TEXT("UI"),.4f);
}
bool ADungeonHero::CanPlace(FIntPoint Cell,FIntPoint Size,int32 Ignore) const
{
    if(Cell.X<0||Cell.Y<0||Cell.X+Size.X>6||Cell.Y+Size.Y>6) return false;
    for(int32 I=0;I<Inventory.Num();++I) if(I!=Ignore)
    {
        const auto& E=Inventory[I]; const FIntPoint S=E.Item.Size();
        if(Cell.X<E.Cell.X+S.X&&Cell.X+Size.X>E.Cell.X&&Cell.Y<E.Cell.Y+S.Y&&Cell.Y+Size.Y>E.Cell.Y) return false;
    }
    return true;
}
bool ADungeonHero::AddToInventory(const FDungeonItem& Item)
{
    if(Item.IsEmpty()) return false;
    for(int32 Y=0;Y<6;++Y) for(int32 X=0;X<6;++X) if(CanPlace({X,Y},Item.Size()))
    {
        Inventory.Add({Item,{X,Y}}); InventoryMessage=TEXT("Item added to your bag."); return true;
    }
    InventoryMessage=TEXT("Bag full. Make space, then press E at the chest again."); return false;
}
bool ADungeonHero::EquipFromInventory(int32 Index)
{
    if(!Inventory.IsValidIndex(Index)) return false;
    const FDungeonItem Item=Inventory[Index].Item;
    if(!Equipment.IsValidIndex(Item.Slot)) return false;
    const FDungeonItem Old=Equipment[Item.Slot];
    // Same-slot items have identical footprints: even a full bag can swap safely.
    if(!Old.IsEmpty()) Inventory[Index].Item=Old;
    else { Inventory.RemoveAt(Index); SelectedItem=INDEX_NONE; }
    Equip(Item); InventoryMessage=TEXT("Equipped. Previous gear returned to the bag."); return true;
}
bool ADungeonHero::Unequip(int32 Slot)
{
    if(!Equipment.IsValidIndex(Slot)||Equipment[Slot].IsEmpty()) return false;
    if(!AddToInventory(Equipment[Slot])) return false;
    FDungeonItem Empty; Empty.Slot=Slot; Equip(Empty);
    InventoryMessage=TEXT("Equipment returned to your bag."); return true;
}
void ADungeonHero::ReceiveHit(float Damage)
{
    if(Invulnerable>0||Health<=0) return;
    const float Actual=FMath::Max(2.f,Damage-Armor*.3f);
    Health=FMath::Max(0.f,Health-Actual);
    HurtTime=.18f; Invulnerable=.65f;
    if(auto* G=Mode(this)) G->PlaySound(Health<=0?TEXT("Death"):TEXT("Hurt"));
    if(auto* G=Mode(this)) G->AddImpact(DungeonView::Project(GetActorLocation()),Actual,true);
}
void ADungeonHero::Dodge()
{
    auto* G=Mode(this);
    if(!G||G->IsGameplayBlocked()||bInventoryOpen||Health<=0||RollCooldown>0||IsCasting()) return;
    RollAim=FVector2D(InputX,InputY).GetSafeNormal(); if(RollAim.IsNearlyZero()) RollAim=Aim;
    RollDirection=(FMath::RoundToInt(FMath::Atan2(RollAim.Y,RollAim.X)/(PI/2))+4)%4;
    RollTime=.48f; RollCooldown=1.15f; Invulnerable=.34f; AttackTime=0; bAttackHit=true;
    G->PlaySound(TEXT("Roll"));
}
void ADungeonHero::Menu() { if(auto* G=Mode(this)) G->ToggleMenu(); }
void ADungeonHero::Confirm() { if(auto* G=Mode(this)) if(G->IsMenu()) { if(G->HasRun()) G->ToggleMenu(); else G->StartGame(); } }
void ADungeonHero::TransitionWalk(FVector2D From,FVector2D To,float Progress)
{
    SetActorLocation(DungeonView::Unproject(FMath::Lerp(From,To,Progress)));
    Facing=DungeonView::Direction(To-From); Aim=(To-From).GetSafeNormal(); bWalking=true;
    WalkDistance=Progress*FVector2D::Distance(From,To); AttackTime=RollTime=0;
}
void ADungeonHero::Equip(const FDungeonItem& Item)
{
    if(!Equipment.IsValidIndex(Item.Slot)) return;
    Equipment[Item.Slot]=Item;
    if(auto* G=Mode(this)) G->PlaySound(TEXT("Equip"),.65f);
    AttackPower=24; Armor=8; MaxHealth=150;
    for(const auto& E:Equipment) { AttackPower+=E.Attack; Armor+=E.Defense; MaxHealth+=E.Vitality; }
    Health=FMath::Clamp(Health,0.f,MaxHealth); // Swapping vitality gear must not repeatedly heal.
}
ADungeonEnemy::ADungeonEnemy()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("EnemyRoot"));
}
void ADungeonEnemy::TakeDungeonDamage(float Damage)
{
    if(Health<=0||SpawnTime>0) return;
    Health=FMath::Max(0.f,Health-Damage); HurtTime=.2f;
    auto* G=Mode(this);
    const FVector2D P=DungeonView::Project(GetActorLocation());
    if(G) G->AddImpact(P,Damage);
    if(G) G->PlaySound(Health<=0?TEXT("Death"):TEXT("Hit"),.75f,FMath::FRandRange(.9f,1.1f));
    if(auto* H=Player(this))
    {
        auto Push=(P-DungeonView::Project(H->GetActorLocation())).GetSafeNormal();
        SetActorLocation(DungeonView::Unproject(DungeonView::Clamp(P+Push*(bBoss?5:18))));
    }
    if(Health<=0&&G) G->EnemyDefeated(this);
}
ADungeonGameMode::ADungeonGameMode()
{
    DefaultPawnClass=ADungeonHero::StaticClass(); HUDClass=ADungeonHUD::StaticClass();
    PrimaryActorTick.bCanEverTick=true;
}
void ADungeonGameMode::BeginPlay()
{
    Super::BeginPlay();
    GConfig->GetBool(TEXT("DungeonAudio"),TEXT("MuteMusic"),bMusicMuted,GGameUserSettingsIni);
    GConfig->GetBool(TEXT("DungeonAudio"),TEXT("MuteEffects"),bEffectsMuted,GGameUserSettingsIni);
#if !UE_BUILD_SHIPPING
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonVerify"))) VerifyCampaign();
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonCapture"))&&!FParse::Param(FCommandLine::Get(),TEXT("DungeonMenuPreview"))) { bMenu=false; bHasRun=true; SpawnWave(); }
    int32 PreviewBiome=0;
    FParse::Value(FCommandLine::Get(),TEXT("DungeonBiome="),PreviewBiome);
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonCapture"))&&!bMenu) { Room=1+4*FMath::Clamp(PreviewBiome,0,3); SpawnWave(); }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonBossPreview"))) { Room=4+4*FMath::Clamp(PreviewBiome,0,3); SpawnWave(); }
#endif
}
void ADungeonGameMode::Tick(float Dt)
{
    Super::Tick(Dt);
    UpdateAudio();
    RunPackagedSmokeTest();
#if !UE_BUILD_SHIPPING
    // Explicit review-only launch flags. Never run during normal editor play.
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonCapture")))
    {
        static bool Prepared=false, Captured=false;
        const float Time=GetWorld()->GetTimeSeconds();
        if(!Prepared&&Time>2.f)
        {
            Prepared=true;
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonRewardPreview"))||FParse::Param(FCommandLine::Get(),TEXT("DungeonInventoryPreview"))||FParse::Param(FCommandLine::Get(),TEXT("DungeonChestPreview")))
            {
                PendingSpawns=0;
                for(auto& E:Enemies) if(IsValid(E)) E->Destroy();
                Enemies.Empty(); bChest=true;
                if(auto* H=Player(this))
                {
                    H->SetActorLocation(DungeonView::Unproject(FVector2D(640,424)));
                    if(!FParse::Param(FCommandLine::Get(),TEXT("DungeonChestPreview"))) PlayerInteract(H);
                    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonInventoryPreview")))
                    {
                        H->Inventory.Empty();
                        for(int I=0;I<9;++I) H->AddToInventory(MakeItem(I,I%5));
                        H->SelectedItem=2; H->ToggleInventory();
                    }
                }
            }
        }
        static bool TeaPrepared=false;
        if(!TeaPrepared&&Time>3.85f&&FParse::Param(FCommandLine::Get(),TEXT("DungeonTeaPreview")))
        {
            TeaPrepared=true; PendingSpawns=0;
            for(auto& E:Enemies) if(IsValid(E)) E->Destroy();
            Enemies.Empty(); Shots.Empty();
            FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            for(int I=0;I<2;++I)
            {
                auto* E=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),DungeonView::Unproject(FVector2D(600+I*80,425)),FRotator::ZeroRotator,Params);
                E->Health=E->MaxHealth=200; E->SpawnTime=0; E->SetActorTickEnabled(false); Enemies.Add(E);
            }
            if(auto* H=Player(this)) { H->SetActorLocation(DungeonView::Unproject(FVector2D(640,610))); LaunchTea(H,FVector2D(640,425)); }
        }
        if(!Captured&&Time>4.5f)
        {
            Captured=true;
            FString Name=FParse::Param(FCommandLine::Get(),TEXT("DungeonRewardPreview"))?TEXT("RewardReview"):Room==2?TEXT("BossReview"):TEXT("ArenaReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonInventoryPreview"))) Name=TEXT("InventoryReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonMenuPreview"))) Name=TEXT("MenuReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonRosterPreview"))) Name=TEXT("RosterReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonRollPreview"))) Name=TEXT("RollReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonFXPreview"))) Name=TEXT("FXReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonTeaPreview"))) Name=TEXT("TeaReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonChestPreview"))) Name=TEXT("ChestReview");
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonBossPreview"))) Name=FString::Printf(TEXT("BossReview%d"),GetBiome());
            if(FParse::Param(FCommandLine::Get(),TEXT("DungeonEquipmentReview"))) Name=FParse::Param(FCommandLine::Get(),TEXT("ReviewLeft"))?TEXT("EquipmentLeftReview"):TEXT("EquipmentReview");
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("ArtSource")/(Name+TEXT(".png")),true,false);
        }
        if(Time>6.f) FPlatformMisc::RequestExit(false);
    }
#endif
    if(bMenu) return;
    if(TransitionTime>0)
    {
        TransitionTime=FMath::Max(0.f,TransitionTime-Dt);
        if(auto* H=Player(this)) H->TransitionWalk(TransitionFrom,DoorPosition(TransitionDoor)-FVector2D(0,95),FMath::Min(1.f,TransitionProgress()*1.45f));
        if(TransitionTime<=0) NextRoom();
        return;
    }
    if(auto* H=Player(this)) if(H->IsInventoryOpen()) return;
    LootTimer=FMath::Max(0.f,LootTimer-Dt); TransitionCooldown=FMath::Max(0.f,TransitionCooldown-Dt);
    for(auto& Impact:Impacts) Impact.Life-=Dt;
    Impacts.RemoveAll([](const FDungeonImpact& I){return I.Life<=0;});
    if(IsDead()) return;
    UpdateProjectiles(Dt);
    if(PendingSpawns>0)
    {
        SpawnTimer-=Dt;
        if(SpawnTimer<=0) { SpawnOneEnemy(); --PendingSpawns; SpawnTimer=.8f; }
    }
}
bool ADungeonGameMode::IsDead() const { auto* H=Player(const_cast<ADungeonGameMode*>(this)); return H&&H->Health<=0; }
void ADungeonGameMode::SpawnWave() { PendingSpawns=IsBossRoom()?1:FMath::Min(9,3+Wave+(Room-1)%4); SpawnTimer=2.f; }
void ADungeonGameMode::SpawnOneEnemy()
{
    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FVector2D Pos;
    if(IsBossRoom()) Pos=FVector2D(640,310);
    else
    {
        float A=FMath::FRandRange(0.f,2*PI);
        Pos=FVector2D(640+FMath::Cos(A)*450,455+FMath::Sin(A)*190);
    }
    if(auto* E=GetWorld()->SpawnActor<ADungeonEnemy>(DungeonView::Unproject(Pos),FRotator::ZeroRotator,Params))
    {
        E->bBoss=IsBossRoom(); E->Species=E->bBoss?24+GetBiome():GetBiome()*6+FMath::RandRange(0,5);
        const auto& S=DungeonRoster::Get(E->Species);
        E->MaxHealth=E->Health=S.HP*(1.f+(Room-1)*.035f);
        Enemies.Add(E);
        PlaySound(TEXT("Spawn"),.4f,E->bBoss?.65f:1.f);
    }
}
void ADungeonGameMode::PlayerAttack(ADungeonHero* H)
{
    const FVector2D P=DungeonView::Project(H->GetActorLocation());
    TArray<TObjectPtr<ADungeonEnemy>> Hits;
    for(auto& E:Enemies) if(IsValid(E)&&E->SpawnTime<=0)
    {
        FVector2D V=DungeonView::Project(E->GetActorLocation())-P;
        if(V.Size()<(E->bBoss?120.f:92.f)&&(V.Size()<32||FVector2D::DotProduct(H->GetAim(),V.GetSafeNormal())>-.15f)) Hits.Add(E);
    }
    for(auto& E:Hits) if(IsValid(E)) E->TakeDungeonDamage(H->AttackPower);
}
void ADungeonGameMode::EnemyDefeated(ADungeonEnemy* E)
{
    Enemies.Remove(E); E->Destroy();
    if(Enemies.IsEmpty()&&PendingSpawns==0)
    {
        if(!IsBossRoom()&&Wave<2) { ++Wave; SpawnWave(); }
        else bChest=true;
    }
}
void ADungeonGameMode::AddImpact(FVector2D P,float Damage,bool Boss)
{
    FDungeonImpact I; I.Position=P; I.Damage=Damage; I.bBoss=Boss; Impacts.Add(I);
}
FDungeonItem ADungeonGameMode::MakeItem(int32 Icon,int32 Rarity)
{
    const TCHAR* Names[]={TEXT("Keepsteel Longsword"),TEXT("Gravebreaker Axe"),TEXT("Cinderbrand"),
        TEXT("Sentinel Plate"),TEXT("Verdant Scale"),TEXT("Warden's Carapace"),
        TEXT("Jade Heart"),TEXT("Sapphire Ward"),TEXT("Emberheart")};
    FDungeonItem I; I.Icon=FMath::Clamp(Icon,0,8); I.Slot=I.Icon/3; I.Rarity=FMath::Clamp(Rarity,0,4); I.Name=Names[I.Icon];
    float Mult=1.f+I.Rarity*.65f;
    if(I.Slot==0) I.Attack=(6+I.Icon*3)*Mult;
    if(I.Slot==1) { I.Defense=(5+(I.Icon-3)*2)*Mult; I.Vitality=5*Mult; }
    if(I.Slot==2) I.Vitality=(20+(I.Icon-6)*8)*Mult;
    return I;
}
void ADungeonGameMode::PlayerInteract(ADungeonHero* H)
{
    if(!H) return;
    if(H->IsInventoryOpen()||IsGameplayBlocked()||H->IsRolling()) return;
    if(IsDead()) { RestartRun(); return; }
    if(bChest)
    {
        int32 Choice=INDEX_NONE; float Nearest=100;
        for(int32 I=0;I<3;++I) { float D=FVector2D::Distance(DungeonView::Project(H->GetActorLocation()),ChestPosition(I)); if(D<Nearest) { Nearest=D; Choice=I; } }
        if(Choice==INDEX_NONE) return;
        if(!ChestRolled[Choice])
        {
            const int32 Roll=FMath::RandRange(1,100);
            int32 Rarity=Roll<=35?0:Roll<=65?1:Roll<=85?2:Roll<=97?3:4;
            ChestLoot[Choice]=MakeItem(Choice*3+FMath::RandRange(0,2),IsBossRoom()?4:Rarity); ChestRolled[Choice]=true;
        }
        Loot=ChestLoot[Choice]; bLootRolled=true;
        if(!H->AddToInventory(Loot)) { LootTimer=8; return; }
        H->Health=FMath::Min(H->MaxHealth,H->Health+35);
        PlaySound(TEXT("Chest"));
        bChest=false; bLootClaimed=true; LootTimer=8; TransitionCooldown=.8f; return;
    }
    if(bLootClaimed&&TransitionCooldown<=0)
    {
        for(int32 I=0;I<3;++I) if(FVector2D::Distance(DungeonView::Project(H->GetActorLocation()),DoorPosition(I))<95)
        { StartTransition(I); return; }
    }
}
void ADungeonGameMode::NextRoom()
{
    bChest=bLootClaimed=bLootRolled=false; LootTimer=0; ++Room; Wave=1;
    for(bool& Rolled:ChestRolled) Rolled=false;
    Shots.Empty(); Splashes.Empty();
    if(auto* H=Player(this)) H->SetActorLocation(DungeonView::Unproject(FVector2D(640,615)));
    SpawnWave();
}
void ADungeonGameMode::RestartRun()
{
    for(auto& E:Enemies) if(IsValid(E)) E->Destroy();
    Enemies.Empty(); Impacts.Empty(); Room=Wave=1; bChest=bLootClaimed=bLootRolled=false; LootTimer=0;
    TransitionTime=0; Shots.Empty(); Splashes.Empty(); for(bool& Rolled:ChestRolled) Rolled=false;
    if(auto* H=Player(this)) H->Restart();
    SpawnWave();
}
FString ADungeonGameMode::GetObjective() const
{
    if(IsDead()) return TEXT("YOU FELL  -  Press E to begin a new run");
    if(bChest) return bLootRolled?TEXT("Bag full - I to make space / choose one chest with E"):TEXT("CHOOSE ONE: weapon, armor or charm chest. The other two will vanish.");
    if(bLootClaimed) return TEXT("Reward collected - I to equip gear / E at a glowing arch to continue");
    if(IsBossRoom()) return PendingSpawns?TEXT("A GUARDIAN AWAKENS"):TEXT("Watch the telegraph / SPACE to dodge / strike during recovery");
    return FString::Printf(TEXT("ROOM %d   /   WAVE %d OF 2   /   %d REMAINING"),Room,Wave,Enemies.Num()+PendingSpawns);
}

UTexture2D* ADungeonHUD::Texture(const FString& Name)
{
    if(auto* T=Textures.Find(Name)) return *T;
    FString Path=FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*Name,*Name);
    auto* T=LoadObject<UTexture2D>(nullptr,*Path);
#if WITH_EDITOR
    if(T&&T->IsCompiling())
    {
        UTexture* Pending[]={T}; FTextureCompilingManager::Get().FinishCompilation(Pending);
    }
#endif
    if(T) T->WaitForPendingInitOrStreaming();
    Textures.Add(Name,T); return T;
}
void ADungeonHUD::Sprite(const FString& N,float X,float Y,float W,float H,FLinearColor Tint,float Rotation,FVector2D Pivot)
{
    if(auto* T=Texture(N)) DrawTexture(T,Offset.X+X*Scale,Offset.Y+Y*Scale,W*Scale,H*Scale,0,0,1,1,Tint,BLEND_Translucent,1,false,Rotation,Pivot);
}
void ADungeonHUD::Box(float X,float Y,float W,float H,FLinearColor Color)
{ DrawRect(Color,Offset.X+X*Scale,Offset.Y+Y*Scale,W*Scale,H*Scale); }
void ADungeonHUD::Label(const FString& T,float X,float Y,FLinearColor C,float Size)
{ DrawText(T,C,Offset.X+X*Scale,Offset.Y+Y*Scale,GEngine->GetMediumFont(),Size*Scale); }
void ADungeonHUD::Ring(FVector2D C,float R,FLinearColor Color,float Width)
{
    for(int32 I=0;I<40;++I)
    {
        float A=I*2*PI/40,B=(I+1)*2*PI/40;
        FVector2D P=C+FVector2D(FMath::Cos(A)*R,FMath::Sin(A)*R*.65f),Q=C+FVector2D(FMath::Cos(B)*R,FMath::Sin(B)*R*.65f);
        DrawLine(Offset.X+P.X*Scale,Offset.Y+P.Y*Scale,Offset.X+Q.X*Scale,Offset.Y+Q.Y*Scale,Color,Width*Scale);
    }
}
void ADungeonHUD::Shadow(FVector2D Center,float Radius)
{
    for(int32 Y=-6;Y<=6;++Y)
    {
        const float Half=Radius*FMath::Sqrt(FMath::Max(0.f,1.f-Y*Y/49.f));
        Box(Center.X-Half,Center.Y+Y*1.5f,Half*2,1.5f,FLinearColor(0,0,0,.28f));
    }
}
void ADungeonHUD::Hero(ADungeonHero* H,float HS)
{
    FVector2D P=DungeonView::Project(H->GetActorLocation());
    if(H->IsRolling())
    {
        Shadow(P,27);
        FString RollName=FString::Printf(TEXT("Roll_%d_%d"),H->GetRollDirection(),FMath::Clamp((int)(H->RollProgress()*8),0,7));
        if(H->Equipment.Num()==3&&H->Equipment[1].Defense>0)
        {
            const TCHAR* Sets[]={TEXT("Sentinel"),TEXT("Verdant"),TEXT("Warden")};
            RollName=FString(Sets[FMath::Clamp(H->Equipment[1].Icon-3,0,2)])+TEXT("_")+RollName;
        }
        KeySprite(TEXT("Tea_")+RollName,P.X-64*HS,P.Y-116*HS,128*HS,128*HS,FLinearColor::White);
        return;
    }
    const int32 D=H->GetFacingDirection(),F=H->GetAnimationFrame();
    FString Name=FString::Printf(TEXT("%s%s_%d_%d"),H->IsAttacking()||H->IsCasting()?TEXT("Attack"):TEXT("Walk"),D%2?TEXT("Diagonal"):TEXT("Cardinal"),D/2,F);
    Shadow(P+FVector2D(0,-2),28);
    FString ArmorName=TEXT("Tea_Base_")+Name;
    if(H->Equipment.Num()==3&&H->Equipment[1].Defense>0)
    {
        const TCHAR* Sets[]={TEXT("Sentinel"),TEXT("Verdant"),TEXT("Warden")};
        ArmorName=TEXT("Tea_")+FString(Sets[FMath::Clamp(H->Equipment[1].Icon-3,0,2)])+TEXT("_")+Name;
    }
    UMaterialInstanceDynamic* Worn=nullptr;
    if(!ArmorName.IsEmpty())
    {
        if(auto* Cached=ArmorMaterials.Find(ArmorName)) Worn=*Cached;
        else if(auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/V2/M_WornArmor.M_WornArmor")))
        {
            Worn=UMaterialInstanceDynamic::Create(Base,this);
            Worn->SetTextureParameterValue(TEXT("ArmorTexture"),Texture(ArmorName));
            Worn->SetTextureParameterValue(TEXT("HeroMask"),Texture(Name));
            ArmorMaterials.Add(ArmorName,Worn);
        }
    }
    if(Worn&&!Worn->GetMaterial()->IsCompiling())
    {
        Worn->SetVectorParameterValue(TEXT("Tint"),H->HurtTime>0?FLinearColor(1,.35f,.3f):FLinearColor::White);
        DrawMaterialSimple(Worn,Offset.X+(P.X-64*HS)*Scale,Offset.Y+(P.Y-116*HS)*Scale,128*HS*Scale,128*HS*Scale);
    }
    else Sprite(Name,P.X-64*HS,P.Y-116*HS,128*HS,128*HS,H->HurtTime>0?FLinearColor(1,.35f,.3f):FLinearColor::White);
    if(H->Equipment.Num()!=3) return;
    if(H->IsCasting())
    {
        if(H->GetCastProgress()<.46f)
        {
            const auto Hand=P+(HeroSockets::Attack[D][F]-FVector2D(64,116))*HS;
            KeySprite(TEXT("TeaFX_0"),Hand.X-17,Hand.Y-22,34,34,FLinearColor::White);
        }
        return;
    }
    const bool Front=D>=2&&D<=6;
    if(H->Equipment[2].Vitality>0&&Front)
        Sprite(ItemArt(H->Equipment[2].Icon),P.X-7*HS,P.Y-58*HS,14*HS,14*HS);
    const auto A=H->GetAim();
    const FVector2D Socket=H->IsAttacking()?HeroSockets::Attack[D][F]:HeroSockets::Walk[D][F];
    const FVector2D Origin=P-FVector2D(64,116)*HS, Hand=Origin+Socket*HS;
    const float Angle=H->IsAttacking()?HeroSockets::Angles[D][F]:(D<=4?145.f:215.f);
    if(!H->Equipment[0].IsEmpty())
    {
        const FVector2D Pivot=HeroSockets::Grip(H->Equipment[0].Icon)/128.;
        const float WeaponSize=96*HS;
        Sprite(ItemArt(H->Equipment[0].Icon),Hand.X-Pivot.X*WeaponSize,Hand.Y-Pivot.Y*WeaponSize,
            WeaponSize,WeaponSize,FLinearColor::White,Angle,Pivot);
        // Re-draw only the gripping fingers over the handle, never a second hero.
        if(auto* T=Texture(Name)) DrawTexture(T,Offset.X+(Hand.X-2.5f*HS)*Scale,Offset.Y+(Hand.Y-2.5f*HS)*Scale,
            5*HS*Scale,5*HS*Scale,(Socket.X-2.5f)/128.,(Socket.Y-2.5f)/128.,5.f/128,5.f/128,
            FLinearColor::White,BLEND_Translucent);
    }
    if(!H->Equipment[0].IsEmpty()&&H->IsAttacking()&&H->GetAttackProgress()>.25f&&H->GetAttackProgress()<.8f)
    {
        float Base=FMath::Atan2(A.Y,A.X);
        float End=Base-1.3f+H->GetAttackProgress()*3.2f;
        for(int I=0;I<12;++I)
        {
            float T=End-I*.065f; FVector2D Q=P+FVector2D(FMath::Cos(T)*76,FMath::Sin(T)*56-30);
            Box(Q.X,Q.Y,4,3,FLinearColor(1,.82f,.4f,1.f-I/12.f));
        }
    }
}
void ADungeonHUD::Enemy(ADungeonEnemy* E)
{
    FVector2D P=DungeonView::Project(E->GetActorLocation());
    const auto& S=DungeonRoster::Get(E->Species);
    const float Size=E->bBoss?225.f:(E->Species==2||E->Species==4?106.f:136.f);
    Shadow(P,E->bBoss?45:22);
    if(E->SpawnTime>0) Ring(P,32+E->SpawnTime*25,FLinearColor(.5f,.25f,1,.8f),3);
    const float Lift=S.Flying?18+FMath::Sin(GetWorld()->GetTimeSeconds()*4+E->Species)*5:0;
    FLinearColor Tint=E->IsHurt()?FLinearColor(1,.45f,.35f):FLinearColor::White;
    Tint.A=FMath::Clamp(1.f-E->SpawnTime/.6f,0.f,1.f);
    KeySprite(E->Species==24?FString::Printf(TEXT("Finance_%d"),E->AnimationFrame()):FString::Printf(TEXT("Creature_%d_%d"),E->Species,E->AnimationFrame()),
        P.X-Size/2,P.Y-Size*116.f/128.f-Lift,Size,Size,Tint,E->Facing==3);
    if(!E->bBoss)
    {
        Box(P.X-25,P.Y-Size-2-Lift,50,4,FLinearColor(.12f,.02f,.025f));
        Box(P.X-25,P.Y-Size-2-Lift,50*E->Health/E->MaxHealth,4,FLinearColor(.8f,.15f,.12f));
    }
}
void ADungeonHUD::KeySprite(const FString& Name,float X,float Y,float W,float H,FLinearColor Tint,bool Flip,float Rotation)
{
    UMaterialInstanceDynamic* M=nullptr;
    const FString Key=TEXT("Key_")+Name;
    if(auto* Cached=ArmorMaterials.Find(Key)) M=*Cached;
    else if(auto* Base=LoadObject<UMaterialInterface>(nullptr,Name.StartsWith(TEXT("TeaFX_"))?TEXT("/Game/Art/V2/M_TeaFX.M_TeaFX"):TEXT("/Game/Art/V2/M_KeySprite.M_KeySprite")))
    {
        M=UMaterialInstanceDynamic::Create(Base,this);
        M->SetTextureParameterValue(TEXT("SpriteTexture"),Texture(Name)); ArmorMaterials.Add(Key,M);
    }
    if(M&&!M->GetMaterial()->IsCompiling())
    {
        M->SetVectorParameterValue(TEXT("Tint"),Tint);
        DrawMaterial(M,Offset.X+X*Scale,Offset.Y+Y*Scale,W*Scale,H*Scale,Flip?1.f:0.f,0,Flip?-1.f:1.f,1,1,false,Rotation,FVector2D(.5,.5));
    }
}
void ADungeonHUD::DrawMenu(ADungeonGameMode* G)
{
    Sprite(TEXT("TeaTitle"),0,0,1280,800);
    const float Time=GetWorld()->GetTimeSeconds();
    for(int I=0;I<22;++I)
    {
        const float X=FMath::Fmod(I*127.3f+FMath::Sin(Time+I)*14,1280.f);
        const float Y=800-FMath::Fmod(Time*(12+I%5*4)+I*51.f,800.f);
        Box(X,Y,2,3,FLinearColor(1,.48f,.10f,.25f+.3f*FMath::Sin(Time+I)*FMath::Sin(Time+I)));
    }
    float MX=-100,MY=-100; if(auto* PC=GetOwningPlayerController()) PC->GetMousePosition(MX,MY);
    const FVector2D Mouse=(FVector2D(MX,MY)-Offset)/Scale;
    auto Button=[&](const TCHAR* Text,float Y)
    {
        const bool Hover=Mouse.X>=150&&Mouse.X<=480&&Mouse.Y>=Y&&Mouse.Y<=Y+48;
        Box(150,Y,330,48,Hover?FLinearColor(.3f,.5f,.36f,.96f):FLinearColor(.015f,.035f,.032f,.94f));
        Box(150,Y,330,1,Gold); Box(150,Y+47,330,1,Gold);
        Box(150,Y,2,48,Gold); Box(478,Y,2,48,Gold);
        Label(Text,182,Y+13,Hover?FLinearColor::White:Gold,1.15f);
    };
    if(G->bShowControls)
    {
        Box(118,372,460,357,Ink); Label(TEXT("HOW TO PLAY"),150,392,Gold,1.25f);
        Label(TEXT("WASD    Move in screen directions"),150,436,Pale);
        Label(TEXT("SHIFT    Sprint / twice walking speed"),150,467,Pale);
        Label(TEXT("SPACE dodge  /  RMB throw tea (10s cooldown)"),150,498,Pale);
        Label(TEXT("MOUSE + LMB    Aim and strike"),150,529,Pale);
        Label(TEXT("E    Choose ONE chest / enter a gate"),150,560,Pale);
        Label(TEXT("I    Inventory & equipment     P    Pause"),150,591,Pale);
        Label(TEXT("A different guardian awaits every fourth room."),150,629,Gold,.85f);
        Button(TEXT("BACK"),666);
    }
    else
    {
        Button(G->HasRun()?TEXT("RESUME DESCENT"):TEXT("BEGIN DESCENT"),396);
        Button(TEXT("HOW TO PLAY"),460);
        Button(G->HasRun()?TEXT("NEW RUN"):TEXT("EXIT"),524);
        if(G->HasRun()) Button(TEXT("EXIT"),588);
        Label(TEXT("Four realms. Twenty-four foes. One way onward."),150,668,Pale,.9f);
        Label(TEXT("ENTER to begin / resume"),150,695,Gold,.85f);
        Label(FString::Printf(TEXT("M: MUSIC %s   /   N: EFFECTS %s"),G->IsMusicMuted()?TEXT("OFF"):TEXT("ON"),G->AreEffectsMuted()?TEXT("OFF"):TEXT("ON")),150,724,Pale,.85f);
    }
}
void ADungeonHUD::InventoryClick()
{
    auto* H=Player(this); auto* PC=GetOwningPlayerController();
    float X=0,Y=0; if(!H||!PC||!PC->GetMousePosition(X,Y)||Scale<=0) return;
    const FVector2D P=(FVector2D(X,Y)-Offset)/Scale;
    auto In=[&](float L,float T,float W,float Height){return P.X>=L&&P.X<L+W&&P.Y>=T&&P.Y<T+Height;};
    if(auto* G=Mode(this)) if(G->IsMenu())
    {
        if(G->bShowControls) { if(In(150,666,330,45)) G->bShowControls=false; return; }
        if(In(150,396,330,48)) { if(G->HasRun()) G->ToggleMenu(); else G->StartGame(); }
        if(In(150,460,330,48)) G->bShowControls=true;
        if(In(150,524,330,48)) { if(G->HasRun()) G->StartGame(); else UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false); }
        if(G->HasRun()&&In(150,588,330,48)) UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);
        return;
    }
    if(!H->IsInventoryOpen()) return;
    if(In(960,112,100,32)) { H->ToggleInventory(); return; }
    if(In(260,202,360,360))
    {
        const FIntPoint C((int32)((P.X-260)/60),(int32)((P.Y-202)/60));
        H->SelectedItem=INDEX_NONE;
        for(int I=0;I<H->Inventory.Num();++I)
        {
            const auto& E=H->Inventory[I]; auto S=E.Item.Size();
            if(C.X>=E.Cell.X&&C.X<E.Cell.X+S.X&&C.Y>=E.Cell.Y&&C.Y<E.Cell.Y+S.Y) { H->SelectedItem=I; break; }
        }
    }
    if(In(665,543,180,36)) H->EquipFromInventory(H->SelectedItem);
    if(In(865,543,180,36)&&H->Inventory.IsValidIndex(H->SelectedItem))
    {
        H->Inventory.RemoveAt(H->SelectedItem); H->SelectedItem=INDEX_NONE;
        H->InventoryMessage=TEXT("Item discarded. Bag space is now available.");
    }
    for(int S=0;S<3;++S) if(In(665+S*132,330,124,28)) H->Unequip(S);
}
void ADungeonHUD::DrawInventory(ADungeonHero* H)
{
    Box(0,0,1280,800,FLinearColor(0,0,0,.7f));
    Box(228,96,852,586,Ink); Box(228,96,852,3,Gold);
    Label(TEXT("INVENTORY"),260,119,Gold,1.45f);
    Label(TEXT("PAUSED  /  I TO CLOSE"),665,124,Pale,.9f);
    Box(960,112,100,32,FLinearColor(.12f,.18f,.19f)); Label(TEXT("CLOSE"),986,119,Pale);
    Label(TEXT("BAG  6 x 6"),260,169,Pale);
    Label(TEXT("EQUIPPED"),665,169,Gold);
    for(int Y=0;Y<6;++Y) for(int X=0;X<6;++X)
        Box(260+X*60,202+Y*60,58,58,FLinearColor(.07f,.11f,.12f));
    for(int I=0;I<H->Inventory.Num();++I)
    {
        const auto& E=H->Inventory[I]; auto S=E.Item.Size();
        const float X=260+E.Cell.X*60,Y=202+E.Cell.Y*60,W=S.X*60-2,Height=S.Y*60-2;
        auto C=RarityColor(E.Item.Rarity);
        Box(X,Y,W,Height,I==H->SelectedItem?Gold:C);
        Box(X+2,Y+2,W-4,Height-4,FLinearColor(.055f,.075f,.085f));
        if(E.Item.Slot==0) Sprite(ItemArt(E.Item.Icon),X-29,Y+3,116,112);
        else Sprite(ItemArt(E.Item.Icon),X+3,Y+3,W-6,Height-6);
    }
    for(int S=0;S<3;++S)
    {
        const float X=665+S*132; const auto& E=H->Equipment[S];
        Label(S==0?TEXT("WEAPON"):S==1?TEXT("ARMOR"):TEXT("CHARM"),X+8,193,Pale,.85f);
        Box(X,214,124,110,FLinearColor(.06f,.10f,.11f));
        if(!E.IsEmpty())
        {
            Sprite(ItemArt(E.Icon),X+12,215,100,100); Box(X,321,124,3,RarityColor(E.Rarity));
        }
        else Label(TEXT("EMPTY"),X+34,257,Pale,.8f);
        Box(X,330,124,28,FLinearColor(.12f,.18f,.19f)); Label(TEXT("UNEQUIP"),X+27,336,Pale,.8f);
    }
    if(H->Inventory.IsValidIndex(H->SelectedItem))
    {
        const auto& E=H->Inventory[H->SelectedItem].Item;
        Label(E.Name,665,388,RarityColor(E.Rarity),1.1f);
        Label(RarityNames[E.Rarity],665,417,RarityColor(E.Rarity),.9f);
        Label(E.Stats().IsEmpty()?TEXT("Starter equipment - no bonus stats"):E.Stats(),665,447,Pale,.9f);
        const auto& Current=H->Equipment[E.Slot];
        Label(TEXT("CURRENT: ")+ (Current.IsEmpty()?TEXT("Empty"):Current.Name),665,481,Pale,.8f);
        Label(Current.Stats(),665,501,Pale,.8f);
        Box(665,543,180,36,FLinearColor(.12f,.32f,.23f)); Label(TEXT("EQUIP / SWAP"),696,552,Pale);
        Box(865,543,180,36,FLinearColor(.28f,.10f,.08f)); Label(TEXT("DISCARD"),920,552,Pale);
    }
    else Label(TEXT("Select an item in the bag to view or equip it."),665,403,Pale,.85f);
    Label(TEXT("Weapons 1 x 2  /  Armor 2 x 2  /  Charms 1 x 1"),260,586,Pale,.85f);
    Label(H->InventoryMessage,260,622,Gold,.8f);
    Label(TEXT("Discard removes the selected bag item permanently for this run."),260,651,Pale,.75f);
}
void ADungeonHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* G=Mode(this); auto* H=Player(this); if(!G||!H||!Canvas) return;
    if(Textures.IsEmpty())
    {
        LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/V2/M_KeySprite.M_KeySprite"));
        for(int I=0;I<16;++I) Texture(FString::Printf(TEXT("TeaFX_%d"),I));
        for(int I=0;I<8;++I) Texture(FString::Printf(TEXT("Finance_%d"),I));
        for(int S=0;S<28;++S) for(int F=0;F<8;++F) Texture(FString::Printf(TEXT("Creature_%d_%d"),S,F));
        for(int D=0;D<4;++D) for(int F=0;F<8;++F)
            for(const auto* Prefix:{TEXT(""),TEXT("Sentinel_"),TEXT("Verdant_"),TEXT("Warden_")})
                Texture(FString::Printf(TEXT("Tea_%sRoll_%d_%d"),Prefix,D,F));
        for(const auto& State:{TEXT("Walk"),TEXT("Attack")}) for(const auto& Group:{TEXT("Cardinal"),TEXT("Diagonal")})
            for(int R=0;R<4;++R) for(int F=0;F<6;++F) Texture(FString::Printf(TEXT("%s%s_%d_%d"),State,Group,R,F));
        for(int R=0;R<4;++R) for(int F=0;F<4;++F)
        { Texture(FString::Printf(TEXT("EnemyWalk_%d_%d"),R,F)); Texture(FString::Printf(TEXT("EnemyAttack_%d_%d"),R,F)); }
        for(int R=0;R<2;++R) for(int F=0;F<4;++F) Texture(FString::Printf(TEXT("BossMotion_%d_%d"),R,F));
    }
    Scale=FMath::Min(Canvas->SizeX/1280.f,Canvas->SizeY/800.f);
    Offset=FVector2D((Canvas->SizeX-1280*Scale)/2,(Canvas->SizeY-800*Scale)/2);
    DrawRect(FLinearColor(.005f,.008f,.012f),0,0,Canvas->SizeX,Canvas->SizeY);
    if(G->IsMenu()) { DrawMenu(G); return; }
    Sprite(G->GetBiome()==0?TEXT("Arena"):FString::Printf(TEXT("Arena%d"),G->GetBiome()),0,0,1280,800);
#if !UE_BUILD_SHIPPING
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonRosterPreview")))
    {
        Box(0,0,1280,800,Ink); Label(TEXT("BESTIARY / 24 ENEMIES + 4 BOSSES"),30,18,Gold);
        for(int I=0;I<28;++I)
        {
            float X=30+(I%7)*178,Y=66+(I/7)*181;
            KeySprite(I==24?FString::Printf(TEXT("Finance_%d"),(int)(GetWorld()->GetTimeSeconds()*8)%8):FString::Printf(TEXT("Creature_%d_%d"),I,(int)(GetWorld()->GetTimeSeconds()*8)%8),X,Y,146,146,FLinearColor::White);
            Label(DungeonRoster::Get(I).Name,X,Y+151,Pale,.75f);
        }
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonRollPreview")))
    {
        Box(0,0,1280,800,Ink); Label(TEXT("DODGE / EIGHT POSES / FOUR DIRECTIONS"),30,18,Gold);
        for(int D=0;D<4;++D) for(int F=0;F<8;++F)
            KeySprite(FString::Printf(TEXT("Tea_Roll_%d_%d"),D,F),16+F*156,58+D*181,146,146,FLinearColor::White);
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonFXPreview")))
    {
        Label(TEXT("TEA / STOCKS / BONDS / RANGED EFFECTS"),30,20,Gold);
        for(int I=0;I<16;++I)
            KeySprite(FString::Printf(TEXT("TeaFX_%d"),I),90+(I%4)*285,55+(I/4)*180,155,155,FLinearColor::White);
        return;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("DungeonEquipmentReview")))
    {
        const auto Position=H->GetActorLocation(); const auto Equipped=H->Equipment;
        H->HurtTime=0;
        const bool Left=FParse::Param(FCommandLine::Get(),TEXT("ReviewLeft"));
        Box(0,0,1280,800,Ink);
        Label(Left?TEXT("LEFT ATTACK / SIX POSES / FOUR OUTFITS"):TEXT("RIGHT ATTACK / SIX POSES / FOUR OUTFITS"),30,20,Gold);
        for(int R=0;R<4;++R) for(int F=0;F<6;++F)
        {
            if(R==0) { H->Equipment[1]=FDungeonItem(); }
            else H->Equipment[1]=ADungeonGameMode::MakeItem(R+2,2);
            H->SetActorLocation(DungeonView::Unproject(FVector2D(140+F*200,197+R*185)));
            H->SetReviewPose(Left?6:2,F); Hero(H,1.1f);
        }
        H->SetActorLocation(Position); H->Equipment=Equipped;
        return;
    }
#endif
    for(int I=0;I<3;++I)
    {
        const FVector2D P=ADungeonGameMode::DoorPosition(I);
        if(G->AreDoorsOpen())
        {
            float Pulse=.65f+.25f*FMath::Sin(GetWorld()->GetTimeSeconds()*3);
            Sprite(TEXT("Portal"),P.X-65,60,130,160,FLinearColor(.4f,1,.65f,Pulse));
            Ring(P,34,FLinearColor(.2f,1,.55f,Pulse),4);
            Label(TEXT("E  ENTER"),P.X-30,220,FLinearColor(.45f,1,.65f),.9f);
        }
        else Label(TEXT("SEALED"),P.X-26,162,FLinearColor(.6f,.42f,.3f),.8f);
    }
    for(auto& E:G->GetEnemies()) if(IsValid(E)&&E->Windup>0)
    {
        const auto& Spec=DungeonRoster::Get(E->Species);
        const float Radius=Spec.AttackStyle==4?Spec.Range:Spec.AttackStyle==0?Spec.Range:35;
        Ring(E->AttackTarget,Radius,FLinearColor(1,.15f,.08f,.9f),3);
        Ring(E->AttackTarget,Radius*(1.f-E->Windup/Spec.Windup),FLinearColor(1,.5f,.12f,.65f),2);
        if(Spec.AttackStyle==2)
        {
            const auto From=DungeonView::Project(E->GetActorLocation());
            DrawLine(Offset.X+From.X*Scale,Offset.Y+From.Y*Scale,Offset.X+E->AttackTarget.X*Scale,Offset.Y+E->AttackTarget.Y*Scale,FLinearColor(1,.25f,.1f),3*Scale);
        }
    }
    if(G->HasChest())
    {
        for(int I=0;I<3;++I)
        {
            const auto P=G->ChestPosition(I);
            Ring(P,45,Gold,3); Sprite(TEXT("Chest"),P.X-60,P.Y-109,120,120);
            Label(I==0?TEXT("E  WEAPON"):I==1?TEXT("E  ARMOR"):TEXT("E  CHARM"),P.X-42,P.Y+16,Gold);
        }
    }
    for(const auto& FX:G->GetSplashes())
    {
        const float Age=.65f-FX.Life,Size=FX.Radius*2*(.7f+.3f*FMath::Clamp(Age/.13f,0.f,1.f));
        KeySprite(FString::Printf(TEXT("TeaFX_%d"),FX.Art),FX.Position.X-Size*.5f,FX.Position.Y-Size*.325f,Size,Size*.65f,FLinearColor(1,1,1,FMath::Min(1.f,FX.Life/.25f)));
    }
    // Draw actor sprites once, sorted by foot depth.
    TArray<AActor*> Actors; Actors.Add(H);
    for(auto& E:G->GetEnemies()) if(IsValid(E)) Actors.Add(E);
    Actors.Sort([](const AActor& A,const AActor& B){return DungeonView::Project(A.GetActorLocation()).Y<DungeonView::Project(B.GetActorLocation()).Y;});
    for(auto* A:Actors) { if(A==H) Hero(H); else Enemy(CastChecked<ADungeonEnemy>(A)); }
    for(const auto& Shot:G->GetShots())
    {
        FVector2D P=Shot.Position;
        const float Angle=Shot.bFriendly?Shot.Age*260:FMath::RadiansToDegrees(FMath::Atan2(Shot.Velocity.Y,Shot.Velocity.X));
        if(Shot.bFriendly)
        {
            Shadow(P,12); const float T=FMath::Clamp(Shot.Age/Shot.FlightTime,0.f,1.f);
            P.Y-=50*(1-T)+FMath::Sin(T*PI)*75;
            Ring(Shot.Target,Shot.BlastRadius,FLinearColor(.95f,.71f,.33f,.3f),1);
        }
        else P.Y-=22;
        const float Size=Shot.Art==1||Shot.Art==2?46:Shot.Art==3?42:Shot.bFriendly?42:48;
        KeySprite(FString::Printf(TEXT("TeaFX_%d"),Shot.Art),P.X-Size*.5f,P.Y-Size*.5f,Size,Size,FLinearColor::White,false,Angle);
    }
    for(const auto& I:G->GetImpacts())
    {
        float Age=.65f-I.Life; FLinearColor C=I.bBoss?FLinearColor(1,.23f,.07f):Gold; C.A=I.Life/.65f;
        if(Age<.25f) for(int J=0;J<9;++J)
        {
            float A=J*2*PI/9; FVector2D Q=I.Position+FVector2D(FMath::Cos(A),FMath::Sin(A))*(12+Age*160)-FVector2D(0,35);
            Box(Q.X,Q.Y,4,4,C);
        }
        if(I.Damage>0) Label(FString::Printf(TEXT("%.0f"),I.Damage),I.Position.X-8,I.Position.Y-75-Age*42,C,1.4f);
        else Ring(I.Position,30+Age*150,C,5);
    }
    Box(24,22,266,100,Ink); Box(24,22,3,100,Gold);
    Label(TEXT("BEARD & BLADE"),40,33,Gold,1.15f);
    Box(40,63,230,12,FLinearColor(.15f,.035f,.025f));
    Box(40,63,230*H->Health/H->MaxHealth,12,FLinearColor(.72f,.12f,.08f));
    Label(FString::Printf(TEXT("%.0f / %.0f HP     ATK %.0f   ARM %.0f"),H->Health,H->MaxHealth,H->AttackPower,H->Armor),40,87,Pale,.85f);
    Label(FString::Printf(TEXT("%s / ROOM %02d"),DungeonRoster::Biome(G->GetBiome()),G->GetRoom()),900,28,Gold,.85f);
    for(int I=0;I<3&&I<H->Equipment.Num();++I)
    {
        const auto& E=H->Equipment[I]; float X=1040+I*66;
        Box(X,56,60,62,Ink); Box(X,116,60,2,RarityColor(E.Rarity));
        if(!E.IsEmpty()) Sprite(ItemArt(E.Icon),X+5,59,50,50);
        else Label(TEXT("--"),X+23,78,Pale);
    }
    for(auto& E:G->GetEnemies()) if(IsValid(E)&&E->bBoss)
    {
        Box(390,27,500,68,Ink); Label(DungeonRoster::Get(E->Species).Name,500,34,Gold);
        Box(410,64,460,11,FLinearColor(.13f,.025f,.025f)); Box(410,64,460*E->Health/E->MaxHealth,11,FLinearColor(.95f,.23f,.06f));
    }
    Box(210,744,860,36,Ink); Label(G->GetObjective(),230,754,Gold,.95f);
    Label(TEXT("WASD MOVE  SHIFT SPRINT  SPACE DODGE  LMB STRIKE  RMB TEA  E INTERACT  I BAG  P MENU"),290,786,Pale,.72f);
    Box(40,132,110,5,FLinearColor(.08f,.1f,.13f)); Box(40,132,110*(1.f-H->GetRollCooldown()/1.15f),5,FLinearColor(.35f,.8f,1));
    Box(166,128,124,25,Ink); Box(170,147,116*(1.f-H->GetPowerCooldown()/10.f),3,Gold);
    Label(H->GetPowerCooldown()>0?FString::Printf(TEXT("RMB TEA  %.1fs"),H->GetPowerCooldown()):TEXT("RMB TEA  READY"),170,130,Gold,.72f);
    if(G->IsLootRevealed())
    {
        auto& L=G->GetLoot(); FLinearColor C=RarityColor(L.Rarity);
        Box(910,268,345,204,Ink); Box(910,268,345,3,C);
        Sprite(ItemArt(L.Icon),926,297,84,84);
        Label(G->HasChest()?TEXT("BAG FULL"):TEXT("COLLECTED"),1020,290,Gold);
        Label(RarityNames[L.Rarity],1020,315,C,.9f);
        Label(L.Name,930,391,C,1.1f);
        Label(L.Stats(),930,423,Pale,.85f);
        Label(G->HasChest()?TEXT("Make space in your bag, then retry the chest"):TEXT("Press I to open your bag and equip this item"),930,451,Pale,.7f);
    }
    if(H->IsInventoryOpen()) DrawInventory(H);
    if(G->IsDead())
    {
        Box(360,305,560,140,Ink);
        Label(TEXT("YOU FELL"),552,336,FLinearColor(1,.3f,.15f),1.6f);
        Label(TEXT("Press E to restart your run"),515,390,Pale);
    }
    if(G->IsTransitioning())
    {
        const float T=G->TransitionProgress();
        Box(0,0,1280,800,FLinearColor(0,0,0,FMath::Clamp((T-.45f)/.45f,0.f,1.f)));
        if(T>.5f) Label(TEXT("DESCENDING TO THE NEXT CHAMBER"),440,370,Gold,1.2f);
    }
}

void ADungeonGameMode::VerifyGameplay()
{
#if !UE_BUILD_SHIPPING
    int Errors=0;
    auto Check=[&](bool B,const TCHAR* What){if(!B){++Errors;UE_LOG(LogTemp,Error,TEXT("DUNGEON_VERIFY: %s"),What);}};
    auto* H=Player(this); Check(H!=nullptr,TEXT("Hero spawned"));
    if(!H) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    H->Restart();
    FVector2D Start=DungeonView::Project(H->GetActorLocation());
    H->MoveForward(1); H->Tick(.1f); H->MoveForward(0);
    FVector2D WalkEnd=DungeonView::Project(H->GetActorLocation());
    Check(WalkEnd.Y<Start.Y&&FMath::Abs(WalkEnd.X-Start.X)<.01f,TEXT("W moves screen-up"));
    H->SetActorLocation(DungeonView::Unproject(Start)); H->SprintPressed(); H->MoveForward(1); H->Tick(.1f);
    FVector2D SprintEnd=DungeonView::Project(H->GetActorLocation());
    Check(FMath::IsNearlyEqual((float)(Start.Y-SprintEnd.Y),(float)(2*(Start.Y-WalkEnd.Y)),.01f),TEXT("Sprint is 2x walk speed"));
    H->SprintReleased(); H->MoveForward(0); H->Restart();
    for(int D=0;D<8;++D)
    {
        float A=D*PI/4;
        Check(DungeonView::Direction(FVector2D(FMath::Sin(A),-FMath::Cos(A)))==D,TEXT("8-way direction mapping"));
    }
    FVector2D Test(811,355);
    Check(DungeonView::Project(DungeonView::Unproject(Test)).Equals(Test,.01f),TEXT("Projection round-trip"));
    PendingSpawns=0; SpawnOneEnemy(); auto* E=Enemies[0].Get(); E->SpawnTime=0;
    H->SetActorLocation(DungeonView::Unproject(FVector2D(640,400)));
    E->SetActorLocation(DungeonView::Unproject(FVector2D(640,468)));
    float Before=E->Health; PlayerAttack(H); Check(E->Health<Before,TEXT("Melee hits without overlap"));
    while(!bChest)
    {
        while(PendingSpawns>0) { SpawnOneEnemy(); --PendingSpawns; }
        const auto Batch=Enemies;
        for(auto& Target:Batch) { Target->SpawnTime=0; Target->TakeDungeonDamage(10000); }
    }
    Check(Room==1&&Wave==2,TEXT("Two waves before reward"));
    H->SetActorLocation(DungeonView::Unproject(FVector2D(640,424)));
    const float BeforeAttack=H->AttackPower,BeforeArmor=H->Armor,BeforeMax=H->MaxHealth;
    PlayerInteract(H); Check(bLootClaimed&&!bChest&&H->Inventory.Num()==1,TEXT("Chest adds one bag item and unlocks"));
    Check(H->AttackPower==BeforeAttack&&H->Armor==BeforeArmor&&H->MaxHealth==BeforeMax,TEXT("Pickup does not equip or change stats"));
    PlayerInteract(H); Check(H->Inventory.Num()==1,TEXT("Claimed chest cannot duplicate reward"));
    FDungeonItem Item=MakeItem(2,4); H->Equip(Item); float Attack=H->AttackPower; H->Equip(Item);
    Check(FMath::IsNearlyEqual(Attack,H->AttackPower),TEXT("Equipment replacement does not stack"));
    H->Equip(MakeItem(5,3)); H->Equip(MakeItem(8,2));
    Check(H->Armor>8&&H->MaxHealth>150,TEXT("Armor and health slots apply"));
    TransitionCooldown=0; H->SetActorLocation(DungeonView::Unproject(DoorPosition(1))); PlayerInteract(H);
    Check(Room==2,TEXT("Door enters room two"));
    PendingSpawns=0; SpawnOneEnemy(); E=Enemies[0]; E->SpawnTime=0;
    Check(E->bBoss&&E->Health==360,TEXT("Room two boss"));
    E->SetActorLocation(H->GetActorLocation()); E->Tick(.01f);
    Check(E->Windup>0,TEXT("Boss telegraphs before damage"));
    E->TakeDungeonDamage(10000); Check(bChest,TEXT("Boss defeat unlocks chest"));
    H->SetActorLocation(DungeonView::Unproject(FVector2D(640,424))); PlayerInteract(H);
    Check(Loot.Rarity==4&&Loot.Icon==2,TEXT("Boss legendary Cinderbrand reward"));
    Check(!Impacts.IsEmpty(),TEXT("Damage creates impact effects"));
    for(int I=0;I<3;++I)
    {
        const int BeforeRoom=Room;
        bLootClaimed=true; TransitionCooldown=0; PendingSpawns=0;
        H->SetActorLocation(DungeonView::Unproject(DoorPosition(I))); PlayerInteract(H);
        Check(Room==BeforeRoom+1,TEXT("All three visible doors usable"));
    }
    H->Restart(); H->Inventory.Empty();
    for(int I=0;I<18;++I) Check(H->AddToInventory(MakeItem(I%3,I%5)),TEXT("18 weapons fit the 6x6 grid"));
    Check(!H->AddToInventory(MakeItem(0,0)),TEXT("19th weapon rejected"));
    for(int I=0;I<H->Inventory.Num();++I) Check(H->CanPlace(H->Inventory[I].Cell,H->Inventory[I].Item.Size(),I),TEXT("Inventory entries in bounds with no overlaps"));
    const FDungeonItem Starter=H->Equipment[0];
    Check(H->EquipFromInventory(2)&&H->Inventory.Num()==18,TEXT("Full inventory permits a weapon swap"));
    Check(H->Equipment[0].Icon==2&&H->Inventory[2].Item.Name==Starter.Name,TEXT("Swap preserves old weapon"));
    Check(!H->Unequip(0)&&H->Equipment[0].Icon==2,TEXT("Full bag cannot lose unequipped item"));
    H->Inventory.Empty();
    for(int I=0;I<9;++I) Check(H->AddToInventory(MakeItem(3+I%3,2)),TEXT("Nine armor pieces fit"));
    Check(!H->AddToInventory(MakeItem(6,0)),TEXT("Armor fills exactly 36 cells"));
    Check(H->EquipFromInventory(2)&&H->Inventory.Num()==9&&H->Equipment[1].Icon==5,TEXT("Full bag armor swap"));
    bChest=true; bLootClaimed=bLootRolled=false; Room=2;
    H->SetActorLocation(DungeonView::Unproject(FVector2D(640,424))); PlayerInteract(H);
    Check(bChest&&!bLootClaimed&&Loot.Icon==2,TEXT("Full bag leaves boss reward in chest"));
    const FString SavedLoot=Loot.Name; PlayerInteract(H);
    Check(Loot.Name==SavedLoot&&H->Inventory.Num()==9,TEXT("Retry neither rerolls nor duplicates loot"));
    H->Inventory.RemoveAt(0); PlayerInteract(H);
    Check(!bChest&&bLootClaimed&&H->Inventory.Num()==9,TEXT("Make space then collect saved reward"));
    H->ToggleInventory(); const auto PausedPos=H->GetActorLocation(); H->MoveForward(1); H->Tick(.1f); H->MoveForward(0);
    Check(H->GetActorLocation().Equals(PausedPos),TEXT("Inventory pauses movement")); H->ToggleInventory();
    H->Inventory.Empty(); H->AddToInventory(MakeItem(8,4)); H->EquipFromInventory(0);
    const float HP=H->Health; H->Unequip(2); H->EquipFromInventory(0);
    Check(H->Health<=HP,TEXT("Vitality swaps cannot heal repeatedly"));
    for(const auto& Set:{TEXT("Sentinel"),TEXT("Verdant"),TEXT("Warden")})
        for(const auto& State:{TEXT("Walk"),TEXT("Attack")}) for(const auto& Group:{TEXT("Cardinal"),TEXT("Diagonal")})
            for(int R=0;R<4;++R) for(int F=0;F<6;++F)
            {
                FString N=FString::Printf(TEXT("%s_%s%s_%d_%d"),Set,State,Group,R,F);
                Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Worn armor animation texture load"));
            }
    Check(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/V2/M_WornArmor.M_WornArmor"))!=nullptr,TEXT("Worn armor material load"));
    for(const auto& State:{TEXT("Walk"),TEXT("Attack")}) for(const auto& Group:{TEXT("Cardinal"),TEXT("Diagonal")})
        for(int R=0;R<4;++R) for(int F=0;F<6;++F)
        {
            FString N=FString::Printf(TEXT("%s%s_%d_%d"),State,Group,R,F);
            FString P=FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N);
            Check(LoadObject<UTexture2D>(nullptr,*P)!=nullptr,TEXT("Animation texture load"));
        }
    for(const auto& State:{TEXT("EnemyWalk"),TEXT("EnemyAttack"),TEXT("BossMotion")})
        for(int R=0;R<(FString(State)==TEXT("BossMotion")?2:4);++R) for(int F=0;F<4;++F)
        {
            FString N=FString::Printf(TEXT("%s_%d_%d"),State,R,F);
            FString P=FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N);
            Check(LoadObject<UTexture2D>(nullptr,*P)!=nullptr,TEXT("Enemy and boss animation texture load"));
        }
    UE_LOG(LogTemp,Display,TEXT("DUNGEON_VERIFY_COMPLETE errors=%d"),Errors);
    FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
