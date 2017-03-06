// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "SoulVisionStructures.h"
#include "SoulVisionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SOULVISION_API USoulVisionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	static const float TypeChart[][(int32)(EElementalTypes::Max)];

public:
	static FORCEINLINE UDataTable* LoadObjFromPath(const FName& Path)
	{
		if (Path == NAME_None) return NULL;

		return Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *Path.ToString()));
	}

	static FORCEINLINE float GetStatusBonus(const EStatusTypes Status)
	{
		float StatusBonus = 1.0f;
		switch (Status)
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

		return StatusBonus;
	}

	static FORCEINLINE TArray<EElementalTypes> GetCreatureType(const FName& CreatureName)
	{
		TArray<EElementalTypes> Type;
		UDataTable* CreaturesDataTable = LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
		if (CreaturesDataTable)
		{
			Type = CreaturesDataTable->FindRow<FBaseCreatureData>(CreatureName, TEXT("Find Creature Data"))->Type;
		}
		return Type;
	}

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static bool SaveStringTextToFile(FString SaveDirectory, FString FileName, FString TextToSave, bool AllowOverWriting = false);
	
	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static float GetEffectiveness(const EElementalTypes& Attacking, const EElementalTypes& Defending);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetDamage(const FCreatureData& AttackingCreature, const FCreatureData& DefendingCreature, const FName& Move, float& Damage, EStatusTypes& Status, ETargetTypes& Target);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetExperienceAtLevel(const int32& Level, int32& Experience);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetStatsAtLevel(const FName& CreatureName, const int32& Level, int32& Health, int32& Attack, int32& Defense, int32& Speed);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetMoveSetAtLevel(const FName& CreatureName, const int32& Level, TArray<FName>& MoveSet);
	
	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetPossessionRate(const FCreatureData& CreatureData, float& PossessionRate);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static bool CanPossess(const FCreatureData& CreatureData, const int32& MaxTries, int32& SuccessfulTries);

	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static int32 GetExperienceGain(const FCreatureData& Winner, const FCreatureData& Loser);

	UFUNCTION(BlueprintCallable, Category = "TensorFlowHelpers")
	static FString HashCreatureTypeArray(const TArray<EElementalTypes>& CreatureType);

	UFUNCTION(BlueprintCallable, Category = "TensorFlowHelpers")
	static FString HashCreatureStatus(const EStatusTypes& CreatureStatus);

	UFUNCTION(BlueprintCallable, Category = "TensorFlowHelpers")
	static TArray<float> ConvertToLocalMovesProbabilityArray(const TArray<float>& GlobalMovesProbabilityArray, const TArray<FName>& AvailableMovesArray);

	UFUNCTION(BlueprintCallable, Category = "TensorFlowHelpers")
	static TArray<float> ConvertToGlobalMovesProbabilityArray(const TArray<float>& LocalMovesProbabilityArray, const TArray<FName>& AvailableMovesArray);

	UFUNCTION(BlueprintCallable, Category = "TensorFlowHelpers")
	static FString FloatArrayToString(const TArray<float>& FloatArray, const FString& Separator);
};
