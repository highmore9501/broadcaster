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

	// 添加一个skelton mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	USkeletalMeshComponent *avatar;

	// 添加一个列表。列表的元素是`{"start_time":0,"end_time":0.9,"value":"A"}`这种结构
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<FMouthAnimation> mouth_animation_data;

	//添加一个方法，用于更新嘴唇动画数据
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void UpdateMouthAnimation(TArray<FMouthAnimation> new_animation);

	//添加一个方法，用于播放嘴唇动画
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMouthAnimation();

	// 添加一个UAudioComponent
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* AudioPlayer;

	// 用于播放声音的函数
	void PlaySound(USoundBase* Sound);
	
	// 一个FTimerHandle成员变量
	UPROPERTY()
	FTimerHandle AnimationTimerHandle;

	UPROPERTY()
	TArray<FName> morphTargetNames;

};
