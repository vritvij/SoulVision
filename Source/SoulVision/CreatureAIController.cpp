// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "CreatureAIController.h"

ACreatureAIController::ACreatureAIController()
{
	// Create sight config
	UAISenseConfig_Sight* SightConfig;
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	SightConfig->SightRadius = 500.f;
	SightConfig->LoseSightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 100.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;	


	// Setup perception component
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component")));
	GetAIPerceptionComponent()->ConfigureSense(*SightConfig);
	GetAIPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());

	GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ACreatureAIController::UpdateSenses);
}


void ACreatureAIController::Possess(APawn* NewPawn)
{
	Super::Possess(NewPawn);

	UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(StaticLoadObject(UBehaviorTree::StaticClass(), NULL, *FString("BehaviorTree'/Game/AI/Creature_BT.Creature_BT'")));
	RunBehaviorTree(BehaviorTree);

	// Make possessed pawn the leader
	SetLeader(Cast<ABaseCreature>(NewPawn));
	
	bIsInElection = false;
}

void ACreatureAIController::UpdateSenses(AActor* Actor, FAIStimulus Stimulus)
{
	// UE_LOG(General, Log, TEXT("%s sensed %s"), *GetDebugName(this), *GetDebugName(Actor));

	ABaseCreature* Creature = Cast<ABaseCreature>(Actor);

	if (Creature) {
		// If we loose sight of the creature
		if (!Stimulus.WasSuccessfullySensed())
		{
			// Remove from neighbor set
			Neighbors.Remove(Creature);

			if (Leader == Creature)
			{
				ElectLeader();
			}
		}
		else
		{
			// Check if creature is the same species as you
			if (Creature->GetClass() == GetPawn()->GetClass())
			{
				// Add to the neighbor set
				Neighbors.Add(Creature);
				
				// Check if creature is a better leader
				if (Creature->GetUniqueID() < GetLeader()->GetUniqueID())
				{
					// Change leader
					if (SetLeader(Creature))
					{
						UE_LOG(General, Log, TEXT("%s's Leader Changed to %s"), *GetDebugName(GetPawn()), *GetDebugName(Creature));
					}
				}				
			}
			else
			{
				// Make them your enemy
				// TODO: Choose enemy wisely

			}
		}
	}
}

void ACreatureAIController::ElectLeader()
{
	// Find leader only if not in election
	if (!bIsInElection)
	{
		bIsInElection = true;

		ABaseCreature* TempLeader = Cast<ABaseCreature>(GetPawn());
		if (TempLeader)
		{
			for (auto &Creature : Neighbors)
			{
				// Other Creature is a better leader
				if (Creature->GetUniqueID() < GetLeader()->GetUniqueID())
				{
					TempLeader = Creature;
				}
			}

			// Change leader
			if (SetLeader(TempLeader))
			{
				UE_LOG(General, Log, TEXT("%s's Leader Changed to %s"), *GetDebugName(GetPawn()), *GetDebugName(TempLeader));
			}
		}

		bIsInElection = false;
	}
}

void ACreatureAIController::Attack()
{

}