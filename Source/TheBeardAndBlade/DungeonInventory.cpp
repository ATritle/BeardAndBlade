#include "DungeonActors.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

bool ADungeonHero::MoveBagItem(int32 Index,FIntPoint Cell)
{
    if(!Inventory.IsValidIndex(Index)||!CanPlace(Cell,Inventory[Index].Item.Size(),Index)) return false;
    Inventory[Index].Cell=Cell; SelectedItem=Index; InventoryMessage.Empty(); return true;
}
void ADungeonHUD::CancelInventoryGesture()
{
    DragItem=LastClickedItem=INDEX_NONE; bDragging=false; LastClickTime=-1;
}
void ADungeonHUD::UpdateInventoryDrag(ADungeonHero* H)
{
    if(DragItem==INDEX_NONE) return;
    auto* PC=GetOwningPlayerController(); float X=0,Y=0;
    if(!H->Inventory.IsValidIndex(DragItem)||!PC||Scale<=0||!PC->GetMousePosition(X,Y)) {CancelInventoryGesture();return;}
    DragMouse=(FVector2D(X,Y)-Offset)/Scale;
    if(FVector2D::Distance(DragMouse,DragStart)>6) bDragging=true;
    if(PC->IsInputKeyDown(EKeys::LeftMouseButton)) return;
    if(bDragging)
    {
        const int Index=DragItem;
        bool Dropped=false;
        for(int Slot=0;Slot<3;++Slot)
            if(DragMouse.X>=665+Slot*132&&DragMouse.X<789+Slot*132&&DragMouse.Y>=214&&DragMouse.Y<324)
                if(H->Inventory[Index].Item.Slot==Slot) Dropped=H->EquipFromInventory(Index);
        if(!Dropped&&DragMouse.X>=260&&DragMouse.X<620&&DragMouse.Y>=202&&DragMouse.Y<562)
        {
            const auto TopLeft=DragMouse-DragGrab;
            Dropped=H->MoveBagItem(Index,FIntPoint(FMath::FloorToInt((TopLeft.X-260)/60),FMath::FloorToInt((TopLeft.Y-202)/60)));
        }
        if(!Dropped) H->InventoryMessage=TEXT("Cannot place here.");
        CancelInventoryGesture();
    }
    else DragItem=INDEX_NONE; // A click remains eligible for double-click equipping.
}
void ADungeonHUD::DrawInventoryDrag(ADungeonHero* H)
{
    if(!bDragging||!H->Inventory.IsValidIndex(DragItem)) return;
    const auto& Item=H->Inventory[DragItem].Item; const auto Size=Item.Size();
    const auto P=DragMouse-DragGrab;
    const FIntPoint Cell(FMath::FloorToInt((P.X-260)/60),FMath::FloorToInt((P.Y-202)/60));
    bool Valid=false;
    if(DragMouse.X>=260&&DragMouse.X<620&&DragMouse.Y>=202&&DragMouse.Y<562)
    {
        Valid=H->CanPlace(Cell,Size,DragItem);
        if(Cell.X>=0&&Cell.Y>=0&&Cell.X+Size.X<=6&&Cell.Y+Size.Y<=6)
            Box(260+Cell.X*60,202+Cell.Y*60,Size.X*60-2,Size.Y*60-2,Valid?FLinearColor(0,.4f,.1f,.45f):FLinearColor(.7f,0,0,.45f));
    }
    for(int Slot=0;Slot<3;++Slot) if(DragMouse.X>=665+Slot*132&&DragMouse.X<789+Slot*132&&DragMouse.Y>=214&&DragMouse.Y<324)
    {
        Valid=Item.Slot==Slot;Box(665+Slot*132,214,124,110,Valid?FLinearColor(0,.4f,.1f,.45f):FLinearColor(.7f,0,0,.45f));
    }
    Sprite(Item.Art(),P.X+3,P.Y+3,Size.X*60-8,Size.Y*60-8,FLinearColor(1,1,1,.8f));
}
int32 ADungeonHUD::VerifyInventoryGestures(ADungeonHero* H)
{
    auto* PC=GetOwningPlayerController();if(!PC||!H||!H->IsInventoryOpen()) return 1;
    const auto Bag=H->Inventory;const auto Gear=H->Equipment;const float HP=H->Health;
    int Errors=0;
    auto Check=[&](bool OK){if(!OK)++Errors;};
    auto Mouse=[&](float X,float Y){PC->SetMouseLocation(FMath::RoundToInt(Offset.X+X*Scale),FMath::RoundToInt(Offset.Y+Y*Scale));};
    auto Drag=[&](float X,float Y,float TX,float TY){CancelInventoryGesture();Mouse(X,Y);InventoryClick();Mouse(TX,TY);UpdateInventoryDrag(H);};
    H->Inventory.Empty();H->AddToInventory(ADungeonGameMode::RollItem(8,4));H->AddToInventory(ADungeonGameMode::RollItem(24,2));
    const auto Weapon=H->Inventory[0].Item;
    Drag(280,222,520,462);Check(H->Inventory[0].Cell==FIntPoint(4,4));
    Drag(520,462,340,222);Check(H->Inventory[0].Cell==FIntPoint(4,4)); // occupied armor
    Drag(520,462,640,590);Check(H->Inventory[0].Cell==FIntPoint(4,4)); // outside
    Drag(520,462,830,250);Check(H->Equipment[0].Attack==Gear[0].Attack&&H->Inventory[0].Cell==FIntPoint(4,4)); // wrong slot
    Drag(520,462,720,250);Check(H->Equipment[0].CatalogId==Weapon.CatalogId&&H->Equipment[0].Attack==Weapon.Attack);
    const auto Old=H->Inventory[0].Item;
    CancelInventoryGesture();Mouse(520,462);InventoryClick();InventoryClick();Check(H->Equipment[0].Name==Old.Name&&H->Equipment[0].Attack==Old.Attack);
    Check(!H->MoveBagItem(0,FIntPoint(5,5))&&!H->MoveBagItem(-1,FIntPoint(0,0)));
    H->Inventory=Bag;H->Equipment=Gear;H->RebuildStats();H->Health=HP;H->SelectedItem=INDEX_NONE;H->InventoryMessage.Empty();CancelInventoryGesture();
    return Errors;
}
