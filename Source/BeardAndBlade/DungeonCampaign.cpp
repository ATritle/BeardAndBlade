#include "DungeonActors.h"
#include "DungeonRoster.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformMisc.h"

void ADungeonGameMode::StartGame()
{
    bMenu=false; bHasRun=true; bShowControls=false; RestartRun();
    PlaySound(TEXT("UI"));
}
void ADungeonGameMode::ToggleMenu()
{
    if(IsTransitioning()) return;
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0))) if(H->IsInventoryOpen()) H->ToggleInventory();
    if(bMenu&&!bHasRun) { StartGame(); return; }
    bMenu=!bMenu; bShowControls=false;
    PlaySound(TEXT("UI"),.5f);
}
void ADungeonGameMode::StartTransition(int32 Door)
{
    if(!bLootClaimed||TransitionTime>0||!Enemies.IsEmpty()||PendingSpawns>0) return;
    TransitionDoor=FMath::Clamp(Door,0,2); TransitionTime=2.f; Shots.Empty(); Splashes.Empty();
    PlaySound(TEXT("Portal"));
    if(auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0))) TransitionFrom=DungeonView::Project(H->GetActorLocation());
}
void ADungeonEnemy::Tick(float Dt)
{
    Super::Tick(Dt);
    auto* G=Cast<ADungeonGameMode>(UGameplayStatics::GetGameMode(this));
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!G||G->IsGameplayBlocked()||!H||H->IsInventoryOpen()||H->Health<=0||Health<=0) return;
    const auto& S=DungeonRoster::Get(Species);
    HurtTime=FMath::Max(0.f,HurtTime-Dt); bWalking=false;
    if(SpawnTime>0) { SpawnTime=FMath::Max(0.f,SpawnTime-Dt); return; }
    const FVector2D P=DungeonView::Project(GetActorLocation()),Target=DungeonView::Project(H->GetActorLocation());
    if(ChargeTime>0)
    {
        const float Step=FMath::Min(ChargeTime,Dt); ChargeTime=FMath::Max(0.f,ChargeTime-Dt);
        const FVector2D Next=DungeonView::Clamp(P+ChargeAim*(bBoss?360.f:410.f)*Step);
        SetActorLocation(DungeonView::Unproject(Next)); bWalking=true;
        const FVector2D Segment=Next-P;
        const float T=Segment.IsNearlyZero()?0:FMath::Clamp(FVector2D::DotProduct(Target-P,Segment)/Segment.SizeSquared(),0.,1.);
        if(FVector2D::Distance(P+Segment*T,Target)<(bBoss?48:30)) H->ReceiveHit(S.Damage);
        return;
    }
    if(Windup>0)
    {
        Windup=FMath::Max(0.f,Windup-Dt);
        if(Windup<=0) { G->FireAttack(this); Recovery=S.Recovery; }
        return;
    }
    if(Recovery>0) { Recovery=FMath::Max(0.f,Recovery-Dt); return; }
    const FVector2D Delta=Target-P; Facing=Delta.X>=0?1:3;
    if(Delta.Size()<S.Range)
    {
        AttackTarget=Target; ChargeAim=Delta.GetSafeNormal(); Windup=S.Windup*(bBoss&&Health<MaxHealth*.5f?.8f:1.f); return;
    }
    FVector2D Step=Delta.GetSafeNormal()*S.Speed*(bBoss&&Health<MaxHealth*.5f?1.25f:1.f);
    for(auto& Other:G->GetEnemies()) if(IsValid(Other)&&Other!=this)
    {
        FVector2D Apart=P-DungeonView::Project(Other->GetActorLocation());
        if(Apart.SizeSquared()>1&&Apart.SizeSquared()<1600) Step+=Apart.GetSafeNormal()*40;
    }
    if(HurtTime>0) Step*=.25f;
    WalkDistance+=Step.Size()*Dt; bWalking=true;
    SetActorLocation(DungeonView::Unproject(DungeonView::Clamp(P+Step*Dt)));
}
int32 ADungeonEnemy::AnimationFrame() const
{
    const auto& S=DungeonRoster::Get(Species);
    if(Windup>0) return Windup>S.Windup*.5f?4:5;
    if(ChargeTime>0) return 6;
    if(Recovery>0) return Recovery>S.Recovery-.22f?6:7;
    return (bWalking||S.Flying)?(int32)((WalkDistance+ (S.Flying?GetWorld()->GetTimeSeconds()*30:0))/10.f)%4:0;
}
void ADungeonGameMode::FireAttack(ADungeonEnemy* E)
{
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)); if(!H||!IsValid(E)) return;
    const auto& S=DungeonRoster::Get(E->Species);
    PlaySound(E->Species==24?TEXT("Paper"):S.AttackStyle==0?TEXT("Sword"):S.AttackStyle==4?TEXT("Explosion"):S.AttackStyle==2?TEXT("Roll"):TEXT("Magic"),.65f);
    const FVector2D P=DungeonView::Project(E->GetActorLocation());
    if(S.AttackStyle==2) { E->ChargeTime=E->bBoss?.65f:.45f; return; }
    if(S.AttackStyle==0||S.AttackStyle==4)
    {
        const float Radius=S.AttackStyle==4?(E->bBoss?145.f:S.Range):S.Range;
        FVector2D Delta=DungeonView::Project(H->GetActorLocation())-E->AttackTarget; Delta.Y/=.65f;
        if(Delta.Size()<Radius) H->ReceiveHit(S.Damage);
        AddImpact(E->AttackTarget,0,true);
        if(E->Species!=27) return; // The forge boss also throws a radial ember burst.
    }
    int Count=S.AttackStyle==6?(E->bBoss?12:6):S.AttackStyle==3?3:S.AttackStyle==5?(E->bBoss?5:2):1;
    if(E->Species==27) Count=8;
    const bool Ring=S.AttackStyle==6||E->Species==27;
    const bool Bonds=E->Species==24&&(E->AttackCount++%2==1);
    if(E->Species==24) Count=Bonds?3:5;
    const float Base=FMath::Atan2(E->AttackTarget.Y-P.Y,E->AttackTarget.X-P.X);
    for(int I=0;I<Count;++I)
    {
        float A=Ring?I*2*PI/Count:Base+(I-(Count-1)*.5f)*.18f;
        FDungeonShot Shot; Shot.Position=P; Shot.Style=S.AttackStyle; Shot.Damage=S.Damage;
        Shot.Radius=S.AttackStyle==5?12:8;
        Shot.Velocity=FVector2D(FMath::Cos(A),FMath::Sin(A))*(S.AttackStyle==5?125.f:190.f);
        Shot.Art=E->Species==1?3:E->Species==3||E->Species==16?14:
            E->Species==6||E->Species==8||E->Species==22||E->Species==25?6:
            E->Species==9||E->Species==11?7:E->Species==13||E->Species==26?5:4;
        if(E->Species==24) { Shot.Art=Bonds?2:1; Shot.Velocity*=Bonds?.8f:1.25f; }
        Shot.BlastRadius=Shot.Art==3?0:Shot.Art==1?34:Shot.Art==2?72:Shot.Art==6?65:Shot.Art==4?58:48;
        Shot.Origin=P; Shot.Target=E->AttackTarget;
        Shot.Life=Ring?2.3f:FMath::Clamp((float)FVector2D::Distance(P,E->AttackTarget)/(float)Shot.Velocity.Size(),.55f,2.7f);
        Shot.FlightTime=Shot.Life;
        Shots.Add(Shot);
    }
}
void ADungeonGameMode::VerifyCampaign()
{
#if !UE_BUILD_SHIPPING
    int Errors=0; auto Check=[&](bool B,const TCHAR* Text){if(!B){++Errors;UE_LOG(LogTemp,Error,TEXT("CAMPAIGN_VERIFY: %s"),Text);}};
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0));
    Check(H!=nullptr,TEXT("Hero exists")); if(!H) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    Check(bMenu&&PendingSpawns==0,TEXT("Starts at menu with no combat"));
    StartGame(); Check(!bMenu&&PendingSpawns>0,TEXT("Start menu begins run"));
    H->MoveForward(1); auto P=DungeonView::Project(H->GetActorLocation()); H->Tick(.1f); H->MoveForward(0);
    Check(DungeonView::Project(H->GetActorLocation()).Y<P.Y,TEXT("W is screen-up"));
    H->Dodge(); const float HP=H->Health; H->ReceiveHit(100); Check(H->Health==HP&&H->IsRolling(),TEXT("Dodge has invulnerability"));
    H->Tick(.5f); H->Dodge(); Check(!H->IsRolling(),TEXT("Dodge cooldown prevents spam")); H->Tick(1.f);
    ToggleMenu(); P=DungeonView::Project(H->GetActorLocation()); H->MoveRight(1); H->Tick(.2f); H->MoveRight(0);
    Check(DungeonView::Project(H->GetActorLocation()).Equals(P),TEXT("Menu pauses movement")); ToggleMenu();
    for(int S=0;S<28;++S)
    {
        FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* E=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),FVector::ZeroVector,FRotator::ZeroRotator,Params);
        E->Species=S; E->bBoss=S>=24; E->AttackTarget=DungeonView::Project(H->GetActorLocation()); E->ChargeAim=FVector2D(1,0);
        const int Style=DungeonRoster::Get(S).AttackStyle; Shots.Empty(); FireAttack(E);
        Check(Style!=2||E->ChargeTime>0,TEXT("Charge attack starts"));
        Check(Style==0||Style==2||(Style==4&&S!=27)||Shots.Num()>0,TEXT("Ranged attack creates projectiles"));
        E->Destroy(); H->Restart();
    }
    Shots.Empty();
    for(int R=1;R<=16;++R)
    {
        Check(Room==R,TEXT("Sequential room progression"));
        Check(IsBossRoom()==(R%4==0),TEXT("Boss every fourth room"));
        Check(GetBiome()==(R-1)/4,TEXT("Four-room biome progression"));
        int Guard=0;
        while(!bChest&&++Guard<20)
        {
            while(PendingSpawns>0) { SpawnOneEnemy(); --PendingSpawns; }
            if(IsBossRoom()) Check(Enemies.Num()==1&&Enemies[0]->Species==24+GetBiome(),TEXT("Correct unique boss"));
            const auto Batch=Enemies;
            for(auto& E:Batch) { E->SpawnTime=0; E->TakeDungeonDamage(100000); }
        }
        Check(bChest&&Enemies.IsEmpty(),TEXT("Clear spawns chest choices"));
        if(R==1)
        {
            H->Inventory.Empty(); for(int I=0;I<18;++I) H->AddToInventory(MakeItem(0,0));
            H->SetActorLocation(DungeonView::Unproject(ChestPosition(0))); PlayerInteract(H);
            const auto Held=ChestLoot[0]; PlayerInteract(H);
            Check(bChest&&!bLootClaimed&&ChestRolled[0]&&ChestLoot[0].Name==Held.Name,TEXT("Full bag preserves reward and choices"));
        }
        H->Inventory.Empty(); const int Choice=(R-1)%3;
        H->SetActorLocation(DungeonView::Unproject(ChestPosition(Choice)));
        const float ATK=H->AttackPower; PlayerInteract(H);
        Check(H->Inventory.Num()==1&&H->Inventory[0].Item.Slot==Choice,TEXT("Chosen chest category enters inventory"));
        Check(!bChest&&bLootClaimed&&H->AttackPower==ATK,TEXT("Other chests vanish without auto-equip"));
        PlayerInteract(H); Check(H->Inventory.Num()==1,TEXT("No second chest reward"));
        TransitionCooldown=0; H->SetActorLocation(DungeonView::Unproject(DoorPosition(Choice))); PlayerInteract(H);
        Check(IsTransitioning()&&Room==R,TEXT("Gate starts transition, not instant teleport"));
        Tick(2.1f); Check(Room==R+1&&!IsTransitioning(),TEXT("Transition finishes in next room"));
    }
    H->Inventory.Empty(); for(int I=0;I<18;++I) Check(H->AddToInventory(MakeItem(0,0)),TEXT("Weapon bag capacity"));
    Check(!H->AddToInventory(MakeItem(0,0))&&H->EquipFromInventory(0),TEXT("Full bag swaps safely"));
    H->Restart(); Shots.Empty();
    H->PowerMove(); Check(H->IsCasting()&&H->GetPowerCooldown()==10.f,TEXT("RMB tea starts ten-second cooldown"));
    H->PowerMove(); H->Tick(.25f);
    Check(Shots.Num()==1&&Shots[0].bFriendly,TEXT("Tea releases exactly one cup"));
    const float Cooldown=H->GetPowerCooldown(); ToggleMenu(); H->Tick(20.f);
    Check(H->GetPowerCooldown()==Cooldown,TEXT("Menu pauses tea cooldown")); ToggleMenu();
    H->Tick(Cooldown+.01f); Check(H->GetPowerCooldown()==0,TEXT("Tea recharges after ten active seconds"));
    Shots.Empty(); H->Restart();
    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    TArray<ADungeonEnemy*> TeaTargets;
    for(int I=0;I<3;++I)
    {
        auto* E=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),DungeonView::Unproject(FVector2D(640+(I==2?240:I*50),450)),FRotator::ZeroRotator,Params);
        E->Health=E->MaxHealth=200; E->SpawnTime=0; Enemies.Add(E); TeaTargets.Add(E);
    }
    LaunchTea(H,FVector2D(640,450)); const float ExpectedDamage=Shots[0].Damage; UpdateProjectiles(1.f);
    Check(TeaTargets[0]->Health==200-ExpectedDamage&&TeaTargets[1]->Health==200-ExpectedDamage&&TeaTargets[2]->Health==200,TEXT("Tea AoE damages nearby enemies once, not distant enemies"));
    const float Before=TeaTargets[0]->Health; UpdateProjectiles(.2f);
    Check(TeaTargets[0]->Health==Before,TEXT("Tea splash is one hit, not damage every frame"));
    for(auto* E:TeaTargets) { Enemies.Remove(E); E->Destroy(); }
    auto* Finance=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),FVector::ZeroVector,FRotator::ZeroRotator,Params);
    Finance->Species=24; Finance->bBoss=true; Finance->AttackTarget=FVector2D(640,520); Shots.Empty(); FireAttack(Finance);
    Check(Shots.Num()==5&&Shots[0].Art==1,TEXT("Finance Guy throws stock certificates")); Shots.Empty(); FireAttack(Finance);
    Check(Shots.Num()==3&&Shots[0].Art==2&&Shots[0].BlastRadius==72,TEXT("Finance Guy alternates to bond splash attacks")); Finance->Destroy(); Shots.Empty();
    for(int I=0;I<16;++I)
    {
        const FString N=FString::Printf(TEXT("TeaFX_%d"),I);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Projectile/splash art available"));
    }
    for(int I=0;I<8;++I)
    {
        const FString N=FString::Printf(TEXT("Finance_%d"),I);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Finance Guy animation available"));
    }
    for(int S=0;S<28;++S) for(int F=0;F<8;++F)
    {
        FString N=FString::Printf(TEXT("Creature_%d_%d"),S,F);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Unique creature animation frame available"));
    }
    for(int D=0;D<4;++D) for(int F=0;F<8;++F)
    {
        FString N=FString::Printf(TEXT("Roll_%d_%d"),D,F);
        Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Dodge animation frame available"));
    }
    for(const TCHAR* Outfit:{TEXT("Base"),TEXT("Sentinel"),TEXT("Verdant"),TEXT("Warden")})
    {
        for(const TCHAR* State:{TEXT("Walk"),TEXT("Attack")}) for(const TCHAR* Group:{TEXT("Cardinal"),TEXT("Diagonal")})
            for(int D=0;D<4;++D) for(int F=0;F<6;++F)
            {
                const FString N=FString::Printf(TEXT("Tea_%s_%s%s_%d_%d"),Outfit,State,Group,D,F);
                Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Updated hero frame available"));
            }
        for(int D=0;D<4;++D) for(int F=0;F<8;++F)
        {
            const FString Prefix=FString(Outfit)==TEXT("Base")?TEXT("Tea_Roll"):FString::Printf(TEXT("Tea_%s_Roll"),Outfit);
            const FString N=FString::Printf(TEXT("%s_%d_%d"),*Prefix,D,F);
            Check(LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Art/V2/%s.%s"),*N,*N))!=nullptr,TEXT("Updated roll frame available"));
        }
    }
    UE_LOG(LogTemp,Display,TEXT("DUNGEON_CAMPAIGN_VERIFY_COMPLETE errors=%d"),Errors);
    FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
