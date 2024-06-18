// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "ASpokeman.generated.h"

USTRUCT(BlueprintType)
struct FMouthAnimation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float start_time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float end_time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString value;
};

UCLASS()
class BROADCASTER_API AASpokeman : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASpokeman();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ���һ��skelton mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	USkeletalMeshComponent *avatar;

	// ���һ���б��б��Ԫ����`{"start_time":0,"end_time":0.9,"value":"A"}`���ֽṹ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<FMouthAnimation> mouth_animation_data;

	//���һ�����������ڸ����촽��������
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateMouthAnimation(TArray<FMouthAnimation> new_animation);

	//���һ�����������ڲ����촽����
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMouthAnimation();

	// ���һ��UAudioComponent
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* AudioPlayer;

	// ���ڲ��������ĺ���
	void PlaySound(USoundBase* Sound);
	
	// һ��FTimerHandle��Ա����
	UPROPERTY()
	FTimerHandle AnimationTimerHandle;

	UPROPERTY()
	TArray<FName> morphTargetNames;

};
