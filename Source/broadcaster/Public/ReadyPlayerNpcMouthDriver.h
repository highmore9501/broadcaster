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
	float StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMotionData> MotionDatas;

	UFUNCTION(BlueprintCallable)
	bool PareseJsonToMotionData(const TArray<UVaRestJsonObject*>& JsonObject);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* NpcSkeletalMeshComponent;

	void ApplyShapeKeyTransition(FName StartViseme, FName EndViseme, float Fraction);

	UFUNCTION(BlueprintCallable)
	void ResetAnimationState();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// 播放语音用的关键帧索引
	int32 CurrentKeyframe = 0;
	// 计算变形插值时所用的当前时间值
	float CurrentTransitionTime = 0.0f;
	// 单次变形消耗的时间
	float TransitionTime = 0.1f;
	// 是否正在执行变形插值
	bool bIsTransitioning = false;
	// 当前shapekey的名称
	FName CurrentVisemeName;
	// 下一个shapekey的名称
	FName NextVisemeName;
	// 当前发音结束的时间
	float LastKeyframeTime = 0.0f;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
