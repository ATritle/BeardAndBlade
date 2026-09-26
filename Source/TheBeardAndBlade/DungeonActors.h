#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "DungeonProgression.h"
#include "DungeonDescent.h"
#include "DungeonActors.generated.h"

class UCameraComponent;
class UTexture2D;
class UMaterialInstanceDynamic;
class UAudioComponent;
class USoundBase;
class ADungeonGameMode;
class SBackgroundBlur;
class SBorder;

// Shared virtual canvas: input, combat and rendering use one mapping.
namespace DungeonView
{
    inline FVector2D Project(const FVector& P) { return {640.+(P.Y-P.X)*.24,424.+(P.X+P.Y)*.12}; }
    inline FVector Unproject(const FVector2D& P) { const FVector2D D=P-FVector2D(640,424); return {(D.Y/.12-D.X/.24)*.5,(D.Y/.12+D.X/.24)*.5,0}; }
    inline FVector2D Clamp(const FVector2D& P) { return {FMath::Clamp(P.X,110.,1170.),FMath::Clamp(P.Y,212.,702.)}; }
    inline int32 Direction(const FVector2D& D) { return (FMath::RoundToInt(FMath::Atan2(D.X,-D.Y)/(PI/4))+8)%8; }
}
struct FDungeonItem
{
    FString Name;
    int32 Slot=0, Icon=0, Rarity=0;
    float Attack=0, Defense=0, Vitality=0;
    int32 CatalogId=-1, ItemLevel=1, Effect=0;
    float CritChance=0,CritDamage=0,Speed=0,Regen=0;
    FString Art() const { return CatalogId<0?FString::Printf(TEXT("Item_%d_%d"),Icon/3,Icon%3):FString::Printf(TEXT("Loot_%d"),CatalogId); }
    FString EffectText() const;
    float EquippedScale() const;
    FString Stats() const;
    FIntPoint Size() const { return Slot==0?FIntPoint(1,2):Slot==1?FIntPoint(2,2):FIntPoint(1,1); }
    bool IsEmpty() const { return Name.IsEmpty(); }
};
struct FDungeonBagEntry { FDungeonItem Item; FIntPoint Cell; };
struct FDungeonPotion { FVector2D Position; float Age=0; };
enum class ERewardPhase : uint8 { Closed, Opening, Ejecting, Available, Collected };
struct FRewardPresentation
{
    ERewardPhase Phase=ERewardPhase::Closed;
    int32 Choice=INDEX_NONE;
    float Age=0,CollectedAge=0;
    bool BagFull=false;
    FVector2D Landing=FVector2D::ZeroVector;
};
struct FDungeonBlood { FVector2D Position; float Age=0,Size=80; int32 Variant=0; bool bRemains=false; };
struct FDungeonImpact
{
    FVector2D Position;
    float Life=.65f, Damage=0;
    bool bBoss=false;
};
struct FDungeonShot
{
    TWeakObjectPtr<class ADungeonEnemy> SourceEnemy;
    FVector2D Position,Velocity;
    float Life=4,Radius=9,Damage=10,HitHeight=0;
    int32 Style=1;
    int32 Art=3;
    FVector2D Origin,Target;
    float Age=0,FlightTime=1,BlastRadius=0;
    bool bFriendly=false;
    bool bMotionTuned=false,bTrackingStopped=false;
};
struct FDungeonSplash
{
    FVector2D Position;
    float Radius=50,Life=.65f;
    int32 Art=8;
    bool bFriendly=false;
};
UCLASS()
class THEBEARDANDBLADE_API ADungeonHero : public APawn
{
    GENERATED_BODY()
public:
    ADungeonHero();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void MoveForward(float V) { InputY=-V; }
    void MoveRight(float V) { InputX=V; }
    void SprintPressed() { bSprinting=true; }
    void SprintReleased() { bSprinting=false; }
    void Attack();
    void CancelCombatActions();
    FString AttackQuip;
    float QuipTime=0,QuipCooldown=0;
    void PowerMove();
    void Freedom();
    bool IsCasting() const { return PowerCastTime>0; }
    float GetPowerCooldown() const { return PowerCooldown; }
    float GetCastProgress() const { return 1.f-PowerCastTime/.48f; }
    void Dodge();
    float Stamina=100,MaxStamina=100;
    bool bExhausted=false;
    void UpdateStamina(float Dt,bool Sprinting);
    void SpendStamina(float Amount);
    void Menu();
    void ToggleMusic();
    void ToggleEffects();
    void Confirm();
    void SkipIntro();
    void ToggleIntroMotion();
    void TestFinance();
    void TestMack();
    void TestWebroot();
    void TestRime();
    void TestCinder();
    void TestIce();
    void TestTwister();
    void TestFlashBang();
    void TestGreaseEnemies();
    void TestBunkerEnemies();
    void TestStormEnemies();
    bool IsRolling() const { return RollTime>0; }
    float RollProgress() const { return 1.f-RollTime/.48f; }
    float GetRollCooldown() const { return RollCooldown; }
    int32 GetRollDirection() const { return RollDirection; }
    void TransitionWalk(FVector2D From,FVector2D To,float Progress,float GaitScale=1.f);
    void Interact();
    void Restart();
    void ReceiveHit(float Damage,bool PerProjectile=false);
    bool IsBulletImmune() const;
    FVector2D ScreenVelocity=FVector2D::ZeroVector;
    void Equip(const FDungeonItem& Item);
    void RebuildStats();
    bool HasEffect(int32 Effect) const;
    float CritChance=.05f,CritMultiplier=1.5f,AttackSpeed=1,StaminaRegen=1;
    int32 StrikeCount=0;
#if !UE_BUILD_SHIPPING
    void SetReviewPose(int32 D,int32 F) { Facing=AttackDirection=D; AttackTime=.48f*(1.f-(F+.01f)/6.f); bAttackHit=true; }
    void SetWalkReviewPose(int32 D,int32 F) { Facing=D;AttackTime=PowerCastTime=RollTime=0;bWalking=true;WalkDistance=(F+.01f)*24;IdleBreathBlend=0; }
    void SetIdleReviewPose(int32 D,float Phase) { Facing=D;AttackTime=PowerCastTime=RollTime=HurtTime=0;bWalking=false;IdleBreathBlend=1;BreathPhase=Phase; }
    void SetFlashReviewAim(FVector2D Direction) { Aim=Direction;Facing=DungeonView::Direction(Aim); }
#endif
    void ToggleInventory();
    bool AddToInventory(const FDungeonItem& Item);
    bool EquipFromInventory(int32 Index);
    bool Unequip(int32 Slot);
    bool CanPlace(FIntPoint Cell,FIntPoint Size,int32 Ignore=INDEX_NONE) const;
    bool MoveBagItem(int32 Index,FIntPoint Cell);
    bool IsInventoryOpen() const { return bInventoryOpen; }
    TArray<FDungeonBagEntry> Inventory;
    int32 SelectedItem=INDEX_NONE;
    FString InventoryMessage;
    bool IsAttacking() const { return AttackTime>0; }
    bool IsWalking() const { return bWalking; }
    float WalkCycle() const { return WalkDistance/144.f*2.f*PI; }
    int32 GetFacingDirection() const { return IsCasting()?PowerDirection:IsAttacking()?AttackDirection:Facing; }
    int32 GetAnimationFrame() const;
    float GetAttackProgress() const { return 1.f-AttackTime/.48f; }
    FVector2D GetAim() const { return IsCasting()?PowerAim:IsAttacking()?AttackAim:Aim; }
    float Health=150,MaxHealth=150,AttackPower=24,Armor=8,HurtTime=0;
    float StunTime=0,SlowTime=0,StatusClock=0;
    float FlashBlindTime=0;
    bool ApplyFlashBang(FVector2D Explosion,float Radius);
    void ReceiveFlashStab();
    float IdleBreathBlend=0,BreathPhase=0;
    bool IsDamageImmune() const;
    void ApplyBurgerStatus();
    TArray<FDungeonItem> Equipment;
    UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
private:
    float InputX=0,InputY=0,WalkDistance=0,AttackTime=0,Invulnerable=0;
    float RollTime=0,RollCooldown=0;
    float StaminaDelay=0;
    float PowerCooldown=0,PowerCastTime=0;
    int32 PowerDirection=4;
    FVector2D PowerAim=FVector2D(0,1),PowerTarget;
    bool bTeaReleased=false;
    int32 RollDirection=2;
    FVector2D RollAim=FVector2D(1,0);
    bool bSprinting=false,bWalking=false,bAttackHit=false;
    bool bInventoryOpen=false;
    float FootstepDistance=0;
    int32 Facing=4,AttackDirection=4;
    FVector2D Aim=FVector2D(0,1),AttackAim=FVector2D(0,1);
};
UCLASS()
class THEBEARDANDBLADE_API ADungeonEnemy : public APawn
{
    GENERATED_BODY()
public:
    ADungeonEnemy();
    virtual void Tick(float Dt) override;
    void TakeDungeonDamage(float Damage);
    int32 Species=0;
    int32 AttackCount=0;
    int32 BossAttack=0; // 0 close strike, 1 aimed volley, 2 heavy area strike, 3 radial barrage
    float MotionClock=0,ActionTime=0,ActionDuration=0,ShotTimer=0,FlashTime=0;
    int32 Action=0,ShotsRemaining=0,ShotSerial=0;
    FVector2D ActionFrom,ActionTo;
    float HealthLag=0;
    void TickNewBoss(float Dt,ADungeonHero* H,ADungeonGameMode* G);
    void TickFlashBoss(float Dt,ADungeonHero* H,ADungeonGameMode* G);
    void BeginFlashAmbush(ADungeonHero* H,ADungeonGameMode* G);
    FVector2D DrakeMouth() const;
    float BleedTime=0,PoisonTime=0,SlowTime=0,BleedDPS=0,PoisonDPS=0;
    float AilmentDamage=0,AilmentTick=0;
    void UpdateAilments(float Dt);
    float AttackWindup=1,AttackRadius=90;
    float ChargeTime=0;
    FVector2D ChargeAim=FVector2D::ZeroVector;
    float Health=50,MaxHealth=50,HurtTime=0,SpawnTime=.6f,Windup=0,Recovery=0,FreedomImmuneTime=0;
    float WalkDistance=0;
    int32 Facing=2;
    int32 AnimationFrame() const;
    bool bBoss=false,bWalking=false;
    FVector2D AttackTarget=FVector2D::ZeroVector;
    bool IsHurt() const { return HurtTime>0; }
    bool IsAttacking() const { return Windup>0 || Recovery>.6f; }
    bool IsWalking() const { return bWalking; }
};
// Retained names allow existing maps to load. Presentation belongs to one HUD.
UCLASS()
class THEBEARDANDBLADE_API ADungeonChest : public AActor { GENERATED_BODY() };
UCLASS()
class THEBEARDANDBLADE_API ADungeonDoor : public AActor { GENERATED_BODY() };
UCLASS()
class THEBEARDANDBLADE_API ADungeonGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADungeonGameMode();
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    void PlayerAttack(ADungeonHero* Hero);
    void PlaySound(const FString& Name,float Volume=1.f,float Pitch=1.f);
    void UpdateAudio();
    void RunPackagedSmokeTest();
    void ToggleMusic();
    void ToggleEffects();
    bool IsMusicMuted() const { return bMusicMuted; }
    float GetMasterVolume() const { return MasterVolume; }
    void SetMasterVolume(float Value,bool Save=true);
    bool AreEffectsMuted() const { return bEffectsMuted; }
    void PlayerInteract(ADungeonHero* Hero);
    void EnemyDefeated(ADungeonEnemy* Enemy);
    bool ActivateFreedom(ADungeonHero* Hero);
    void UpdateFreedom(float Dt);
    int32 FreedomKills=0;
    float FreedomTime=0;
    bool IsFreedomActive() const { return FreedomTime>0; }
    float FreedomProgress() const { return 1.f-FreedomTime/3.f; }
    TArray<FDungeonBlood> Blood;
    void RestartRun();
    void FinishRun(bool Victory);
    void UpdateEnding(float Dt);
    void RestartFromEnding();
    void VerifyEndings();
    void VerifyWeekend();
    void RunEndingPreview();
    bool HasEnding() const { return EndState!=0; }
    bool IsVictory() const { return EndState==2; }
    float GetEndingTime() const { return EndTime; }
    void StartGame();
    void StartPlaytestRoom(int32 Number);
    void CompleteRoom();
    bool CanCollectReward(const ADungeonHero* Hero) const;
    void ToggleMenu();
    void StartTransition(int32 Door);
    void FireAttack(ADungeonEnemy* Enemy);
    void LaunchTea(ADungeonHero* Hero,FVector2D Target);
    void UpdateProjectiles(float Dt);
    void ResolveProjectile(const FDungeonShot& Shot,FVector2D Position);
    const TArray<FDungeonSplash>& GetSplashes() const { return Splashes; }
    const TArray<FDungeonPotion>& GetPotions() const { return Potions; }
    void UpdatePotions(float Dt);
    bool IsMenu() const { return bMenu; }
    bool HasRun() const { return bHasRun; }
    bool IsTransitioning() const { return TransitionTime>0; }
    bool IsGameplayBlocked() const { return HasEnding()||bMenu||IsTransitioning()||IsBossIntroActive()||IsBossDialogueActive()||BossGrace>0; }
    bool IsBossIntroActive() const { return BossIntroTime>=0; }
    float GetBossIntroTime() const { return BossIntroTime; }
    bool IsIntroReducedMotion() const { return bIntroReducedMotion; }
    void StartBossIntro();
    void FinishBossIntro();
    void CancelBossIntro();
    void UpdateBossIntro(float Dt);
    void ToggleIntroMotion();
    void VerifyBossIntro();
    bool IsBossDialogueActive() const { return DialogueIndex<DialogueLines.Num(); }
    void BeginBossDialogue();
    void AdvanceBossDialogue(bool Skip=false);
    const FString& GetDialogueLine() const { return DialogueLines[DialogueIndex]; }
    FString GetDialogueSpeaker() const;
    int32 GetDialogueIndex() const { return DialogueIndex; }
    int32 GetDialogueCount() const { return DialogueLines.Num(); }
    bool CanAdvanceDialogue() const { return DialogueWait<=0; }
    float TransitionProgress() const { return 1.f-TransitionTime/DungeonDescent::Duration; }
    int32 GetBiome() const { return DungeonProgression::Themes[DungeonProgression::Chapter(Room)]; }
    int32 GetBossSpecies() const { return DungeonProgression::Bosses[DungeonProgression::Chapter(Room)]; }
    void FireThemeAttack(ADungeonEnemy* Enemy);
    void VerifyProgression();
    void ThrowFlashBang(ADungeonEnemy* Enemy);
    void VerifyFlashBang();
    void UpdateReward(float Dt);
    bool InteractReward(ADungeonHero* H);
    const FRewardPresentation& GetRewardPresentation() const { return Reward; }
    void EmitBossShot(ADungeonEnemy* E,bool Burger);
    void AddBossFX(FVector2D P,int32 Art);
    void VerifySeptember();
    void RunSeptemberSmoke();
    bool bShowControls=false;
    const TArray<FDungeonShot>& GetShots() const { return Shots; }
    void AddImpact(FVector2D P,float Damage,bool bBoss=false);
    FString GetObjective() const;
    int32 GetRoom() const { return Room; }
    int32 GetWave() const { return Wave; }
    const TArray<TObjectPtr<ADungeonEnemy>>& GetEnemies() const { return Enemies; }
    bool HasChest() const { return bChest; }
    bool AreDoorsOpen() const { return bLootClaimed; }
    bool IsLootRevealed() const { return LootTimer>0; }
    bool IsDead() const;
    bool IsBossRoom() const { return Room%DungeonProgression::RoomsPerChapter==0; }
    const FDungeonItem& GetLoot() const { return Loot; }
    const TArray<FDungeonImpact>& GetImpacts() const { return Impacts; }
    static FDungeonItem MakeItem(int32 Icon,int32 Rarity);
    static FDungeonItem RollChestLoot(bool Boss,int32 Level=1);
    static FDungeonItem RollItem(int32 Definition,int32 Rarity,int32 Level=1);
    static FVector2D DoorPosition(int32 I) { return FVector2D(340+I*300,170); }
    static FVector2D ChestPosition(int32 I) { return FVector2D(390+I*250,440); }
