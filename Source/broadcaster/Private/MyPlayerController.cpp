// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

FString AMyPlayerController::SaveTextFile(FString SaveText, FString FileName)
{
	FString SaveDirectory = FPaths::ProjectDir() + TEXT("incomingFIles/");

	if (!FPaths::DirectoryExists(SaveDirectory))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDirectory);
	}

	FString SaveFileName = SaveDirectory + FileName;

	FFileHelper::SaveStringToFile(SaveText, *SaveFileName);

	return SaveFileName;
}
