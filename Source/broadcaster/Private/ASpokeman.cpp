// Fill out your copyright notice in the Description page of Project Settings.


#include "ASpokeman.h"

// Sets default values
AASpokeman::AASpokeman()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 初始化avatar
	avatar = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Avatar"));
	// 将avatar附加到RootComponent
	avatar->SetupAttachment(RootComponent);

	mouth_animation_data.Empty();	

	// 初始化AudioPlayer
	AudioPlayer = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioPlayer"));
	// 将AudioPlayer附加到RootComponent
	AudioPlayer->SetupAttachment(RootComponent);

	// 获取所有的形态目标名称
	morphTargetNames.Empty();

	USkeletalMesh* SkeletalMesh = avatar->GetSkeletalMeshAsset();
	if (SkeletalMesh)
	{
		const auto& MorphTargets = SkeletalMesh->GetMorphTargets();
		for (const auto& MorphTarget : MorphTargets)
		{
			FName MorphTargetName = MorphTarget->GetFName();
			// 在这里，你可以获取到形态目标的名称
			morphTargetNames.Add(MorphTargetName);
		}
	}

}

// Called when the game starts or when spawned
void AASpokeman::BeginPlay()
{
	Super::BeginPlay();
	
}

void AASpokeman::UpdateMouthAnimation(TArray<FMouthAnimation> new_animation)
{
	mouth_animation_data = new_animation;
}

void AASpokeman::PlayMouthAnimation()
{
	// 遍历mouth_animation_data，打印每个元素的值
	for (int i = 0; i < mouth_animation_data.Num(); i++)
	{
		float start_time = mouth_animation_data[i].start_time;
		FString shape_key_name = mouth_animation_data[i].value;

		// 设置一个定时器，当定时器到达start_time时，调用SetShapeKey方法
		GetWorld()->GetTimerManager().SetTimer(AnimationTimerHandle, [this, shape_key_name]()
			{
				// 遍历所有的形态目标，将它们的权重设置为0
				for (FName name : morphTargetNames)
				{
					if (name != shape_key_name)
					{
						avatar->SetMorphTarget(name, 0.0f);
					}
				}
				// 在这里设置shape_key的值为1
				avatar->SetMorphTarget(*shape_key_name, 1.0f);
			}, start_time, false);
	}
}

void AASpokeman::PlaySound(USoundBase* Sound)
{
	
}

// Called every frame
void AASpokeman::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

