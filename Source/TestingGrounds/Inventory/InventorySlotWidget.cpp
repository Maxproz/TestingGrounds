$// Fill out your copyright notice in the Description page of Project Settings.

#include "TestingGrounds.h"
#include "InventorySlotWidget.h"
#include "PickUp.h"
#include "../Player/FirstPersonCharacter.h"

void UInventorySlotWidget::SetEquippedItem()
{
    AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    if (Char)
    {
        Char->SetEquippedItem(ItemTexture);
    }
}

void UInventorySlotWidget::SetItemTexture(APickUp* Item)
{
    // If the item is valid update the widget's texture.
    // If not, assign a nullptr to it so the widget won't broadcast wrong info to player
    (Item) ? ItemTexture = Item->GetPickupTexture() : ItemTexture = nullptr;
}



