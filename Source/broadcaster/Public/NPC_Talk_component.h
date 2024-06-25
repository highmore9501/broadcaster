// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sound/StreamingSoundWave.h"
#include "VaRestRequestJSON.h"
#include "VaRestJsonObject.h"
#include "HttpModule.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPC_Talk_component.generated.h"

USTRUCT(BlueprintType)
struct FVoiceFiles
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VoiceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStreamingSoundWave* VoiceFile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UVaRestJsonObject * VoiceFileJson;

	FVoiceFiles() : VoiceId(""), VoiceFile(nullptr), VoiceFileJson(nullptr)
	{
	}

};

USTRUCT(BlueprintType)
struct FVoiceTasksState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VoiceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool VoiceFileReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool VoiceFileJsonReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool TaskFinished;

	FVoiceTasksState() : VoiceId(""), VoiceFileReady(false), VoiceFileJsonReady(false), TaskFinished(false)
	{
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BROADCASTER_API UNPC_Talk_component : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UNPC_Talk_component();

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<FVoiceFiles> VoiceFilesArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVoiceTasksState> VoiceTasksStateArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString HttpUrl;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void SaveVoiceFile(const TArray<FString>& VoiceIds);

	void ExecuteVoiceTask();

	void OnVoiceRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceID, FString VoiceUrl);

	void OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceID, FString JsonUrl);

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void TryDownload(const FString& VoiceID, const FString& Url);

};


