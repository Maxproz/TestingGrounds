// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "FirstPersonCharacter.generated.h"

#define MAX_INVENTORY_ITEMS 4

class UInputComponent;

UCLASS(config=Game)
class AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	AFirstPersonCharacter();

	virtual void BeginPlay() override;
    
    virtual void Tick(float DeltaSeconds) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;
    
    /** Projectile class to spawn */
    UPROPERTY(EditDefaultsOnly, Category= "Setup")
    TSubclassOf<class AGun> GunBlueprint;
    
    // Getter for the Inventory
    TArray<class APickUp*> GetInventory() { return Inventory; }
    
    // Sets a new equipped item based on the given texture
    void SetEquippedItem(UTexture2D* Texture);
    

private:
    // The Gun
    AGun* Gun;
    
    // Raycasts in front of the character to find usable items
    void Raycast();
    
    // Reference to the last seen pickup item. Nullptr if none*/
    class APickUp* LastItemSeen;
    
    // Handles the pickup input
    UFUNCTION()
    void PickUpItem();
    
    // The actual Inventory
    UPROPERTY(VisibleAnywhere)
    TArray<class APickUp*> Inventory;
    
    // Handles the Inventory by sending information to the controller
    UFUNCTION()
    void HandleInventoryInput();
    
    // Reference to the currently equipped item
    class APickUp* CurrentlyEquippedItem;
    
    // Drops the currently equipped item
    UFUNCTION()
    void DropEquippedItem();
    
protected:
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
    // The range of the Raycast
    UPROPERTY(EditAnywhere)
    float RaycastRange = 250.f;
    
    UPROPERTY(EditAnywhere)
    TSubclassOf<class APickUp> PickupBPRef;
    
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