private:
    bool bFreedomResolved=false;
    float BossIntroTime=-1;
    bool bIntroReducedMotion=false;
    UPROPERTY() TObjectPtr<UAudioComponent> IntroAudio;
    TArray<FString> DialogueLines;
    int32 DialogueIndex=0;
    float DialogueWait=0,BossGrace=0;
    void SpawnWave();
    void SpawnOneEnemy();
    void NextRoom();
    void VerifyGameplay();
    void VerifyCampaign();
    UPROPERTY() TArray<TObjectPtr<ADungeonEnemy>> Enemies;
    UPROPERTY() TMap<FString,TObjectPtr<USoundBase>> Sounds;
    UPROPERTY() TObjectPtr<UAudioComponent> MusicComponent;
    TMap<FString,double> LastSoundTime;
    FString MusicName;
    bool bMusicMuted=false,bEffectsMuted=false;
    float MasterVolume=1.f;
    int32 Room=1,Wave=1,PendingSpawns=0;
    int32 RosterCursor=0;
    float SpawnTimer=0,LootTimer=0,TransitionCooldown=0;
    bool bChest=false,bLootClaimed=false,bLootRolled=false;
    FDungeonItem Loot;
    FRewardPresentation Reward;
    FDungeonItem ChestLoot[3];
    bool ChestRolled[3]={false,false,false};
    bool bMenu=true,bHasRun=false;
    int32 EndState=0;
    float EndTime=0;
    bool bEndingCleaned=false;
    float TransitionTime=0;
    int32 TransitionDoor=1;
    FVector2D TransitionFrom;
    TArray<FDungeonShot> Shots;
    TArray<FDungeonSplash> Splashes;
    TArray<FDungeonImpact> Impacts;
    TArray<FDungeonPotion> Potions;
};
UCLASS()
class THEBEARDANDBLADE_API ADungeonHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void InventoryClick();
    void CancelInventoryGesture();
    int32 VerifyInventoryGestures(ADungeonHero* H);
    void DialogueClick();
    void EndingClick();
    void PreloadBossIntro(int32 Species);
