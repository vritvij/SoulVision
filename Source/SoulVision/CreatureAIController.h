// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "BaseCreature.h"
#include "CreatureSpawner.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "CreatureAIController.generated.h"

UENUM()
enum class ECommRequests : uint8
{
	RequestLeadership,
	RelinquishLeadership,
	RequestFollowership,
	ConfirmFollowership,
	AbandonLeader
};

UENUM()
enum class ECommResponses : uint8
{
	OK,
	N_OK
};

UCLASS()
class SOULVISION_API ACreatureAIController : public AAIController
{
	GENERATED_BODY()

private:
	// Stores a reference to spawner
	ACreatureSpawner* Spawner;

	// Controlled creature reference
	ABaseCreature* ControlledCreature;

	// Called when a new actor is perceived
	UFUNCTION()
	void UpdateSenses(AActor* Actor, FAIStimulus Stimulus);

	// Called when an attack is to be performed
	void Attack();


	/* Depth-Limited Ripple Leader Election Algorithm */

	// Store reference to Leader
	ACreatureAIController* Leader = NULL;

	// Stores a list of all neighbors
	TSet<ACreatureAIController*> Neighbors;

	// Stores a list of followers if the creature is a leader
	TSet<ACreatureAIController*> Followers;

	// Maximum number of followers a leader should have 
	int32 MaxFollowers = 5;

	// Max subjugation depth
	const int32 MaxCommunicationDepth = 5;

	// Called when a challenge has been initiated against the controlled creature
	void ProcessChallenge(ACreatureAIController* Challenger, int32 Depth);

	// Called when the creature is being subjugated
	void ProcessSubjugate(ACreatureAIController* Subjugator, int32 Depth);

	// Called when the creature finds a new leader
	void InitLeadership(ACreatureAIController* Leader);

	// Called to notify other creatures of certain events
	ECommResponses Notify(ECommRequests Request, ACreatureAIController* Originator);

	// True if creature was recently subjugated, false otherwise
	bool bCanSubjugate;

	// Timer value used to set Timer that resets the bRecentlySubjugated flag
	uint32 SubjugateTimerValue;

	// Timer Handle used to reference the Subjugation timer
	FTimerHandle SubjugateTimeoutHandle;
	
	// Called to disable subjugation
	void BlockSubjugation();

	// Called to enable subjugation
	void AllowSubjugation();


public:
	ACreatureAIController(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds);

	virtual void Possess(APawn* NewPawn) override;

	virtual void UnPossess() override;

	FORCEINLINE bool isSameSpecies(ABaseCreature* OtherCreature)
	{
		if (ControlledCreature && OtherCreature)
		{
			return  ControlledCreature->Base.Name.IsEqual(OtherCreature->Base.Name);
		}

		return false;
	}

	FORCEINLINE ACreatureAIController* GetLeader() 
	{
		return Leader;
	}

	FORCEINLINE void SetLeader(ACreatureAIController* NewLeader)
	{
		if (NewLeader == NULL || NewLeader == this)
		{
			Leader = this;
			Blackboard->SetValueAsBool(FName("IsLeader"), true);
			Blackboard->SetValueAsObject(FName("Leader"), NULL);
		}
		else
		{
			Leader = NewLeader;
			Blackboard->SetValueAsBool(FName("IsLeader"), false);
			Blackboard->SetValueAsObject(FName("Leader"), NewLeader);
		}
	}

	FORCEINLINE bool IsLeader()
	{
		return Leader == this;
	}

	FORCEINLINE ABaseCreature* GetControlledCreature()
	{
		return ControlledCreature;
	}

	FORCEINLINE void SetSpawner(ACreatureSpawner* CreatureSpawner)
	{
		Spawner = CreatureSpawner;
	}

	FORCEINLINE ACreatureSpawner* GetSpawner()
	{
		return Spawner;
	}
};
