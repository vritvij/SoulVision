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


const float USoulVisionFunctionLibrary::TypeChart[][(int32)(EElementalTypes::Max)] = {
	{ 0.5f, 0.5f, 1.0f, 1.0f},
	{ 2.0f, 0.5f, 2.0f, 1.0f},
	{ 2.0f, 1.0f, 1.0f, 0.25f},
	{ 1.0f, 1.0f, 1.0f, 1.0f}
};


float USoulVisionFunctionLibrary::GetEffectiveness(
	const EElementalTypes& Attacking,
	const EElementalTypes& Defending) 
{
	return TypeChart[(int32)Attacking][(int32)Defending];
}

void USoulVisionFunctionLibrary::GetDamage(
	const FCreatureData& AttackingCreature,
	const FCreatureData& DefendingCreature,
	const FName& Move,
	float& Damage, 
	EStatusTypes& Status,
	ETargetTypes& Target)
{
	//Initialize variables
	Damage = 0.0f;
	Status = EStatusTypes::None;
	Target = ETargetTypes::Enemy;

	
	//Fetch Move Data
	FMoveData* MoveData = GetMoveData(Move);

	if (MoveData)
	{
		float ScaleFactor = 1.0f;
		//Bonus for using move with same type as creature
		for (EElementalTypes AttackerType : USoulVisionFunctionLibrary::GetCreatureType(AttackingCreature.Name))
		{
			if (MoveData->Type == AttackerType)
			{
				ScaleFactor *= 1.5;
			}
		}
		//Bonus for using move opponent is weak against
		for (EElementalTypes DefenderType : USoulVisionFunctionLibrary::GetCreatureType(DefendingCreature.Name))
		{
			ScaleFactor *= GetEffectiveness(MoveData->Type, DefenderType);
		}
		//Add some randomness
		ScaleFactor *= FMath::FRandRange(0.85, 1);

		Damage = (((2 * (float)AttackingCreature.Level + 10) / 250) * ((float)MoveData->BaseDamage * (float)AttackingCreature.Attack / (float)DefendingCreature.Defense) + 2) * ScaleFactor;
		Status = MoveData->StatusEffect;
		Target = MoveData->Target;
	}
}


void USoulVisionFunctionLibrary::GetExperienceAtLevel(const int32& Level, int32& Experience)
{
	Experience = ((6 / 5) * FMath::Pow(Level, 3)) - (15 * FMath::Pow(Level, 2)) + (100 * Level) - 140;
}

void USoulVisionFunctionLibrary::GetStatsAtLevel(
	const FName& CreatureName,
	const int32& Level,
	int32& Health,
	int32& Attack,
	int32& Defense,
	int32& Speed)
{
	//Initialize variables
	Health = 0;
	Attack = 0;
	Defense = 0;
	Speed = 0;

	
	//Fetch creature data
	FBaseCreatureData* BaseCreatureData = GetBaseCreatureData(CreatureName);

	if (BaseCreatureData)
	{
		Health = ((float)BaseCreatureData->BaseHealth * 2 * Level / 100) + Level + 10;
		Attack = ((float)BaseCreatureData->BaseAttack * 2 * Level / 100) + 5;
		Defense = ((float)BaseCreatureData->BaseDefense * 2 * Level / 100) + 5;
		Speed = ((float)BaseCreatureData->BaseSpeed * 2 * Level / 100) + 5;
	}
}

void USoulVisionFunctionLibrary::GetMoveSetAtLevel(
	const FName& CreatureName,
	const int32& CreatureLevel,
	TArray<FName>& MoveSet)
{
	//Initialize variables
	MoveSet.Empty();
	
	//Fetch Creature Data
	FBaseCreatureData* BaseCreatureData = GetBaseCreatureData(CreatureName);

	if (BaseCreatureData)
	{
		for (FLearnableMove Move : BaseCreatureData->LearnSet)
		{
			//If creature level is lower than level needed to learn move, break 
			if (CreatureLevel < Move.Level)
				break;

			MoveSet.Add(Move.MoveName);
		}
	}
}


