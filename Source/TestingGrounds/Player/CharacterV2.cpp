// Fill out your copyright notice in the Description page of Project Settings.

#include "TestingGrounds.h"
#include "Components/TextRenderComponent.h"
#include "../Weapons/Gun.h"
#include "CharacterV2.h"


// Sets default values
ACharacterV2::ACharacterV2()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    
    // set our turn rates for input
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;
    
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    
    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
    
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);
    Mesh1P->SetupAttachment(CameraBoom);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
    Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
    
    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
    
    // Init text render comp
    CharText = CreateDefaultSubobject<UTextRenderComponent>(FName("CharText"));
    
    // Set a relative location
    CharText->SetRelativeLocation(FVector(0,0,100));
    
    // attach it to root comp
    CharText->SetupAttachment(GetRootComponent());
    
}

// Called when the game starts or when spawned
void ACharacterV2::BeginPlay()
{
	Super::BeginPlay();
	
    InitHealth();
    InitBombCount();
    
    if (GunBlueprint == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("GunBlueprint missing"));
        return;
    }
    
    Gun = GetWorld()->SpawnActor<AGun>(GunBlueprint);
    Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint")); //Attach gun mesh component to Skeleton, doing it here because the skelton is not yet created in the constructor
    Gun->AnimInstance = Mesh1P->GetAnimInstance();
}

// Called every frame
void ACharacterV2::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void ACharacterV2::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    PlayerInputComponent->BindAxis("MoveForward", this, &ACharacterV2::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACharacterV2::MoveRight);
    
    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &ACharacterV2::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ACharacterV2::LookUpAtRate);
    
    PlayerInputComponent->BindAction("ThrowBomb", IE_Pressed, this, &ACharacterV2::AttempToSpawnBomb);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, Gun, &AGun::OnFire);
}

void ACharacterV2::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Tell the engine to call the OnRep_Health and OnRep_BombCount each time
    // - a variable changes
    DOREPLIFETIME(ACharacterV2, Health);
    DOREPLIFETIME(ACharacterV2, BombCount);
}

void ACharacterV2::OnRep_Health()
{
    UpdateCharText();
}

void ACharacterV2::OnRep_BombCount()
{
    UpdateCharText();
}

void ACharacterV2::InitHealth()
{
    Health = MaxHealth;
    UpdateCharText();
}

void ACharacterV2::InitBombCount()
{
    BombCount = MaxBombCount;
    UpdateCharText();
}

void ACharacterV2::UpdateCharText()
{
    //Create a string that will display the health and bomb count values
    FString NewText =
    FString("Health: ") + FString::SanitizeFloat(Health) + FString(" Bomb Count: ") + FString::FromInt(BombCount);
    
    //Set the created string to the text render comp
    CharText->SetText(FText::FromString(NewText));
}


float ACharacterV2::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    
    // Decrease the character's hp
    
    Health -= Damage;
    if (Health <= 0) InitHealth();
    
    //Call the update text on the local client
    //OnRep_Health will be called in every other client so the character's text
    //will contain a text with the right values
    
    UpdateCharText();
    
    return Health;
}

void ACharacterV2::ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool ACharacterV2::ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    //Assume that everything is ok without any further checks and return true
    return true;
}

void ACharacterV2::AttempToSpawnBomb()
{
    if (HasBombs())
    {
        //If we don't have authority, meaning that we're not the server
        //tell the server to spawn the bomb.
        //If we're the server, just spawn the bomb - we trust ourselves.
        if (Role < ROLE_Authority)
        {
            ServerSpawnBomb();
        }
        else SpawnBomb();
    }
}

void ACharacterV2::SpawnBomb()
{
    //Decrease the bomb count and update the text in the local client
    //OnRep_BombCount will be called in every other client
    BombCount--;
    UpdateCharText();
    
    FActorSpawnParameters SpawnParameters;
    
    SpawnParameters.Instigator = this;
    SpawnParameters.Owner = GetController();
    
    // Spawn the bomb
    GetWorld()->SpawnActor<ABomb>(
                                  BombActorBP,
                                  GetActorLocation() + GetActorForwardVector() * 200,
                                  GetActorRotation(),
                                  SpawnParameters);
}

void ACharacterV2::ServerSpawnBomb_Implementation()
{
    SpawnBomb();
}

bool ACharacterV2::ServerSpawnBomb_Validate()
{
    return true;
}

void ACharacterV2::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterV2::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterV2::MoveForward(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f))
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ACharacterV2::MoveRight(float Value)
{
    if ( (Controller != NULL) && (Value != 0.0f) )
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        // add movement in that direction
        AddMovementInput(Direction, Value);
    }
}