// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_Talk_component.h"

// Sets default values
UNPC_Talk_component::UNPC_Talk_component()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void UNPC_Talk_component::BeginPlay()
{
	Super::BeginPlay();
	
}


void UNPC_Talk_component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UNPC_Talk_component::SaveVoiceFile(TArray<FString>& VoiceIDs)
{
	if (HttpUrl == "")
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpUrl is empty!"));
		return;
	}
	// ����VoiceIDs,���ɷ���url
	for (int i = 0; i < VoiceIDs.Num(); i++)
	{
		FString VoiceID = VoiceIDs[i];
		FString VoiceUrl = HttpUrl + VoiceID + ".wav";
		// �����ļ���������
		TryDownload(VoiceID, VoiceUrl);

		FString JsonUrl = HttpUrl + VoiceID + ".json";
		// �����ļ���������
		TryDownload(VoiceID, JsonUrl);
	}
	
}

void UNPC_Talk_component::ExecuteVoiceTask()
{
}

void UNPC_Talk_component::OnVoiceRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceID, FString VoiceUrl)
{
	if (bWasSuccessful)
	{
		TArray<uint8> VoiceData = Response->GetContent();
		UStreamingSoundWave* StreamingSoundWave = NewObject<UStreamingSoundWave>();
		StreamingSoundWave->AppendAudioDataFromEncoded(VoiceData, ERuntimeAudioFormat::Wav);
		// ����VoiceFileArray,�ҵ���Ӧ��VoiceFile,��StreamingSoundWave��ֵ��VoiceFile������Ҳ������½�һ��
		bool bFound = false;
		for (int i = 0; i < VoiceFilesArray.Num(); i++)
		{
			FVoiceFiles VoiceFile = VoiceFilesArray[i];
			if (VoiceID == VoiceFile.VoiceId)
			{
				VoiceFile.VoiceFile = StreamingSoundWave;
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			FVoiceFiles VoiceFile;
			VoiceFile.VoiceId = VoiceID;
			VoiceFile.VoiceFile = StreamingSoundWave;
			VoiceFilesArray.Add(VoiceFile);
		}
	}
	else
	{
		// �ȴ�һ�ᣬ��������
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2����ʱ

		// ����һ��ί�в��󶨵�TryDownload������ͬʱ��������Ĳ���
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceID, VoiceUrl);

		// ʹ�ô�����ί�������ü�ʱ��
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);

	}
}

void UNPC_Talk_component::OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceID, FString JsonUrl)
{
	if (bWasSuccessful)
	{
		// ��json��ʽ��������
		UVaRestJsonObject* VaRestJsonObject = NewObject<UVaRestJsonObject>();
		FString JsonStr = Response->GetContentAsString();
		VaRestJsonObject->DecodeJson(JsonStr);
		// ����VoiceFileArray,�ҵ���Ӧ��VoiceFileJson,��vaRestJsonObject��ֵ��VoiceFileJson������Ҳ������½�һ��
		bool bFound = false;
		for (int i = 0; i < VoiceFilesArray.Num(); i++)
		{
			FVoiceFiles VoiceFile = VoiceFilesArray[i];
			if (VoiceID == VoiceFile.VoiceId)
			{
				VoiceFile.VoiceFileJson = VaRestJsonObject;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			FVoiceFiles VoiceFile;
			VoiceFile.VoiceId = VoiceID;
			VoiceFile.VoiceFileJson = VaRestJsonObject;
			VoiceFilesArray.Add(VoiceFile);
		}
		
	}
	else
	{
		// �ȴ�һ�ᣬ��������
		FTimerHandle TimerHandle;
		float Delay = 2.0f; // 2����ʱ

		// ����һ��ί�в��󶨵�TryDownload������ͬʱ��������Ĳ���
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("TryDownload"), VoiceID, JsonUrl);

		// ʹ�ô�����ί�������ü�ʱ��
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
	
	}
}

void UNPC_Talk_component::TryDownload(const FString& VoiceID, const FString& Url)
{
	TSharedRef<IHttpRequest> HttpVoiceRequest = FHttpModule::Get().CreateRequest();
	if (Url.EndsWith(".wav")) 
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNPC_Talk_component::OnVoiceRequestReady, VoiceID, Url);
	}
	else if (Url.EndsWith(".json"))
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNPC_Talk_component::OnJsonRequestReady, VoiceID, Url);
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

