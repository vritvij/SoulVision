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
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EElementalTypes"), true);
	if (!EnumPtr)
		return 1.0f;

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

	//Fetch Creature and Move data tables
	UDataTable* CreaturesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
	UDataTable* MovesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Moves_DT.Moves_DT'"));
	
	if (CreaturesDataTable && MovesDataTable)
	{
		//Fetch Move Data
		FMoveData* MoveData = MovesDataTable->FindRow<FMoveData>(Move, TEXT("Fetch Move Data"));
		//Fetch Attacking Creature Data
		FBaseCreatureData* AttackerBase = CreaturesDataTable->FindRow<FBaseCreatureData>(
			AttackingCreature.Name,
			TEXT("Fetch Attacking Creature Data")
			);
		//Fetch Defending Creature Data
		FBaseCreatureData* DefenderBase = CreaturesDataTable->FindRow<FBaseCreatureData>(
			DefendingCreature.Name,
			TEXT("Fetch Defending Creature Data")
			);

		if (MoveData && AttackerBase && DefenderBase)
		{
			float ScaleFactor = 1.0f;
			//Bonus for using move with same type as creature
			for (EElementalTypes AttackerType : AttackerBase->Type)
			{
				if (MoveData->Type == AttackerType)
				{
					ScaleFactor *= 1.5;
				}
			}
			//Bonus for using move opponent is weak against
			for (EElementalTypes DefenderType : DefenderBase->Type) 
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
	else 
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num()+1,
			5.0f, 
			FColor::Yellow, 
			TEXT("Data table couldn't be loaded")
		);
	}
}


void USoulVisionFunctionLibrary::GetExperienceAtLevel(
	const int32& Level,
	int32& Experience)
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

	//Get Creature data table
	UDataTable* CreaturesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
	if (CreaturesDataTable)
	{
		//Fetch creature data
		FBaseCreatureData* CreatureBase = CreaturesDataTable->FindRow<FBaseCreatureData>(
			CreatureName,
			TEXT("Fetch Creature Data")
			);

		if (CreatureBase)
		{
			Health = ((float)CreatureBase->BaseHealth * 2 * Level / 100) + Level +10;
			Attack = ((float)CreatureBase->BaseAttack * 2 * Level / 100) + 5;
			Defense = ((float)CreatureBase->BaseDefense * 2 * Level / 100) + 5;
			Speed = ((float)CreatureBase->BaseSpeed * 2 * Level / 100) + 5;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num() + 1,
			5.0f,
			FColor::Yellow,
			TEXT("Data table couldn't be loaded")
		);
	}
}

