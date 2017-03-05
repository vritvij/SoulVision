// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SoulVisionStructures.h"
#include "SoulVisionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SOULVISION_API USoulVisionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static bool SaveStringTextToFile(FString SaveDirectory, FString FileName, FString TextToSave, bool AllowOverWriting = false);
	
	UFUNCTION(BlueprintCallable, Category = "SoulVisionFunctionLibrary")
	static void GetDamage(const FCreatureData& AttackingCreature, const FCreatureData& DefendingCreature, const FName& Move, float& Damage, EStatusTypes& Status, ETargetTypes& Target);
	
};
