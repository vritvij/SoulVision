// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "SoulVision_BFL.h"

bool USoulVision_BFL::SaveStringTextToFile(
	FString SaveDirectory,
	FString FileName,
	FString TextToSave,
	bool AllowOverWriting
) {

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SaveDirectory))
	{
		PlatformFile.CreateDirectory(*SaveDirectory);

		if (!PlatformFile.DirectoryExists(*SaveDirectory))
		{
			return false;
		}
	}

	//get complete file path
	SaveDirectory += "/";
	SaveDirectory += FileName;

	//No over writing?
	if (AllowOverWriting || !PlatformFile.FileExists(*SaveDirectory))
	{
		return FFileHelper::SaveStringToFile(TextToSave, *SaveDirectory);
	}

	return false;
}