void USoulVisionFunctionLibrary::GetPossessionRate(
	const FCreatureData& CreatureData,
	float& PossessionRate) 
{
	//Initialize variables
	PossessionRate = 0.0f;

	//Get creature data table
	UDataTable* CreaturesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
	if (CreaturesDataTable)
	{
		//Fetch creature data
		FBaseCreatureData* CreatureBase = CreaturesDataTable->FindRow<FBaseCreatureData>(
			CreatureData.Name,
			TEXT("Fetch Creature Data")
			);

		if (CreatureBase)
		{
			//Bonus for infliced status damage
			float StatusBonus = 1.0f;
			switch (CreatureData.Status)
			{
			case EStatusTypes::Paralyzed:
			case EStatusTypes::Poisoned:
			case EStatusTypes::Burnt:
				StatusBonus = 1.5f;
				break;
			case EStatusTypes::Asleep:
			case EStatusTypes::Frozen:
				StatusBonus = 2.0f;
				break;
			default:
				break;
			}

			PossessionRate = (((3 * CreatureData.MaxHealth) - (2 * CreatureData.CurrentHealth))
				* CreatureBase->BasePossessionProbability * StatusBonus) / (3 * CreatureData.MaxHealth);

		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num() + 1,
			5.0f,
			FColor::Yellow,
			TEXT("Data table couldn't be loaded")
		);
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

	//Get creature data table
	UDataTable* CreaturesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
	if (CreaturesDataTable)
	{
		//Fetch lose creature data
		FBaseCreatureData* LoserCreatureBase = CreaturesDataTable->FindRow<FBaseCreatureData>(
			Loser.Name,
			TEXT("Fetch Loser Creature Data")
			);

		if (LoserCreatureBase)
		{
			ExperienceGain = (LoserCreatureBase->BaseExperienceYield * Loser.Level / 5) *
				FMath::Pow(((2 * Loser.Level + 10) / (Loser.Level + Winner.Level + 10)), 2.5) + 1;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num() + 1,
			5.0f,
			FColor::Yellow,
			TEXT("Data table couldn't be loaded")
		);
	}

	return ExperienceGain;
}

int32 USoulVisionFunctionLibrary::HashCreatureTypeArray(
	const TArray<EElementalTypes>& CreatureType)
{
	//Initialize variables
	int32 Hash = 0;
	
	//Hash generation
	for (EElementalTypes Type : CreatureType)
	{
		Hash = (Hash * 10) + (uint32)Type + 1;
	}

	return Hash;
}

int32 USoulVisionFunctionLibrary::HashCreatureStatus(
	const EStatusTypes& CreatureStatus)
{
	return (int32)CreatureStatus;
}

TArray<float> USoulVisionFunctionLibrary::ConvertToLocalMovesProbabilityArray(
	const TArray<float>& GlobalMovesProbabilityArray, 
	const TArray<FName>& AvailableMovesArray)
{
	//Initialize variables
	TArray<float> LocalMovesProbabilityArray;
	LocalMovesProbabilityArray.Reserve(AvailableMovesArray.Num());

	//Get creature data table
	UDataTable* MovesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Moves_DT.Moves_DT'"));
	if (MovesDataTable)
	{
		//Fetch all row names
		TArray<FName> GlobalMovesArray = MovesDataTable->GetRowNames();

		for(int i=0; i<AvailableMovesArray.Num(); i++)
		{
			int32 index = GlobalMovesArray.Find(AvailableMovesArray[i]);

			if (index != INDEX_NONE) {
				LocalMovesProbabilityArray[i] = GlobalMovesProbabilityArray[index];
			}
		}

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num() + 1,
			5.0f,
			FColor::Yellow,
			TEXT("Data table couldn't be loaded")
		);
	}

	return LocalMovesProbabilityArray;
}

TArray<float> USoulVisionFunctionLibrary::ConvertToGlobalMovesProbabilityArray(
	const TArray<float>& LocalMovesProbabilityArray,
	const TArray<FName>& AvailableMovesArray)
{
	//Initialize variables
	TArray<float> GlobalMovesProbabilityArray;

	//Get creature data table
	UDataTable* MovesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Moves_DT.Moves_DT'"));
	if (MovesDataTable)
	{
		//Fetch all row names
		TArray<FName> GlobalMovesArray = MovesDataTable->GetRowNames();
		GlobalMovesProbabilityArray.Reserve(GlobalMovesArray.Num());

		for (int i = 0; i<AvailableMovesArray.Num(); i++)
		{
			int32 index = GlobalMovesArray.Find(AvailableMovesArray[i]);

			if (index != INDEX_NONE) {
				GlobalMovesProbabilityArray[index] = LocalMovesProbabilityArray[i];
			}
		}

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			GEngine->ScreenMessages.Num() + 1,
			5.0f,
			FColor::Yellow,
			TEXT("Data table couldn't be loaded")
		);
	}

	return GlobalMovesProbabilityArray;
}

TArray<FString> USoulVisionFunctionLibrary::FloatArrayToStringArray(const TArray<float>& FloatArray)
{
	TArray<FString> StringArray;
	StringArray.Reserve(FloatArray.Num());

	for (int i = 0; i<FloatArray.Num(); i++)
	{
		StringArray[i] = FString::SanitizeFloat(FloatArray[i]);
	}

	return StringArray;
}