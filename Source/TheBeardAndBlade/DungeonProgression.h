#pragma once
#include "CoreMinimal.h"
namespace DungeonProgression
{
    inline constexpr int RoomsPerChapter=3, Chapters=7, CampaignRooms=21;
    inline constexpr int Bosses[]={24,28,30,25,26,27,29};
    inline constexpr int Themes[]={0,6,5,1,2,3,4};
    inline int Chapter(int Room) { return ((FMath::Max(1,Room)-1)/RoomsPerChapter)%Chapters; }
    inline int BossRoom(int Species) { for(int I=0;I<Chapters;++I)if(Bosses[I]==Species)return (I+1)*RoomsPerChapter;return 3; }
    inline int RosterBase(int Biome) { const int Bases[]={0,6,12,18,43,37,31};return Bases[FMath::Clamp(Biome,0,6)]; }
}
