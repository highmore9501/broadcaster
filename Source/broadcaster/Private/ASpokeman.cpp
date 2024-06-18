// Fill out your copyright notice in the Description page of Project Settings.


#include "ASpokeman.h"

// Sets default values
AASpokeman::AASpokeman()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ��ʼ��avatar
	avatar = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Avatar"));
	// ��avatar���ӵ�RootComponent
	avatar->SetupAttachment(RootComponent);

	mouth_animation_data.Empty();	

	// ��ʼ��AudioPlayer
	AudioPlayer = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioPlayer"));
	// ��AudioPlayer���ӵ�RootComponent
	AudioPlayer->SetupAttachment(RootComponent);

	// ��ȡ���е���̬Ŀ������
	morphTargetNames.Empty();

	USkeletalMesh* SkeletalMesh = avatar->GetSkeletalMeshAsset();
	if (SkeletalMesh)
	{
		const auto& MorphTargets = SkeletalMesh->GetMorphTargets();
		for (const auto& MorphTarget : MorphTargets)
		{
			FName MorphTargetName = MorphTarget->GetFName();
			// ���������Ի�ȡ����̬Ŀ�������
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
	// ����mouth_animation_data����ӡÿ��Ԫ�ص�ֵ
	for (int i = 0; i < mouth_animation_data.Num(); i++)
	{
		float start_time = mouth_animation_data[i].start_time;
		FString shape_key_name = mouth_animation_data[i].value;

		// ����һ����ʱ��������ʱ������start_timeʱ������SetShapeKey����
		GetWorld()->GetTimerManager().SetTimer(AnimationTimerHandle, [this, shape_key_name]()
			{
				// �������е���̬Ŀ�꣬�����ǵ�Ȩ������Ϊ0
				for (FName name : morphTargetNames)
				{
					if (name != shape_key_name)
					{
						avatar->SetMorphTarget(name, 0.0f);
					}
				}
				// ����������shape_key��ֵΪ1
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

