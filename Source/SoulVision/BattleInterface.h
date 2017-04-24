// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BattleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UBattleInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class SOULVISION_API IBattleInterface
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool InBattle();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool StartBattle(AController* Controller, APawn* Creature);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool EndBattle(AController* Controller, APawn* Creature);
};
