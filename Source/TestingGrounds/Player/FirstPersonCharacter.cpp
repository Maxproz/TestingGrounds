// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "TestingGrounds.h"
#include "FirstPersonCharacter.h"
#include "GameFramework/InputSettings.h"
#include "../Weapons/Gun.h"
#include "../Inventory/PickUp.h"
#include "../Magic/SkillsComponent.h"
#include "../Magic/Skill.h"
#include "MyPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFirstPersonCharacter

AFirstPersonCharacter::AFirstPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);


	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
    
    // magic casting setup
    SkillsRootComp = CreateDefaultSubobject<USceneComponent>(FName("SkillsRootComp"));
    
    // Attach it to our root
    SkillsRootComp->SetupAttachment(RootComponent);
    
    // Create the spring arm components and attach them to their root
    //Create the spring arm components and attach them to their root
    LevelOneSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("LevelOneSpringArm"));
    LevelTwoSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("LevelTwoSpringArm"));
    LevelThreeSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("LevelThreeSpringArm"));
    
    LevelOneSpringArm->SetupAttachment(SkillsRootComp);
    LevelTwoSpringArm->SetupAttachment(SkillsRootComp);
    LevelThreeSpringArm->SetupAttachment(SkillsRootComp);
    
    //Initializing the skills component
    SkillsComponent = CreateDefaultSubobject<USkillsComponent>(FName("SkillsComponent"));
}

void AFirstPersonCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

    // Initalize reference for Item Pickup highlight
    LastItemSeen = nullptr;
    
    // Initalizing our inventory
    Inventory.SetNum(MAX_INVENTORY_ITEMS);
    
    if (GunBlueprint == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("GunBlueprint missing"));
        return;
    }
    Gun = GetWorld()->SpawnActor<AGun>(GunBlueprint);
	Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint")); //Attach gun mesh component to Skeleton, doing it here because the skelton is not yet created in the constructor
    Gun->AnimInstance = Mesh1P->GetAnimInstance();
    
    //InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::TouchStarted);
    if (EnableTouchscreenMovement(InputComponent) == false)
    {
        InputComponent->BindAction("Fire", IE_Pressed, Gun, &AGun::OnFire);
    }
}

void AFirstPersonCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // Raycast every frame
    Raycast();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFirstPersonCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    

	InputComponent->BindAxis("MoveForward", this, &AFirstPersonCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AFirstPersonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AFirstPersonCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AFirstPersonCharacter::LookUpAtRate);
    
    // Action mapping of pickup item
    InputComponent->BindAction("PickUp", IE_Pressed, this, &AFirstPersonCharacter::PickUpItem);

    FInputActionBinding InventoryBinding;
    // We need this bind to execute on pause state
    InventoryBinding.bExecuteWhenPaused = true;
    InventoryBinding.ActionDelegate.BindDelegate(this, FName("HandleInventoryInput"));
    InventoryBinding.ActionName = FName("Inventory");
    InventoryBinding.KeyEvent = IE_Pressed;
    
    // Binding thie Inventory Action
    InputComponent->AddActionBinding(InventoryBinding);
    
    
    InputComponent->BindAction("DropItem", IE_Pressed, this, &AFirstPersonCharacter::DropEquippedItem);
}

void AFirstPersonCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFirstPersonCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		// TODO: OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AFirstPersonCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y * BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void AFirstPersonCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFirstPersonCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFirstPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFirstPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AFirstPersonCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AFirstPersonCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AFirstPersonCharacter::TouchUpdate);
	}
	return bResult;
}

void AFirstPersonCharacter::Raycast()
{
    // Calculating start and end location
    FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
    FVector EndLocation = StartLocation + (FirstPersonCameraComponent->GetForwardVector()
                                           * RaycastRange);
    
    FHitResult RaycastHit;
    
    // Raycast should ignore the character
    FCollisionQueryParams CQP;
    CQP.AddIgnoredActor(this);
    
    // Raycast
    GetWorld()->LineTraceSingleByChannel(
                                         RaycastHit,
                                         StartLocation,
                                         EndLocation,
                                         ECollisionChannel::ECC_WorldDynamic,
                                         CQP);
    
    APickUp* PickUp = Cast<APickUp>(RaycastHit.GetActor());
    
    if (LastItemSeen && LastItemSeen != PickUp)
    {
        // If our character seens a different pickup then disable the glowing effect
        // - on the previous seen item
        LastItemSeen->SetGlowEffect(false);
    }
    
    if (PickUp)
    {
        LastItemSeen = PickUp;
        PickUp->SetGlowEffect(true);
    }// Re-Initalize
    else
    {
        LastItemSeen = nullptr;
    }
}

