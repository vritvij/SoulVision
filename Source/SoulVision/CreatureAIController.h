// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "BaseCreature.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "CreatureAIController.generated.h"

UCLASS()
class SOULVISION_API ACreatureAIController : public AAIController
{
	GENERATED_BODY()

private:
	// Controlled creature reference
	ABaseCreature* ControlledCreature;

	// Called when a new actor is perceived
	UFUNCTION()
	void UpdateSenses(AActor* Actor, FAIStimulus Stimulus);

	// Called when an attack is to be performed
	void Attack();

	TSet<ACreatureAIController*> Followers;

public:
	ACreatureAIController(const FObjectInitializer& ObjectInitializer);

	virtual void Possess(APawn* NewPawn) override;

	FORCEINLINE bool isSameSpecies(ABaseCreature* OtherCreature)
	{
		if (ControlledCreature && OtherCreature)
		{
			return  ControlledCreature->Base.Name.IsEqual(OtherCreature->Base.Name);
		}

		return false;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Creature Leadership")
	ABaseCreature* GetLeader();

	UFUNCTION(BlueprintCallable, Category = "Creature Leadership")
	void SetLeader(ABaseCreature* NewLeader);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Creature Leadership")
	bool IsLeader();

	FORCEINLINE ABaseCreature* GetControlledCreature()
	{
		return ControlledCreature;
	}

	enum class ELeaderEvent : uint8
	{
		HailLeader,
		ForgetLeader
	};

	void NotifyLeader(ELeaderEvent event);

	void ProcessNotify(ELeaderEvent event, ACreatureAIController* Originator);
};
