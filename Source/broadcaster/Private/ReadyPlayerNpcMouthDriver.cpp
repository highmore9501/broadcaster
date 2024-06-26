// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyPlayerNpcMouthDriver.h"

// Sets default values for this component's properties
UReadyPlayerNpcMouthDriver::UReadyPlayerNpcMouthDriver()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	NpcSkeletalMeshComponent = nullptr;

	// ...
}

void UReadyPlayerNpcMouthDriver::SetShapeKeyMap(TArray<FShapeKeyMap>& In)
{
	ShapeKeyMaps = In;
}

bool UReadyPlayerNpcMouthDriver::PareseJsonToMotionData(TArray<UVaRestJsonObject*>& JsonObject)
{
	for (auto& Json : JsonObject)
	{
		float Start = Json->GetNumberField("start");
		FString RhubarbShapeKey = Json->GetStringField("value");
		if (RhubarbShapeKey.IsEmpty())
		{
			return false;
		}
		FName VisemeName = "";
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			if (ShapeKeyMap.RhubarbShapeKeyName == RhubarbShapeKey)
			{
				VisemeName = ShapeKeyMap.VisemeName;
				break;
			}
		}

		MotionDatas.Add(FMotionData{ VisemeName, Start });
	}

	UE_LOG(LogTemp, Warning, TEXT("Parsed %d motion data"), MotionDatas.Num());

	return true;
}

void UReadyPlayerNpcMouthDriver::ResetAnimationState()
{
	CurrentKeyframe = 0;
	bIsTransitioning = false;
	CurrentTransitionTime = 0.0f;
	LastKeyframeTime = 0.0f;
}

void UReadyPlayerNpcMouthDriver::ApplyShapeKeyTransition(FName StartViseme, FName EndViseme, float Fraction)
{
	if (StartViseme.ToString() == "None" && EndViseme.ToString() == "None")
	{
		// 如果StartViseme和EndViseme都是None，确保所有形状键都设置为0
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			NpcSkeletalMeshComponent->SetMorphTarget(ShapeKeyMap.VisemeName, 0.0f);
		}
	}
	else if (StartViseme.ToString() == "None")
	{
		// 如果只有StartViseme是None，将所有形状键设置为0，然后根据Fraction设置EndViseme
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			NpcSkeletalMeshComponent->SetMorphTarget(ShapeKeyMap.VisemeName, 0.0f);
		}
		NpcSkeletalMeshComponent->SetMorphTarget(EndViseme, Fraction);
	}
	else if (EndViseme.ToString() == "None")
	{
		// 如果只有EndViseme是None，根据1-Fraction设置StartViseme，然后将所有其他形状键设置为0
		NpcSkeletalMeshComponent->SetMorphTarget(StartViseme, 1.0f - Fraction);
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			if (ShapeKeyMap.VisemeName != StartViseme)
			{
				NpcSkeletalMeshComponent->SetMorphTarget(ShapeKeyMap.VisemeName, 0.0f);
			}
		}
	}
	else
	{
		// 正常情况下的处理
		NpcSkeletalMeshComponent->SetMorphTarget(StartViseme, 1.0f - Fraction);
		NpcSkeletalMeshComponent->SetMorphTarget(EndViseme, Fraction);
	}
}





// Called when the game starts
void UReadyPlayerNpcMouthDriver::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UReadyPlayerNpcMouthDriver::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 根据MotionDatas来驱动NPC的嘴巴动画
	if (MotionDatas.Num() == 0 || !NpcSkeletalMeshComponent)
	{
		return;
	}

	if (CurrentKeyframe < MotionDatas.Num() - 1)
	{
		if (!bIsTransitioning)
		{
			CurrentVisemeName = MotionDatas[CurrentKeyframe].VisemeName;
			NextVisemeName = MotionDatas[CurrentKeyframe + 1].VisemeName;
			LastKeyframeTime = MotionDatas[CurrentKeyframe].ActiveTime;
			bIsTransitioning = true;
			CurrentTransitionTime = 0.0f;
		}

		CurrentTransitionTime += DeltaTime;
		float TransitionFraction = FMath::Clamp(CurrentTransitionTime / TransitionTime, 0.0f, 1.0f);

		// Assuming you have a method to interpolate between shape keys based on TransitionFraction
		ApplyShapeKeyTransition(CurrentVisemeName, NextVisemeName, TransitionFraction);

		if (CurrentTransitionTime >= TransitionTime)
		{
			bIsTransitioning = false;
			CurrentKeyframe++;
		}
	}
}

