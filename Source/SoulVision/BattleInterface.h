// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Controller.h"
#include "BaseCreature.h"
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
	
public:
	// Battle semaphore
	bool bInBattle = false;

	// Enemy
	AController* EnemyController = NULL;
	ABaseCreature* EnemyCreature = NULL;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool InBattle();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool StartBattle(AController* Controller, ABaseCreature* Creature);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	bool EndBattle();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	void Death();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Battle Interface")
	void Possessed();
};
