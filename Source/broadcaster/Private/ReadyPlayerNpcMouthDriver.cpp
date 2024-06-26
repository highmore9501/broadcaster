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



bool UReadyPlayerNpcMouthDriver::PareseJsonToMotionData(const TArray<UVaRestJsonObject*>& JsonObject)
{
	for (auto& Json : JsonObject)
	{
		float Start = Json->GetNumberField("start");
		float End = Json->GetNumberField("end");
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

		MotionDatas.Add(FMotionData{ VisemeName, Start,End });
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
	MotionDatas.Empty();	
}

void UReadyPlayerNpcMouthDriver::ApplyShapeKeyTransition(FName StartViseme, FName EndViseme, float Fraction)
{
	if (StartViseme.ToString() == "None" && EndViseme.ToString() == "None")
	{
		// ���StartViseme��EndViseme����None��ȷ��������״��������Ϊ0
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			NpcSkeletalMeshComponent->SetMorphTarget(ShapeKeyMap.VisemeName, 0.0f);
		}
	}
	else if (StartViseme.ToString() == "None")
	{
		// ���ֻ��StartViseme��None����������״������Ϊ0��Ȼ�����Fraction����EndViseme
		for (auto& ShapeKeyMap : ShapeKeyMaps)
		{
			NpcSkeletalMeshComponent->SetMorphTarget(ShapeKeyMap.VisemeName, 0.0f);
		}
		NpcSkeletalMeshComponent->SetMorphTarget(EndViseme, Fraction);
	}
	else if (EndViseme.ToString() == "None")
	{
		// ���ֻ��EndViseme��None������1-Fraction����StartViseme��Ȼ������������״������Ϊ0
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
		// ��������µĴ���
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

	// ����MotionDatas������NPC����Ͷ���
	if (MotionDatas.Num() == 0 || !NpcSkeletalMeshComponent)
	{
		return;
	}

	if (CurrentKeyframe < MotionDatas.Num() - 1)
	{
		if (!bIsTransitioning)
		{
			// ÿ��shapekey�������һ�ν�����������������µ�ֵ
			// ��ǰshapekey������
			CurrentVisemeName = MotionDatas[CurrentKeyframe].VisemeName;
			// ��һ��shapekey������
			NextVisemeName = (CurrentKeyframe + 1 < MotionDatas.Num()) ? MotionDatas[CurrentKeyframe + 1].VisemeName : "None";
			// ����ת��һ�������ĵ�ʱ��
			LastKeyframeTime = MotionDatas[CurrentKeyframe].EndTime - MotionDatas[CurrentKeyframe].StartTime;
			// ����Ѿ���ʼת����������ת������ǰ�Ͳ����ٴν������if����
			bIsTransitioning = true;
			// ��ʼ����ǰʱ��
			CurrentTransitionTime = 0.0f;
		}

		CurrentTransitionTime += DeltaTime;
		float TransitionFraction = FMath::Clamp(CurrentTransitionTime / TransitionTime, 0.0f, 1.0f);

		// Assuming you have a method to interpolate between shape keys based on TransitionFraction
		ApplyShapeKeyTransition(CurrentVisemeName, NextVisemeName, TransitionFraction);

		if (CurrentTransitionTime >= LastKeyframeTime)
		{
			// ���������ʾ���α����Ѿ�����������һЩ��ǣ�Ȼ�������һ�α���
			bIsTransitioning = false;
			CurrentKeyframe++;
		}
	}
	else if (CurrentKeyframe >= MotionDatas.Num() - 1 && !bIsTransitioning)
	{
		ResetAnimationState();
	}
}

