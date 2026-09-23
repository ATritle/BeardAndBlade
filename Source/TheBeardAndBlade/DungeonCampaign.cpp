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
    UpdateAilments(Dt); if(Health<=0||IsActorBeingDestroyed()) return;
    const float MoveScale=SlowTime>0?.65f:1.f;
    if(SpawnTime>0) { SpawnTime=FMath::Max(0.f,SpawnTime-Dt); return; }
    const FVector2D P=DungeonView::Project(GetActorLocation()),Target=DungeonView::Project(H->GetActorLocation());
    if(ChargeTime>0)
    {
        const float Step=FMath::Min(ChargeTime,Dt); ChargeTime=FMath::Max(0.f,ChargeTime-Dt);
        const FVector2D Next=DungeonView::Clamp(P+ChargeAim*(bBoss?360.f:410.f)*Step);
        SetActorLocation(DungeonView::Unproject(Next)); bWalking=true;
        const FVector2D Segment=Next-P;
        const float T=Segment.IsNearlyZero()?0:FMath::Clamp(FVector2D::DotProduct(Target-P,Segment)/Segment.SizeSquared(),0.,1.);
        if(FVector2D::Distance(P+Segment*T,Target)<(bBoss?48:30)) H->ReceiveHit(S.Damage*(bBoss?1.32f:1.8f));
        return;
    }
    if(Windup>0)
    {
        Windup=FMath::Max(0.f,Windup-Dt);
        if(Windup<=0) { G->FireAttack(this); Recovery=S.Recovery; }
        return;
    }
    if(Recovery>0)
    {
        const float StepTime=FMath::Min(Dt,Recovery); Recovery=FMath::Max(0.f,Recovery-Dt);
        if(bBoss)
        {
            // Reposition between attacks; planted wind-ups still telegraph the next strike.
            const FVector2D Toward=(Target-P).GetSafeNormal();
            const FVector2D Move=FVector2D::Distance(Target,P)>S.Range*.7f?Toward:FVector2D(-Toward.Y,Toward.X);
            auto Next=DungeonView::Clamp(P+Move*S.Speed*StepTime*MoveScale);
            Next.Y=FMath::Max(Next.Y,double(DungeonRoster::RenderSize(Species)*.8f+90));
            WalkDistance+=FVector2D::Distance(P,Next); bWalking=!Next.Equals(P,.01);
            Facing=Move.X>=0?1:3; SetActorLocation(DungeonView::Unproject(Next));
        }
        return;
    }
    const FVector2D Delta=Target-P; Facing=Delta.X>=0?1:3;
    if(Delta.Size()<(bBoss?520.f:S.Range))
    {
        if(bBoss)
        {
            BossAttack=Delta.Size()<150&&AttackCount%2==0?0:1+(AttackCount%3);
            ++AttackCount;
            AttackRadius=BossAttack==0?115.f:BossAttack==2?(Species==27?185.f:145.f):35.f;
            AttackWindup=BossAttack==0?.65f:BossAttack==2?1.5f:1.f;
            AttackTarget=BossAttack==0?P:Target;
            ChargeAim=Delta.GetSafeNormal(); Windup=AttackWindup; return;
        }
        AttackTarget=Target; ChargeAim=Delta.GetSafeNormal(); Windup=S.Windup*(bBoss&&Health<MaxHealth*.5f?.8f:1.f); return;
    }
    FVector2D Step=Delta.GetSafeNormal()*S.Speed*(bBoss&&Health<MaxHealth*.5f?1.25f:1.f)*MoveScale;
    for(auto& Other:G->GetEnemies()) if(IsValid(Other)&&Other!=this)
    {
        FVector2D Apart=P-DungeonView::Project(Other->GetActorLocation());
        if(Apart.SizeSquared()>1&&Apart.SizeSquared()<1600) Step+=Apart.GetSafeNormal()*40;
    }
    if(HurtTime>0) Step*=.25f;
    WalkDistance+=Step.Size()*Dt; bWalking=true;
    auto Next=DungeonView::Clamp(P+Step*Dt);
    if(bBoss) Next.Y=FMath::Max(Next.Y,double(DungeonRoster::RenderSize(Species)*.8f+90));
    SetActorLocation(DungeonView::Unproject(Next));
}
int32 ADungeonEnemy::AnimationFrame() const
{
    const auto& S=DungeonRoster::Get(Species);
    if(Windup>0) return Windup>(bBoss?AttackWindup:S.Windup)*.5f?4:5;
    if(ChargeTime>0) return 6;
    if(Recovery>0&&!bWalking) return Recovery>S.Recovery-.22f?6:7;
    return (bWalking||S.Flying)?(int32)((WalkDistance+ (S.Flying?GetWorld()->GetTimeSeconds()*30:0))/10.f)%4:0;
}
void ADungeonGameMode::FireAttack(ADungeonEnemy* E)
{
    if(IsGameplayBlocked()) return;
    auto* H=Cast<ADungeonHero>(UGameplayStatics::GetPlayerPawn(this,0)); if(!H||!IsValid(E)) return;
    const auto& S=DungeonRoster::Get(E->Species);
    PlaySound(E->Species==24?TEXT("Paper"):S.AttackStyle==0?TEXT("Sword"):S.AttackStyle==4?TEXT("Explosion"):S.AttackStyle==2?TEXT("Roll"):TEXT("Magic"),.65f);
    const FVector2D P=DungeonView::Project(E->GetActorLocation());
    if(E->bBoss)
    {
        const int Art=E->Species==24?(E->BossAttack==2?2:1):E->Species==25?6:E->Species==26?5:4;
        const float Damage=S.Damage*1.32f;
        if(E->BossAttack==0||E->BossAttack==2)
        {
            // Resolve at the locked telegraph position, never the player's new position.
            FDungeonShot Blast; Blast.Art=Art; Blast.Radius=10; Blast.BlastRadius=E->AttackRadius;
            Blast.Damage=Damage*(E->BossAttack==2?1.65f:1.f);
            ResolveProjectile(Blast,E->AttackTarget); AddImpact(E->AttackTarget,0,true);
            PlaySound(E->BossAttack==2?TEXT("Explosion"):TEXT("Sword"),.8f);
            if(E->BossAttack==0||E->Species!=27) return;
            // Warden's heavy furnace slam also throws eight embers.
        }
        const bool Radial=E->BossAttack==3||E->BossAttack==2;
        const int Counts[]={5,7,5,3};
        const int Count=Radial?(E->Species==26?16:12):Counts[FMath::Clamp(E->Species-24,0,3)];
        const float Base=FMath::Atan2(E->AttackTarget.Y-P.Y,E->AttackTarget.X-P.X);
        for(int I=0;I<Count;++I)
        {
            const float A=Radial?Base+I*2*PI/Count:Base+(I-(Count-1)*.5f)*.15f;
            FDungeonShot Shot; Shot.Position=Shot.Origin=P; Shot.Target=E->AttackTarget;
            Shot.Art=Art; Shot.Style=Radial?6:3; Shot.Radius=E->Species==25?12:8;
            Shot.Damage=Damage*(Radial?.7f:.9f); Shot.BlastRadius=E->Species==25?65:40;
            Shot.Velocity=FVector2D(FMath::Cos(A),FMath::Sin(A))*(E->Species==26?280.f:E->Species==25?200.f:240.f);
            Shot.Life=3.5f; Shot.FlightTime=Shot.Life; Shots.Add(Shot);
        }
        return;
    }
    if(S.AttackStyle==2) { E->ChargeTime=E->bBoss?.65f:.45f; return; }
    if(S.AttackStyle==0||S.AttackStyle==4)
    {
        const float Radius=S.AttackStyle==4?(E->bBoss?145.f:S.Range):S.Range;
        FVector2D Delta=DungeonView::Project(H->GetActorLocation())-E->AttackTarget; Delta.Y/=.65f;
        if(Delta.Size()<Radius) H->ReceiveHit(S.Damage*1.8f);
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
        FDungeonShot Shot; Shot.Position=P; Shot.Style=S.AttackStyle; Shot.Damage=S.Damage*1.8f;
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
    for(int Id=0;Id<48;++Id) for(int R=0;R<5;++R)
    {
        const auto Item=RollItem(Id,R,4);
        Check(Item.CatalogId==Id&&Item.Rarity==R&&Item.ItemLevel==4&&!Item.Name.IsEmpty(),TEXT("Catalog identity and rolled level retained"));
        Check(Item.Slot==(Id<24?0:Id<36?1:2),TEXT("Catalog slot and footprint"));
        Check(R!=4||Item.Effect>0,TEXT("Legendary always has signature effect"));
        Check(R!=0||Item.Effect==0,TEXT("Common has no proc effect"));
        H->Equip(Item); const float Power=H->AttackPower,Speed=H->AttackSpeed,Crit=H->CritChance;
        H->Equip(Item); Check(H->AttackPower==Power&&H->AttackSpeed==Speed&&H->CritChance==Crit,TEXT("Rolled stats do not stack on repeated equip"));
    }
    H->Restart();
    {
        const FVector2D Start=DungeonView::Project(H->GetActorLocation());
        H->MoveRight(1); H->Tick(.1f);
        Check(FMath::IsNearlyEqual(H->WalkCycle(),19.f/144.f*2.f*PI,.001f),TEXT("Walk cadence uses a 144-pixel cycle"));
        Check(H->GetAnimationFrame()==0,TEXT("Walking no longer skips poses at 0.1 seconds"));
        H->Restart(); H->MoveRight(1); H->SprintPressed(); H->Tick(.1f);
        Check(FMath::IsNearlyEqual(float(DungeonView::Project(H->GetActorLocation()).X-Start.X),38.f,.01f),TEXT("Sprint retains 2x movement"));
        Check(FMath::IsNearlyEqual(H->WalkCycle(),38.f/1.54f/144.f*2.f*PI,.001f),TEXT("Sprint cadence is slower than translation"));
        H->Restart();
    }
    {
        FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* Target=GetWorld()->SpawnActor<ADungeonEnemy>(ADungeonEnemy::StaticClass(),FVector::ZeroVector,FRotator::ZeroRotator,P);
        Target->SpawnTime=0;Target->Health=Target->MaxHealth=1000;
        Target->BleedTime=4;Target->BleedDPS=10;Target->PoisonTime=6;Target->PoisonDPS=5;Target->SlowTime=3;
        Target->UpdateAilments(2);Check(Target->Health==970,TEXT("Bleed and poison deal elapsed damage"));
        Target->UpdateAilments(10);Check(Target->Health==930&&Target->BleedTime==0&&Target->PoisonTime==0&&Target->SlowTime==0,TEXT("Long frame caps damage at ailment duration"));
        Target->UpdateAilments(10);Check(Target->Health==930,TEXT("Expired ailments stop dealing damage"));
        const auto SavedEnemies=Enemies; Enemies.Empty(); Enemies.Add(Target);
        Target->SetActorLocation(H->GetActorLocation()); Target->Health=Target->MaxHealth=1000000;
        H->Equipment[0].Effect=1; H->Equipment[1].Effect=2;
        FMath::RandInit(210); int Bleeds=0,Poisons=0;
        for(int I=0;I<400;++I) {
            Target->BleedTime=Target->PoisonTime=0; PlayerAttack(H);
            Bleeds+=Target->BleedTime>0; Poisons+=Target->PoisonTime>0;
        }
        Check(Bleeds>=10&&Bleeds<=80&&Poisons>=10&&Poisons<=80,TEXT("Bleed/poison are low-chance procs, not guaranteed"));
        Enemies=SavedEnemies; H->Restart(); Impacts.Empty(); Blood.Empty();
        Target->Destroy();
    }
    bool SeenLoot[9]={false};
    for(int I=0;I<1000;++I) { const auto Item=RollChestLoot(false); SeenLoot[Item.Icon]=true; }
    for(bool Seen:SeenLoot) Check(Seen,TEXT("Mystery pool includes all weapons, armor and amulets"));
    for(int I=0;I<30;++I) Check(RollChestLoot(true).Rarity==4,TEXT("Boss mystery loot remains legendary"));
    H->MoveRight(1); H->SprintPressed(); H->Tick(.1f); H->MoveRight(0); H->SprintReleased();
    Check(FMath::IsNearlyEqual(H->Stamina,97.5f),TEXT("Sprint drains stamina"));
    const float IdleStamina=H->Stamina; H->SprintPressed(); H->Tick(.1f); H->SprintReleased();
    Check(H->Stamina==IdleStamina,TEXT("Stationary shift does not consume stamina"));
    H->Restart(); H->Dodge(); Check(H->Stamina==70,TEXT("Dodge costs thirty stamina"));
    H->Restart(); H->SpendStamina(100); H->Dodge();
    Check(H->bExhausted&&!H->IsRolling(),TEXT("Empty stamina blocks dodge"));
    const auto ExhaustedStart=DungeonView::Project(H->GetActorLocation());
    H->SprintPressed(); H->MoveRight(1); H->Tick(.1f); H->MoveRight(0); H->SprintReleased();
    Check(FMath::IsNearlyEqual(float(DungeonView::Project(H->GetActorLocation()).X-ExhaustedStart.X),19.f,.01f),TEXT("Exhausted sprint falls back to walking"));
    H->Tick(2.f); Check(H->bExhausted&&H->Stamina>0&&H->Stamina<100,TEXT("Partial refill keeps exhaustion lock"));
    ToggleMenu(); const float PausedStamina=H->Stamina; H->Tick(10); ToggleMenu();
    Check(H->Stamina==PausedStamina,TEXT("Menu pauses stamina regeneration"));
    H->Tick(4.f); Check(!H->bExhausted&&H->Stamina==100,TEXT("Full refill unlocks mobility"));
    H->Restart(); H->Stamina=29; H->Dodge(); Check(!H->IsRolling()&&H->Stamina==29,TEXT("Insufficient dodge cost rejected"));
    H->Restart();
    FDungeonPotion TestPotion; TestPotion.Position=DungeonView::Project(H->GetActorLocation()); Potions.Add(TestPotion);
    UpdatePotions(1); Check(Potions.Num()==1,TEXT("Full health preserves potion"));
    H->Health=50; UpdatePotions(.1f);
    Check(Potions.IsEmpty()&&H->Health==102.5f,TEXT("Walk-over potion restores thirty-five percent"));
    Potions.Add(TestPotion); UpdatePotions(1); Check(H->Health==150&&Potions.IsEmpty(),TEXT("Potion clamps to max health"));
    H->Restart();
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
        E->BossAttack=1;
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
            if(IsBossRoom())
            {
                Check(Enemies.Num()==1&&Enemies[0]->Species==24+GetBiome(),TEXT("Correct unique boss"));
                Check(IsBossDialogueActive()&&IsGameplayBlocked(),TEXT("Boss introduction blocks combat"));
                const float BossHP=Enemies[0]->Health,HeroHP=H->Health;
                const FVector HeroPos=H->GetActorLocation(),BossPos=Enemies[0]->GetActorLocation();
                H->MoveRight(1); H->Tick(.2f); H->MoveRight(0); Enemies[0]->Tick(.2f);
                H->ReceiveHit(100); Enemies[0]->TakeDungeonDamage(100); H->PowerMove(); H->Dodge();
                Check(H->Health==HeroHP&&Enemies[0]->Health==BossHP&&H->GetActorLocation().Equals(HeroPos)&&Enemies[0]->GetActorLocation().Equals(BossPos),TEXT("Conversation freezes movement and damage"));
                AdvanceBossDialogue(); Check(DialogueIndex==0,TEXT("Initial settling delay blocks accidental click"));
                Tick(.9f);
                if(R==8) AdvanceBossDialogue(true);
                else while(IsBossDialogueActive()) { AdvanceBossDialogue(); Tick(.2f); }
                Check(!IsBossDialogueActive()&&IsGameplayBlocked(),TEXT("Closing dialogue provides combat grace"));
                H->Attack(); Check(!H->IsAttacking(),TEXT("Closing grace cannot trigger sword"));
                Tick(.8f); Check(!IsGameplayBlocked(),TEXT("Battle starts after dialogue"));
                auto* Boss=Enemies[0].Get(); Boss->SpawnTime=0;
                for(int Move=0;Move<4;++Move)
                {
                    H->Restart(); Shots.Empty(); Splashes.Empty();
                    Boss->BossAttack=Move; Boss->AttackRadius=Move==2?145:115;
                    Boss->AttackTarget=FVector2D(640,500);
                    H->SetActorLocation(DungeonView::Unproject(Boss->AttackTarget));
                    const float Before=H->Health; FireAttack(Boss);
                    if(Move==0||Move==2) Check(H->Health<Before&&!Splashes.IsEmpty(),TEXT("Boss close/heavy attack damages with visible splash"));
                    else Check(Shots.Num()>=3&&Shots[0].Damage>0&&Shots[0].Art>0,TEXT("Boss ranged/radial attack has damaging themed projectiles"));
                }
                H->Restart(); Shots.Empty(); Splashes.Empty();
            }
            const auto Batch=Enemies;
            for(auto& E:Batch) { E->SpawnTime=0; E->TakeDungeonDamage(100000); }
            if(IsBossRoom()) Check(!Potions.IsEmpty(),TEXT("Boss guarantees potion drop"));
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
        Check(H->Inventory.Num()==1&&H->Inventory[0].Item.Icon==ChestLoot[Choice].Icon&&H->Inventory[0].Item.Rarity==ChestLoot[Choice].Rarity,TEXT("Chosen random chest item enters inventory"));
        Check(!bChest&&bLootClaimed&&H->AttackPower==ATK,TEXT("Other chests vanish without auto-equip"));
        PlayerInteract(H); Check(H->Inventory.Num()==1,TEXT("No second chest reward"));
        TransitionCooldown=0; H->SetActorLocation(DungeonView::Unproject(DoorPosition(Choice))); PlayerInteract(H);
        Check(IsTransitioning()&&Room==R,TEXT("Gate starts transition, not instant teleport"));
        Tick(2.1f); Check(Room==R+1&&!IsTransitioning()&&Potions.IsEmpty(),TEXT("Transition finishes and clears old potions"));
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
    Finance->Species=24; Finance->bBoss=true; Finance->BossAttack=1; Finance->AttackTarget=FVector2D(640,520); Shots.Empty(); FireAttack(Finance);
    Check(Shots.Num()==5&&Shots[0].Art==1,TEXT("Finance Guy throws stock certificates")); Shots.Empty(); Splashes.Empty();
    Finance->BossAttack=2; Finance->AttackRadius=145; FireAttack(Finance);
    Check(Splashes.Num()==1&&Splashes[0].Art==9&&Splashes[0].Radius==145,TEXT("Finance Guy heavy bond attack creates paper area splash")); Finance->Destroy(); Shots.Empty();
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
    for(int I=24;I<28;++I)
    {
        Check(DungeonRoster::Get(I).Speed>380.f,TEXT("Boss pursuit exceeds player sprint"));
        Check(DungeonRoster::Get(I).HP>=760.f,TEXT("Boss health increased"));
        Check(DungeonRoster::Get(I).Damage>=31.5f,TEXT("Boss damage increased"));
        Check(I==24||DungeonRoster::RenderSize(I)>=330.f,TEXT("Monster boss enlarged"));
    }
    RestartRun(); bMenu=false;
    FreedomKills=14;
    Check(!ActivateFreedom(H),TEXT("Freedom requires fifteen kills"));
    PendingSpawns=0; SpawnOneEnemy();
    if(!Enemies.IsEmpty()) { Enemies[0]->SpawnTime=0; Enemies[0]->TakeDungeonDamage(100000); }
    Check(FreedomKills==15,TEXT("Fifteenth ordinary kill charges Freedom"));
    Check(ActivateFreedom(H)&&FreedomKills==0,TEXT("Freedom consumes charge"));
    Check(!ActivateFreedom(H),TEXT("Freedom cannot be activated twice"));
    const float BeforeFreedomHealth=H->Health; H->ReceiveHit(10000);
    Check(H->Health==BeforeFreedomHealth,TEXT("Freedom protects player"));
    UpdateFreedom(1.5f);
    Check(Enemies.IsEmpty()&&PendingSpawns==0&&bChest,TEXT("Freedom clears queued waves and reveals chests"));
    Check(FreedomKills==0,TEXT("Freedom kills cannot recharge itself"));
    UpdateFreedom(10.f); Check(!IsFreedomActive(),TEXT("Freedom ends even with a long frame"));
    RestartRun(); Check(FreedomKills==0&&Blood.IsEmpty(),TEXT("Restart clears combat effect state"));
    UE_LOG(LogTemp,Display,TEXT("DUNGEON_CAMPAIGN_VERIFY_COMPLETE errors=%d"),Errors);
    FPlatformMisc::RequestExitWithStatus(false,Errors?1:0);
#endif
}
