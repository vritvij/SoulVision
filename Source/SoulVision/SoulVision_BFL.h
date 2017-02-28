// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SoulVision_BFL.generated.h"

/**
 * 
 */
UCLASS()
class SOULVISION_API USoulVision_BFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Soulvision_BFL")
	static bool SaveStringTextToFile(FString SaveDirectory, FString FileName, FString TextToSave, bool AllowOverWriting = false);
	
	
	
};