private:
    void UpdateFlashScreen();
    TSharedPtr<SBackgroundBlur> FlashBlur;
    TSharedPtr<SBorder> FlashWhite;
    void DrawDialogue(ADungeonGameMode* G,ADungeonHero* H);
    void DrawBossIntro(ADungeonGameMode* G);
    void DrawReward(ADungeonGameMode* G,ADungeonHero* H);
    void DrawBossUI(ADungeonEnemy* E);
    void DrawStatus(FVector2D P,float Stun,float Slow,float Poison,float Bleed,bool Immune,float Clock);
    void DrawCombatFX(ADungeonGameMode* G,bool Foreground);
    void SpeechBubble(FVector2D Position,FVector2D Size,FVector2D Speaker);
    void DrawVitals(ADungeonHero* H);
    void DrawPotions(ADungeonGameMode* G);
    void Orb(FVector2D Center,float Fraction,FLinearColor Color);
    void DrawInventory(ADungeonHero* H);
    void UpdateInventoryDrag(ADungeonHero* H);
    void DrawInventoryDrag(ADungeonHero* H);
    int32 DragItem=INDEX_NONE,LastClickedItem=INDEX_NONE;
    bool bDragging=false;
    double LastClickTime=-1;
    FVector2D DragStart,DragGrab,DragMouse,LastClickPosition;
    void DrawMenu(ADungeonGameMode* G);
    bool bVolumeDragging=false;
    TMap<FString,float> MenuHoverAmounts;
    FString HoveredMenuButton;
    void DrawSwordCursor();
    void DrawEnding(ADungeonGameMode* G);
    UTexture2D* Texture(const FString& Name);
    void Sprite(const FString& Name,float X,float Y,float W,float H,FLinearColor Tint=FLinearColor::White,float Rotation=0,FVector2D Pivot=FVector2D(.5,.5));
    void KeySprite(const FString& Name,float X,float Y,float W,float H,FLinearColor Tint,bool Flip=false,float Rotation=0);
    void Box(float X,float Y,float W,float H,FLinearColor Color);
    void Label(const FString& Text,float X,float Y,FLinearColor Color,float Size=1);
    void Ring(FVector2D Center,float Radius,FLinearColor Color,float Width=2);
    void Shadow(FVector2D Center,float Radius,float Opacity=1.f);
    void Hero(ADungeonHero* H,float HS=1.375f);
    void Enemy(ADungeonEnemy* E);
    float Scale=1;
    FVector2D Offset=FVector2D::ZeroVector;
    UPROPERTY() TMap<FString,TObjectPtr<UTexture2D>> Textures;
    UPROPERTY() TMap<FString,TObjectPtr<UMaterialInstanceDynamic>> ArmorMaterials;
};
