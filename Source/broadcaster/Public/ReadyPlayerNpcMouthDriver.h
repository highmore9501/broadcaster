// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "VaRestJsonObject.h"


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyPlayerNpcMouthDriver.generated.h"

USTRUCT(BlueprintType)
struct FShapeKeyMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RhubarbShapeKeyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VisemeName;

};

USTRUCT(BlueprintType)
struct FMotionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VisemeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveTime;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BROADCASTER_API UReadyPlayerNpcMouthDriver : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReadyPlayerNpcMouthDriver();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShapeKeyMap> ShapeKeyMaps;

	UFUNCTION(BlueprintCallable)
	void SetShapeKeyMap(TArray<FShapeKeyMap>& In);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMotionData> MotionDatas;

	UFUNCTION(BlueprintCallable)
	bool PareseJsonToMotionData(TArray<UVaRestJsonObject*>& JsonObject);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* NpcSkeletalMeshComponent;

	void ApplyShapeKeyTransition(FName StartViseme, FName EndViseme, float Fraction);

	UFUNCTION(BlueprintCallable)
	void ResetAnimationState();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	int32 CurrentKeyframe = 0;
	float TransitionTime = 0.1f;
	float CurrentTransitionTime = 0.0f;
	bool bIsTransitioning = false;
	FName CurrentVisemeName;
	FName NextVisemeName;
	float LastKeyframeTime = 0.0f;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
