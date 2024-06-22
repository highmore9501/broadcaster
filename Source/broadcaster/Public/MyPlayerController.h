// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BROADCASTER_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "MyPlayerController")
	FString SaveTextFile(FString SaveText, FString FileName);
	
};
