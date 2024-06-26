// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_Talk_component.h"

// Sets default values
UNpcTalkComponent::UNpcTalkComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void UNpcTalkComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UNpcTalkComponent::UpdateVoiceTaskState(FString VoiceId, FString FileType, bool bState)
{
	for (int i = 0; i < VoiceTasksStateArray.Num(); i++)
	{
		FVoiceTasksState& VoiceTaskState = VoiceTasksStateArray[i];
		if (VoiceTaskState.VoiceId == VoiceId)
		{
			if (FileType == "wav")
			{
				VoiceTaskState.bVoiceFileReady = bState;
			}
			else if (FileType == "json")
			{
				VoiceTaskState.bVoiceFileJsonReady = bState;
			}
			break;
		}
	}
}


void UNpcTalkComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UNpcTalkComponent::SaveVoiceFile(const TArray<FString>& VoiceIds)
{
	if (HttpUrl == "")
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpUrl is empty!"));
		return;
	}
	// 初始化VoiceTaskState
	for (int i = 0; i < VoiceIds.Num(); i++)
	{
		FVoiceTasksState VoiceTaskState;
		VoiceTaskState.VoiceId = VoiceIds[i];
		VoiceTaskState.bVoiceFileReady = false;
		VoiceTaskState.bVoiceFileJsonReady = false;
		VoiceTaskState.bTaskFinished = false;
		VoiceTasksStateArray.Add(VoiceTaskState);
	}
	// 遍历VoiceIDs,生成访问url
	for (int i = 0; i < VoiceIds.Num(); i++)
	{
		FString VoiceId = VoiceIds[i];
		FString VoiceUrl = HttpUrl + VoiceId + ".wav";
		// 生成文件下载任务
		TryDownload(VoiceId, VoiceUrl);

		FString JsonUrl = HttpUrl + VoiceId + ".json";
		// 生成文件下载任务
		TryDownload(VoiceId, JsonUrl);
	}
	
}

void UNpcTalkComponent::ExecuteVoiceTask()
{
	// 如果当前有声音正在播放或者任务队列为空，则不执行任何操作
	if (VoiceTasksStateArray.IsEmpty())
	{
		return;
	}

	// 获取队列中的第一个任务
	FVoiceTasksState& VoiceTaskState = VoiceTasksStateArray[0];
	if (VoiceTaskState.bVoiceFileReady && VoiceTaskState.bVoiceFileJsonReady && !VoiceTaskState.bTaskFinished)
	{
		// 标记任务为已完成
		VoiceTaskState.bTaskFinished = true;

		// 找到对应的声音文件并播放
		for (int j = 0; j < VoiceFilesArray.Num(); j++)
		{
			FVoiceFiles& VoiceFile = VoiceFilesArray[j];
			if (VoiceTaskState.VoiceId == VoiceFile.VoiceId)
			{
				// 播放音频
				USoundWave* SoundWave = VoiceFile.VoiceFile;
				UGameplayStatics::PlaySound2D(GetWorld(), SoundWave);

				//将json文件广播出去，方法还没写
				UVaRestJsonObject* VaRestJsonObject = VoiceFile.VoiceFileJson;

				if (VaRestJsonObject) {
					OnJsonReady.Broadcast(VaRestJsonObject);
				}

				// 获取音频持续时间
				float Duration = SoundWave->Duration;

				// 创建定时器，在音频播放结束后执行
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDel;
				TimerDel.BindLambda([this]()
					{
						// 移除已完成的任务
						this->VoiceTasksStateArray.RemoveAt(0);
						// 尝试执行下一个任务
						this->ExecuteVoiceTask();
					});

				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Duration, false);

				// 播放动画等后续操作
				break;
			}
		}
	}
	else
	{
		// 如果当前任务不满足播放条件，等待0.2秒然后再来重试
		FTimerHandle TimerHandle;
		float Delay = 0.2f;

		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ExecuteVoiceTask"));

		if (GetWorld() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetWorld() returned nullptr. Delayed task execution cannot be scheduled."));
			return;
		}

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
	}
}


