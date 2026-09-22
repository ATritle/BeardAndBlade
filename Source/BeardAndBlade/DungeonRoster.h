#pragma once
#include "CoreMinimal.h"
struct FDungeonSpecies
{
    const TCHAR* Name;
    float Speed,HP,Damage,Range,Windup,Recovery;
    int32 AttackStyle;
    bool Flying;
};
namespace DungeonRoster
{
// Attack styles: slash, aimed bolt, charge, fan, slam, venom, radial volley.
inline const FDungeonSpecies Species[]={
 {TEXT("Crypt Guard"),61,44,13,48,.55f,.9f,0,false},
 {TEXT("Grave Archer"),43,32,10,280,.85f,1.7f,1,false},
 {TEXT("Carrion Bat"),101,22,8,160,.65f,1.2f,2,true},
 {TEXT("Lantern Wisp"),48,25,9,240,.9f,1.6f,1,true},
 {TEXT("Tomb Rat"),93,24,8,40,.38f,.85f,0,false},
 {TEXT("Bone Hound"),83,38,14,160,.8f,1.3f,2,false},
 {TEXT("Webspinner"),55,40,11,220,.9f,1.8f,5,false},
 {TEXT("Thorn Crawler"),73,44,15,60,.65f,1.1f,0,false},
 {TEXT("Mire Toad"),38,58,13,250,1.f,2.f,5,false},
 {TEXT("Spore Moth"),61,29,9,200,.8f,2.1f,3,true},
 {TEXT("Rootling"),40,67,17,75,1.f,1.6f,4,false},
 {TEXT("Bog Shaman"),45,37,12,280,1.f,2.2f,3,false},
 {TEXT("Frost Knight"),53,72,20,82,.8f,1.4f,0,false},
 {TEXT("Shard Imp"),84,31,11,260,.6f,1.4f,1,false},
 {TEXT("Snow Harpy"),92,39,15,190,.75f,1.5f,2,true},
 {TEXT("Crystal Beetle"),49,83,21,180,1.f,1.8f,2,false},
 {TEXT("Ice Wraith"),46,41,12,260,1.2f,2.4f,6,true},
 {TEXT("Rime Wolf"),96,49,17,175,.7f,1.4f,2,false},
 {TEXT("Ash Raider"),65,69,20,65,.7f,1.2f,0,false},
 {TEXT("Ember Drake"),69,55,14,230,1.f,2.f,3,true},
 {TEXT("Furnace Slime"),37,85,18,95,.95f,1.7f,4,false},
 {TEXT("Cinder Witch"),48,46,16,280,1.f,2.3f,6,false},
 {TEXT("Iron Scorpion"),72,76,22,105,.85f,1.5f,5,false},
 {TEXT("Lava Brute"),35,120,27,120,1.2f,2.f,4,false},
 {TEXT("FINANCE GUY"),54,380,24,300,1.1f,1.6f,7,false},
 {TEXT("THE WEBROOT MATRIARCH"),52,470,23,320,1.1f,1.6f,5,false},
 {TEXT("THE RIME EMPRESS"),56,530,21,340,1.3f,1.9f,6,true},
 {TEXT("THE CINDER WARDEN"),44,660,36,145,1.3f,1.7f,4,false}
};
inline const FDungeonSpecies& Get(int32 I) { return Species[FMath::Clamp(I,0,27)]; }
inline const TCHAR* Biome(int32 I) { const TCHAR* N[]={TEXT("THE FORGOTTEN KEEP"),TEXT("WEBROOT HOLLOWS"),TEXT("GLACIAL RELIQUARY"),TEXT("CINDER FOUNDRY")}; return N[I%4]; }
}
