// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_Talk_component.h"

// Sets default values
ANPC_Talk_component::ANPC_Talk_component()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPC_Talk_component::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPC_Talk_component::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

