// Fill out your copyright notice in the Description page of Project Settings.


#include "CRPawn.h"

// Sets default values
ACRPawn::ACRPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	RootComponent = Cube;

}

// Called when the game starts or when spawned
void ACRPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

