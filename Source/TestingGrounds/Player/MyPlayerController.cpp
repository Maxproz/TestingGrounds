// Fill out your copyright notice in the Description page of Project Settings.

#include "TestingGrounds.h"
#include "MyPlayerController.h"
#include "FirstPersonCharacter.h"
#include "../Inventory/InventoryWidget.h"

void AMyPlayerController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    if (InventoryWidgetBP)
    {
        // Create the Inventory Widget based on the Blueprint reference we will
        // - input from within the Editor
        InventoryWidgetRef = CreateWidget<class UInventoryWidget>(this,
                                                                  InventoryWidgetBP);
    }
    
    // Initial value
    bIsInventoryOpen = false;
}

void AMyPlayerController::HandleInventoryInput()
{
    AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(GetPawn());
    if (InventoryWidgetRef)
    {
        if (bIsInventoryOpen)
        {
            // Mark the inventory as closed
            bIsInventoryOpen = false;
            
            // remove it from the viewport
            InventoryWidgetRef->RemoveFromViewport();
            
            // Hide the cursor of our game
            bShowMouseCursor = false;
            
            // Tell the game that we want to register inputs for our game only
            // - and not for our UI
            FInputModeGameOnly InputMode;
            SetInputMode(InputMode);
            
            // Unpause the game
            SetPause(false);
        }
        else
        {
            // Mark the inventory as open
            bIsInventoryOpen = true;
            
            // Re-populate the ItemsArray
            InventoryWidgetRef->ItemsArray = Char->GetInventory();
            
            // Show the inventory
            InventoryWidgetRef->Show();
            
            // Show the cursor of our game
            bShowMouseCursor = true;
            
            // Tell the game that we want to register inputs both for our game and UI
            FInputModeGameAndUI InputMode;
            SetInputMode(InputMode);
            
            // Pause the game
            SetPause(true);
        }
    }
    
}
