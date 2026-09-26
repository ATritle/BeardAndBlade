#pragma once
#include "CoreMinimal.h"
// Hand centers measured on the athletic frames, in normalized 128px coordinates.
// Physical textures are 384px; sockets remain independent of screen/UI scale.
namespace AthleticHeroSockets
{
inline const float Angles[8][6]={
 {160,-25,130,65,15,160},{150,-30,125,80,20,150},
 {145,-35,135,90,50,145},{150,-25,160,115,50,150},
 {160,-20,175,180,100,160},{210,25,-160,-115,-50,210},
 {215,35,-135,-90,-50,215},{210,30,-125,-80,-20,210}
};
inline const FVector2D Walk[8][6]={
 {{87.9f,72.0f},{87.6f,72.0f},{84.9f,72.0f},{82.2f,70.8f},{82.7f,71.6f},{89.4f,71.6f}},
 {{98.4f,70.4f},{95.7f,69.2f},{93.8f,71.2f},{96.4f,70.0f},{94.5f,69.2f},{95.5f,70.8f}},
 {{87.1f,71.5f},{76.6f,78.4f},{88.5f,69.1f},{90.3f,68.6f},{88.4f,67.8f},{91.8f,68.2f}},
 {{28.5f,70.3f},{31.5f,70.7f},{30.8f,70.7f},{30.5f,71.1f},{33.5f,70.7f},{34.4f,70.7f}},
 {{43.1f,72.6f},{55.5f,69.4f},{43.0f,73.0f},{54.5f,68.9f},{44.1f,72.6f},{54.4f,68.5f}},
 {{28.5f,68.9f},{29.4f,69.8f},{28.7f,68.9f},{30.1f,69.4f},{29.8f,70.6f},{31.2f,69.8f}},
 {{39.1f,72.1f},{41.2f,72.1f},{42.2f,71.3f},{37.5f,70.1f},{42.4f,71.7f},{37.2f,70.1f}},
 {{77.3f,51.8f},{78.6f,53.8f},{80.0f,58.3f},{76.5f,51.8f},{78.2f,53.4f},{78.0f,58.7f}},
};
inline const FVector2D Attack[8][6]={
 {{87.1f,64.3f},{81.1f,21.2f},{103.2f,83.4f},{106.2f,36.6f},{81.9f,41.5f},{83.3f,65.5f}},
 {{96.4f,64.7f},{78.2f,17.9f},{101.2f,74.8f},{108.2f,40.7f},{47.3f,35.4f},{91.f,65.5f}},
 {{67.1f,62.5f},{62.f,15.3f},{104.f,76.3f},{108.2f,41.4f},{88.4f,50.7f},{64.5f,62.9f}},
 {{50.4f,57.2f},{53.8f,15.3f},{75.9f,84.1f},{106.2f,49.5f},{43.3f,51.9f},{43.f,56.8f}},
 {{45.6f,68.9f},{48.9f,12.7f},{69.4f,78.6f},{53.3f,49.4f},{51.4f,50.6f},{47.1f,66.f}},
 {{39.9f,75.f},{49.8f,15.6f},{67.8f,81.9f},{22.f,46.1f},{64.8f,52.6f},{74.7f,61.6f}},
 {{38.2f,63.5f},{42.f,12.6f},{28.7f,75.3f},{21.2f,39.9f},{41.2f,51.3f},{38.2f,63.5f}},
 {{86.2f,53.0f},{72.1f,19.1f},{27.5f,74.f},{20.3f,41.5f},{72.5f,37.8f},{83.3f,54.2f}},
};
// Rest on a neutral movement pose: independently generated idle art had a
// smaller head and narrower torso despite sharing the same overall height.
inline constexpr int IdleFrame[8]={1,1,1,1,1,1,3,1};
inline FVector2D IdleHand(int D) { return Walk[D][IdleFrame[D]]; }
// Omit the few generated poses that punch with the left arm. The animation
// timeline stays six beats, while the valid right-handed strike is held longer.
inline constexpr int AttackPose[8][6]={
 {0,1,2,3,4,5},{0,1,2,3,5,5},{0,1,2,3,4,5},{0,1,2,2,4,5},
 {0,1,2,3,4,5},{0,1,3,3,4,0},{0,1,3,3,4,0},{0,1,3,3,4,5}
};
// Leftward strikes turn the torso into the authored SW right-arm followthrough.
// The old W/NW extension artwork animated the near LEFT arm instead.
inline int AttackView(int D,int Beat) { return D>=6&&(Beat==2||Beat==3)?5:D; }
inline bool RightHandBehindBody(int D) { return D==6||D==7; }
}