void AFirstPersonCharacter::PickUpItem()
{
    if (LastItemSeen)
    {
        // Find the first available slot
        int32 AvailableSlot = Inventory.Find(nullptr);
        
        if (AvailableSlot != INDEX_NONE)
        {
            // Add the item to the first valid slot we found
            Inventory[AvailableSlot] = LastItemSeen;
            // Destroy the item from the game
            LastItemSeen->Destroy();
        }
        else
        {
            GLog->Log("You can't carry anymore");
        }
    }
}

void AFirstPersonCharacter::HandleInventoryInput()
{
    AMyPlayerController* Con = Cast<AMyPlayerController>(GetController());
    if (Con) Con->HandleInventoryInput();
}

void AFirstPersonCharacter::SetEquippedItem(UTexture2D* Texture)
{
    if (Texture)
    {
        //For this scenario we make the assumption that
        //every pickup has a unique texture.
        //So, in order to set the equipped item we just check every item
        //inside our Inventory. Once we find an item that has the same image as the
        //Texture image we're passing as a parameter we mark that item as
        // - CurrentlyEquipped.
        for (auto It = Inventory.CreateIterator(); It; It++)
        {
            if ((*It) && (*It)->GetPickupTexture() && (*It)->GetPickupTexture()->HasSameSourceArt(Texture))
            {
                CurrentlyEquippedItem = *It;
                GLog->Log("I've set a new equipped item: " + CurrentlyEquippedItem->GetName());
                break;
            }
        }
    }
    else
    {
        GLog->Log("The player has clicked an empty inventory slot");
    }
}

void AFirstPersonCharacter::DropEquippedItem()
{
    if (CurrentlyEquippedItem)
    {
        int32 IndexOfItem;
        if (Inventory.Find(CurrentlyEquippedItem, IndexOfItem))
        {
            // The location of the drop
            FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 200);
            
            // Making a transform with default rotation and scale.
            // Just setting up the location that was calculated above
            FTransform Transform;
            Transform.SetLocation(DropLocation);
            
            // Default actor spawn parameters
            FActorSpawnParameters SpawnParams;
            
            // Spawning our pickup
            APickUp* PickupToSpawn = GetWorld()->SpawnActor<APickUp>(CurrentlyEquippedItem->GetClass(), Transform, SpawnParams);
            
            if (PickupToSpawn)
            {
                // Unreference the item we've just placed
                Inventory[IndexOfItem] = nullptr;
            }
            
        }
    }
}

FTransform AFirstPersonCharacter::GetFixedSpringArmTransform(USpringArmComponent* SpringArm)
{
    FTransform result;
    if (SpringArm)
    {
        result = SpringArm->GetComponentTransform();
        // We want a fixed location for our transform, since we don't want to spawn our skills
        // - right on top of our character
        result.SetLocation(result.GetLocation() + SpringArm->GetForwardVector() * 100);
    }
    return result;
}

void AFirstPersonCharacter::Fire(bool bShouldFireSecondary)
{
    // This is a dummy logic - we currently only have 2 skills
    TSubclassOf<ASkill> SkillBP = (bShouldFireSecondary && SkillsComponent->SkillsArray.IsValidIndex(1)) ? SkillsComponent->SkillsArray[1] : SkillsComponent->SkillsArray[0];
    
    if (SkillBP)
    {
        FActorSpawnParameters ActorSpawnParams;
        
        TArray<FTransform> SpawnTransforms = GetSpawnTransforms(SkillBP->GetDefaultObject<ASkill>()->GetLevel());
        
        for (int32 i = 0; i < SpawnTransforms.Num(); i++)
        {
            GetWorld()->SpawnActor<ASkill>(SkillBP, SpawnTransforms[i]);
        }
        
    }
}

TArray<FTransform> AFirstPersonCharacter::GetSpawnTransforms(int32 Level)
{
    TArray<FTransform> SpawnPoints;
    switch (Level)
    {
        case 1:
        {
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelOneSpringArm));
            break;
        }
        case 2:
        {
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelTwoSpringArm));
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelThreeSpringArm));
            break;
        }
        case 3:
        {
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelOneSpringArm));
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelTwoSpringArm));
            SpawnPoints.Add(GetFixedSpringArmTransform(LevelThreeSpringArm));
        }
        default:
            break;
    }
    return SpawnPoints;
}



