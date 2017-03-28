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
	
	// Stores a reference to the leader of the group
	ABaseCreature* Leader;

	// Stores references for all neighbours in the group
	TSet<ABaseCreature*> Neighbors;

	// Called when a leader is to be elected in the neighboring group
	void ElectLeader();

	// Election Variables
	bool bIsInElection;

	UFUNCTION()
	void UpdateSenses(AActor* Actor, FAIStimulus Stimulus);

	void Attack();

public:
	ACreatureAIController();

	virtual void Possess(APawn* NewPawn) override;

	FORCEINLINE ABaseCreature* GetLeader()
	{
		return Leader;
	}

	FORCEINLINE bool SetLeader(ABaseCreature* NewLeader)
	{
		Leader = NewLeader;

		Blackboard->SetValueAsObject(FName("Leader"), NewLeader);

		return true;
	}
};
