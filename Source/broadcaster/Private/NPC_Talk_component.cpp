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
	// ��ʼ��VoiceTaskState
	for (int i = 0; i < VoiceIds.Num(); i++)
	{
		FVoiceTasksState VoiceTaskState;
		VoiceTaskState.VoiceId = VoiceIds[i];
		VoiceTaskState.bVoiceFileReady = false;
		VoiceTaskState.bVoiceFileJsonReady = false;
		VoiceTaskState.bTaskFinished = false;
		VoiceTasksStateArray.Add(VoiceTaskState);
	}
	// ����VoiceIDs,���ɷ���url
	for (int i = 0; i < VoiceIds.Num(); i++)
	{
		FString VoiceId = VoiceIds[i];
		FString VoiceUrl = HttpUrl + VoiceId + ".wav";
		// �����ļ���������
		TryDownload(VoiceId, VoiceUrl);

		FString JsonUrl = HttpUrl + VoiceId + ".json";
		// �����ļ���������
		TryDownload(VoiceId, JsonUrl);
	}
	
}

void UNpcTalkComponent::ExecuteVoiceTask()
{
	// �����ǰ���������ڲ��Ż����������Ϊ�գ���ִ���κβ���
	if (VoiceTasksStateArray.IsEmpty())
	{
		return;
	}

	// ��ȡ�����еĵ�һ������
	FVoiceTasksState& VoiceTaskState = VoiceTasksStateArray[0];
	if (VoiceTaskState.bVoiceFileReady && VoiceTaskState.bVoiceFileJsonReady && !VoiceTaskState.bTaskFinished)
	{
		// �������Ϊ�����
		VoiceTaskState.bTaskFinished = true;

		// �ҵ���Ӧ�������ļ�������
		for (int j = 0; j < VoiceFilesArray.Num(); j++)
		{
			FVoiceFiles& VoiceFile = VoiceFilesArray[j];
			if (VoiceTaskState.VoiceId == VoiceFile.VoiceId)
			{
				// ������Ƶ
				USoundWave* SoundWave = VoiceFile.VoiceFile;
				UGameplayStatics::PlaySound2D(GetWorld(), SoundWave);

				//��json�ļ��㲥��ȥ��������ûд
				UVaRestJsonObject* VaRestJsonObject = VoiceFile.VoiceFileJson;

				if (VaRestJsonObject) {
					OnJsonReady.Broadcast(VaRestJsonObject);
				}

				// ��ȡ��Ƶ����ʱ��
				float Duration = SoundWave->Duration;

				// ������ʱ��������Ƶ���Ž�����ִ��
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDel;
				TimerDel.BindLambda([this]()
					{
						// �Ƴ�����ɵ�����
						this->VoiceTasksStateArray.RemoveAt(0);
						// ����ִ����һ������
						this->ExecuteVoiceTask();
					});

				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Duration, false);

				// ���Ŷ����Ⱥ�������
				break;
			}
		}
	}
	else
	{
		// �����ǰ�������㲥���������ȴ�0.2��Ȼ����������
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
		// ����VoiceFileArray,�ҵ���Ӧ��VoiceFile,��StreamingSoundWave��ֵ��VoiceFile������Ҳ������½�һ��
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
		// �ȴ�һ�ᣬ��������
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2����ʱ

		// ����һ��ί�в��󶨵�TryDownload������ͬʱ��������Ĳ���
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceId, VoiceUrl);

		if (GetWorld() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetWorld() returned nullptr. Delayed download cannot be scheduled."));
			return;
		}

		// ʹ�ô�����ί�������ü�ʱ��
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
		UE_LOG(LogTemp, Warning, TEXT("restart wav quest %s :"), *VoiceId);

	}
}

void UNpcTalkComponent::OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString JsonUrl)
{
	if (bWasSuccessful && Response->GetResponseCode()==200)
	{
		// ��json��ʽ��������
		UVaRestJsonObject* VaRestJsonObject = NewObject<UVaRestJsonObject>();
		FString JsonStr = Response->GetContentAsString();
		UE_LOG(LogTemp, Warning, TEXT("JsonStr: %s"), *JsonStr);
		VaRestJsonObject->DecodeJson(JsonStr);
		// ����VoiceFileArray,�ҵ���Ӧ��VoiceFileJson,��vaRestJsonObject��ֵ��VoiceFileJson������Ҳ������½�һ��
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
		// �ȴ�һ�ᣬ��������
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2����ʱ

		// ����һ��ί�в��󶨵�TryDownload������ͬʱ��������Ĳ���
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceId, JsonUrl);

		if (GetWorld() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetWorld() returned nullptr. Delayed download cannot be scheduled."));
			return;
		}

		// ʹ�ô�����ί�������ü�ʱ��
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



