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

void UNPC_Talk_component::OnJsonRequestReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString VoiceId, FString JsonUrl)
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

