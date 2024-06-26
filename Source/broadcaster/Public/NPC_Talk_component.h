// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sound/StreamingSoundWave.h"
#include "VaRestRequestJSON.h"
#include "VaRestJsonObject.h"
#include "HttpModule.h"
#include "Kismet/GameplayStatics.h"

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
	bool bVoiceFileReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVoiceFileJsonReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTaskFinished;

	FVoiceTasksState() : VoiceId(""), bVoiceFileReady(false), bVoiceFileJsonReady(false), bTaskFinished(false)
	{
	}

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJsonReadyDelegate, UVaRestJsonObject*, JsonObject);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BROADCASTER_API UNpcTalkComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UNpcTalkComponent();

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<FVoiceFiles> VoiceFilesArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVoiceTasksState> VoiceTasksStateArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString HttpUrl;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnJsonReadyDelegate OnJsonReady;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bIsTalking = false;

	void UpdateVoiceTaskState(FString VoiceId, FString FileType, bool bState);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void SaveVoiceFile(const TArray<FString>& VoiceIds);

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void ExecuteVoiceTask();

	void OnVoiceRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString VoiceUrl);

	void OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString JsonUrl);

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void TryDownload(const FString& VoiceId, const FString& Url);

	UFUNCTION(BlueprintCallable, Category = "NPC_Talk")
	void ResetTalk();
	

};


