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

void UNPC_Talk_component::SaveVoiceFile(const TArray<FString>& VoiceIds)
{
	if (HttpUrl == "")
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpUrl is empty!"));
		return;
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

void UNPC_Talk_component::ExecuteVoiceTask()
{
}

void UNPC_Talk_component::OnVoiceRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString VoiceUrl)
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

void UNPC_Talk_component::OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString JsonUrl)
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

void UNPC_Talk_component::TryDownload(const FString& VoiceId, const FString& Url)
{
	TSharedRef<IHttpRequest> HttpVoiceRequest = FHttpModule::Get().CreateRequest();
	if (Url.EndsWith(".wav")) 
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNPC_Talk_component::OnVoiceRequestReady, VoiceId, Url);
		UE_LOG(LogTemp, Warning, TEXT("wav request %s bind Onready success!"),*VoiceId);
	}
	else if (Url.EndsWith(".json"))
	{
		HttpVoiceRequest->OnProcessRequestComplete().BindUObject(this, &UNPC_Talk_component::OnJsonRequestReady, VoiceId, Url);
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

