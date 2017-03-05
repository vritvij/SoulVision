// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "SoulVisionFunctionLibrary.h"

bool USoulVisionFunctionLibrary::SaveStringTextToFile(
	FString SaveDirectory,
	FString FileName,
	FString TextToSave,
	bool AllowOverWriting)
{

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//Only allow relative file paths
	if (!FPaths::IsRelative(SaveDirectory)) {
		return false;
	}

	//Data storage folder
	FString DataFolderLocation = "/Data";

	//Get absolute path to save directory
	SaveDirectory = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir()+DataFolderLocation, SaveDirectory);

	//Directory exists?
	if (!PlatformFile.DirectoryExists(*SaveDirectory))
	{
		//Create directory if it doesn't exist
		PlatformFile.CreateDirectory(*SaveDirectory);

		//If directory couldn't be created, then fail
		if (!PlatformFile.DirectoryExists(*SaveDirectory))
		{
			return false;
		}
	}

	//get complete file path
	SaveDirectory = FPaths::ConvertRelativePathToFull(SaveDirectory, FileName);

	//No over writing?
	if (AllowOverWriting || !PlatformFile.FileExists(*SaveDirectory))
	{
		return FFileHelper::SaveStringToFile(TextToSave, *SaveDirectory);
	}

	return false;
}

void USoulVisionFunctionLibrary::GetDamage(
	const FCreatureData& AttackingCreature,
	const FCreatureData& DefendingCreature,
	const FName& Move,
	float& Damage, EStatusTypes& Status,
	ETargetTypes& Target)
{

}
