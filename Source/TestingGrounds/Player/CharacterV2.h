// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "../Weapons/Bomb.h"
#include "CharacterV2.generated.h"

UCLASS()
class TESTINGGROUNDS_API ACharacterV2 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterV2();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

    // Marks the properties we wish to replicate
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    /** Bomb Blueprint */
    UPROPERTY(EditAnywhere)
    TSubclassOf<ABomb> BombActorBP;
    
    /** Projectile class to spawn */
    UPROPERTY(EditDefaultsOnly, Category= "Setup")
    TSubclassOf<class AGun> GunBlueprint;
    
    /** Pawn mesh: 1st person view (arms; seen only by self) */
    UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
    class USkeletalMeshComponent* Mesh1P;
    
protected:
    
    // The health of the character
    UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_Health, Category = Stats)
    float Health;
    
    // The max health of the character
    UPROPERTY(EditAnywhere, Category = Stats)
    float MaxHealth = 100.f;
    
    // Then number of bombs that the character carries
    UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_BombCount, Category = Stats)
	int32 BombCount;
    
    // The max number of bombs that a character can have
    UPROPERTY(EditAnywhere, Category = Stats)
    int32 MaxBombCount = 3;
    
    // Text render component - used instead of UMG, can replace later
    UPROPERTY(VisibleAnywhere)
    class UTextRenderComponent* CharText;
    
private:
    AGun* Gun;
    
    // Called when the Health variable gets updated
    UFUNCTION()
    void OnRep_Health();
    
    // Called when the BombCount variable gets updated
    UFUNCTION()
    void OnRep_BombCount();
    
    // Initalizes health
    void InitHealth();
    
    // Initalizes bomb count
    void InitBombCount();
    
    // Updates the character's text to match with the updated stats
    void UpdateCharText();
    
    
    
    // SERVER RELATED STUFF //
private:
    
    /**
     * TakeDamage Server version. Call this instead of TakeDamage when you're a client
     * You don't have to generate an implementation.
     * It will automatically call the ServerTakeDamage_Implementation function
     */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
    
    /** Contains the actual implementation of the ServerTakeDamage function */
    void ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
    
    /** Validates the client. If the result is false the client will be disconnected */
    bool ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
    
    // Bomb related functions
    
    /** Will try to spawn a bomb */
    void AttempToSpawnBomb();
    
    /** Returns true if we can throw a bomb */
    bool HasBombs() { return BombCount > 0; }
    
    /**
     * Spawns a bomb. Call this function when you're authorized to.
     * In case you're not authorized, use the ServerSpawnBomb function.
     */
    void SpawnBomb();
    
    /**
     * SpawnBomb Server version. Call this instead of SpawnBomb when you're a client
     * You don't have to generate an implementation for this. It will automatically call the ServerSpawnBomb_Implementation function
     */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSpawnBomb();
    
    /** Contains the actual implementation of the ServerSpawnBomb function */
    void ServerSpawnBomb_Implementation();
    
    /** Validates the client. If the result is false the client will be disconnected */
    bool ServerSpawnBomb_Validate();
    
private:
    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    
    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;
    
public:
    /** Applies damage to the character */
    virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    
    
public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    
    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float BaseTurnRate;
    
    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float BaseLookUpRate;
    
protected:
    /** Called for forwards/backward input */
    void MoveForward(float Value);
    
    /** Called for side to side input */
    void MoveRight(float Value);
    
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
};