void USoulVisionFunctionLibrary::GetPossessionRate(
	const FCreatureData& CreatureData,
	float& PossessionRate) 
{
	//Initialize variables
	PossessionRate = 0.0f;

	//Fetch creature data
	FBaseCreatureData* BaseCreatureData = GetBaseCreatureData(CreatureData.Name);

	if (BaseCreatureData)
	{
		//Bonus for infliced status damage
		float StatusBonus = GetStatusBonus(CreatureData.Status);

		PossessionRate = (((3 * CreatureData.MaxHealth) - (2 * CreatureData.CurrentHealth))
			* BaseCreatureData->BasePossessionProbability * StatusBonus) / (3 * CreatureData.MaxHealth);
	}
}

bool USoulVisionFunctionLibrary::CanPossess(
	const FCreatureData& CreatureData, 
	const int32& MaxTries,
	int32& SuccessfulTries)
{
	//Initialize variables
	SuccessfulTries = 0;

	//Get possession rate
	float PossessionRate;
	GetPossessionRate(CreatureData, PossessionRate);

	//Calculate possession chance
	float PossessionChance;
	PossessionChance = 65536 / (FMath::Pow((255 / PossessionRate), 0.25));

	for (int i = 1; i <= MaxTries; i++) {
		if (FMath::RandRange(0, 65535) < PossessionChance)
			SuccessfulTries++;
		else
			break;
	}
	return (SuccessfulTries == MaxTries);
}

int32 USoulVisionFunctionLibrary::GetExperienceGain(
	const FCreatureData& Winner, 
	const FCreatureData& Loser)
{
	//Initialize variables
	int32 ExperienceGain = 0;

	//Fetch lose creature data
	FBaseCreatureData* LoserBaseCreatureData = GetBaseCreatureData(Loser.Name);

	if (LoserBaseCreatureData)
	{
		ExperienceGain = (LoserBaseCreatureData->BaseExperienceYield * Loser.Level / 5) *
			FMath::Pow(((2 * Loser.Level + 10) / (Loser.Level + Winner.Level + 10)), 2.5) + 1;
	}

	return ExperienceGain;
}

FString USoulVisionFunctionLibrary::HashCreatureTypeArray(
	const TArray<EElementalTypes>& CreatureType)
{
	//Initialize variables
	FString Hash = "";
	
	//Hash generation
	for (EElementalTypes Type : CreatureType)
	{
		Hash += FString::FromInt((uint32)Type + 1);
	}

	return Hash;
}

FString USoulVisionFunctionLibrary::HashCreatureStatus(
	const EStatusTypes& CreatureStatus)
{
	return FString::FromInt((int32)CreatureStatus);
}

TArray<float> USoulVisionFunctionLibrary::ConvertToLocalMovesProbabilityArray(
	const TArray<float>& GlobalMovesProbabilityArray, 
	const TArray<FName>& AvailableMovesArray)
{
	//Initialize variables
	TArray<float> LocalMovesProbabilityArray;
	LocalMovesProbabilityArray.Init(0.0f, AvailableMovesArray.Num());

	//Fetch all move names
	TArray<FName> GlobalMovesArray = GetMoveNames();

	for (int i = 0; i < AvailableMovesArray.Num(); i++)
	{
		int32 index = GlobalMovesArray.Find(AvailableMovesArray[i]);

		if (index != INDEX_NONE) {
			LocalMovesProbabilityArray[i] = GlobalMovesProbabilityArray[index];
		}
	}

	return LocalMovesProbabilityArray;
}

TArray<float> USoulVisionFunctionLibrary::ConvertToGlobalMovesProbabilityArray(
	const TArray<float>& LocalMovesProbabilityArray,
	const TArray<FName>& AvailableMovesArray)
{
	//Initialize variables
	TArray<float> GlobalMovesProbabilityArray;
	
	//Fetch all move names
	TArray<FName> GlobalMovesArray = GetMoveNames();
	GlobalMovesProbabilityArray.Init(0.0f, GlobalMovesArray.Num());

	for (int i = 0; i < AvailableMovesArray.Num(); i++)
	{
		int32 index = GlobalMovesArray.Find(AvailableMovesArray[i]);

		if (index != INDEX_NONE) {
			GlobalMovesProbabilityArray[index] = LocalMovesProbabilityArray[i];
		}
	}

	return GlobalMovesProbabilityArray;
}

FString USoulVisionFunctionLibrary::FloatArrayToString(const TArray<float>& FloatArray, const FString& Separator)
{
	FString String;

	for (int i = 0; i<FloatArray.Num(); i++)
	{
		String += FString::SanitizeFloat(FloatArray[i]);
		if (i != FloatArray.Num() - 1)
			String += Separator;
	}

	return String;
}