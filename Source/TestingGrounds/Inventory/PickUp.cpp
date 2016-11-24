// Fill out your copyright notice in the Description page of Project Settings.

#include "TestingGrounds.h"
#include "PickUp.h"


// Sets default values
APickUp::APickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    PickupSM = CreateDefaultSubobject<UStaticMeshComponent>(FName("PickupSM"));
    
    PickupTexture = CreateDefaultSubobject<UTexture2D>(FName("ItemTexture"));
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
    
}

// Called every frame
void APickUp::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void APickUp::SetGlowEffect(bool Status)
{
    PickupSM->SetRenderCustomDepth(Status);
}