void UNpcTalkComponent::OnVoiceRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString VoiceUrl)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		TArray<uint8> VoiceData = Response->GetContent();
		UE_LOG(LogTemp, Warning, TEXT("VoiceData size: %d"), VoiceData.Num());
		UStreamingSoundWave* StreamingSoundWave = NewObject<UStreamingSoundWave>();
		StreamingSoundWave->AppendAudioDataFromEncoded(VoiceData, ERuntimeAudioFormat::Wav);
		// 遍历VoiceFileArray,找到对应的VoiceFile,将StreamingSoundWave赋值给VoiceFile，如果找不到就新建一个
		bool bFound = false;
		for (int i = 0; i < VoiceFilesArray.Num(); i++)
		{
			FVoiceFiles& VoiceFile = VoiceFilesArray[i];
			if (VoiceId == VoiceFile.VoiceId)
			{
				VoiceFile.VoiceFile = StreamingSoundWave;
				bFound = true;
				UE_LOG(LogTemp, Warning, TEXT("VoiceID: %s wav update success."), *VoiceId);
				UpdateVoiceTaskState(VoiceId, "wav", true);
				break;
			}
		}
		if (!bFound)
		{
			FVoiceFiles VoiceFile;
			VoiceFile.VoiceId = VoiceId;
			VoiceFile.VoiceFile = StreamingSoundWave;
			VoiceFilesArray.Add(VoiceFile);
			UE_LOG(LogTemp, Warning, TEXT("New VoiceID: %s wav download success."), *VoiceId);
			UpdateVoiceTaskState(VoiceId, "wav", true);
		}	
	}
	else
	{
		// 等待一会，重新下载
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2秒延时

		// 创建一个委托并绑定到TryDownload方法，同时传递所需的参数
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceId, VoiceUrl);

		if (GetWorld() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetWorld() returned nullptr. Delayed download cannot be scheduled."));
			return;
		}

		// 使用创建的委托来设置计时器
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
		UE_LOG(LogTemp, Warning, TEXT("restart wav quest %s :"), *VoiceId);

	}
}

void UNpcTalkComponent::OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString JsonUrl)
{
	if (bWasSuccessful && Response->GetResponseCode()==200)
	{
		// 以json格式接收数据
		UVaRestJsonObject* VaRestJsonObject = NewObject<UVaRestJsonObject>();
		FString JsonStr = Response->GetContentAsString();
		UE_LOG(LogTemp, Warning, TEXT("JsonStr: %s"), *JsonStr);
		VaRestJsonObject->DecodeJson(JsonStr);
		// 遍历VoiceFileArray,找到对应的VoiceFileJson,将vaRestJsonObject赋值给VoiceFileJson，如果找不到就新建一个
		bool bFound = false;
		for (int i = 0; i < VoiceFilesArray.Num(); i++)
		{
			FVoiceFiles& VoiceFile = VoiceFilesArray[i];
			if (VoiceId == VoiceFile.VoiceId)
			{
				VoiceFile.VoiceFileJson = VaRestJsonObject;
				bFound = true;
				UE_LOG(LogTemp, Warning, TEXT("VoiceID: %s json update success."), *VoiceId);
				UpdateVoiceTaskState(VoiceId, "json", true);
				break;
			}
		}

		if (!bFound)
		{
			FVoiceFiles VoiceFile;
			VoiceFile.VoiceId = VoiceId;
			VoiceFile.VoiceFileJson = VaRestJsonObject;
			VoiceFilesArray.Add(VoiceFile);
			UE_LOG(LogTemp, Warning, TEXT("New VoiceID: %s json download success."), *VoiceId);
			UpdateVoiceTaskState(VoiceId, "json", true);
		}
		
	}
	else
	{
		// 等待一会，重新下载
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2秒延时

		// 创建一个委托并绑定到TryDownload方法，同时传递所需的参数
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceId, JsonUrl);

		if (GetWorld() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetWorld() returned nullptr. Delayed download cannot be scheduled."));
			return;
		}

		// 使用创建的委托来设置计时器
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
		UE_LOG(LogTemp, Warning, TEXT("restart json quest %s :"), *VoiceId);
	
	}
}

void UNpcTalkComponent::TryDownload(const FString& VoiceId, const FString& Url)
{
	TSharedRef<IHttpRequest> HttpVoiceRequest = FHttpModule::Get().CreateRequest();
	if (Url.EndsWith(".wav")) 
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNpcTalkComponent::OnVoiceRequestReady, VoiceId, Url);
		UE_LOG(LogTemp, Warning, TEXT("wav request %s bind Onready success!"),*VoiceId);
	}
	else if (Url.EndsWith(".json"))
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNpcTalkComponent::OnJsonRequestReady, VoiceId, Url);
		UE_LOG(LogTemp, Warning, TEXT("wav request %s bind Onready success!"), *VoiceId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Url is not valid!"));
		return;
	}
	
	
	HttpVoiceRequest->SetURL(Url);
	HttpVoiceRequest->SetVerb("GET");
	HttpVoiceRequest->ProcessRequest();
}

void UNpcTalkComponent::ResetTalk()
{
	VoiceFilesArray.Empty();
	VoiceTasksStateArray.Empty();
}



