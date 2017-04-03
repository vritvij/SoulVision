// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "CreatureAIController.h"

ACreatureAIController::ACreatureAIController(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	// Enable Ticking
	PrimaryActorTick.bCanEverTick = true;

	// Create sight config
	UAISenseConfig_Sight* SightConfig;
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	SightConfig->SightRadius = 500.f;
	SightConfig->LoseSightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 120.f;
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

void ACreatureAIController::Tick(float DeltaSeconds)
{
	if (GetLeader())
	{
		DrawDebugLine(
			GetWorld(),
			GetControlledCreature()->GetActorLocation(),
			GetLeader()->GetControlledCreature()->GetActorLocation(),
			FColor(255, 0, 0),
			false, -1.f, 0, 10.f
			);
	}

	for (ACreatureAIController* n : Neighbors)
	{
		DrawDebugLine(
			GetWorld(),
			GetControlledCreature()->GetActorLocation(),
			n->GetControlledCreature()->GetActorLocation(),
			FColor(0, 0, 255),
			false, -1.f, 0, 5.f
		);
	}
}


void ACreatureAIController::Possess(APawn* NewPawn)
{
	Super::Possess(NewPawn);

	UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(StaticLoadObject(UBehaviorTree::StaticClass(), NULL, *FString("BehaviorTree'/Game/AI/Creature_BT.Creature_BT'")));
	RunBehaviorTree(BehaviorTree);

	// Set controlled creature
	ControlledCreature = Cast<ABaseCreature>(NewPawn);

	// Initialize the possessed creature as the leader
	InitLeadership(this);
}

void ACreatureAIController::UnPossess()
{
	// If possessed creature was a leader
	if (IsLeader())
	{
		// Tell all followers that you have relinquished leadership
		for (auto& Follower : Followers)
		{
			Follower->Notify(ECommRequests::RelinquishLeadership, this);
		}
	}
	else
	{
		// Tell leader that you have abandoned followership
		GetLeader()->Notify(ECommRequests::AbandonLeader, this);
	}

	// Clear variables
	Neighbors.Empty();
	Followers.Empty();

	Super::UnPossess();
}

void ACreatureAIController::UpdateSenses(AActor* Actor, FAIStimulus Stimulus)
{
	ABaseCreature* OtherCreature = Cast<ABaseCreature>(Actor);

	if (OtherCreature) {

		// If we see new creature
		if (Stimulus.WasSuccessfullySensed())
		{
			// other creature is not of the same species
			if (!isSameSpecies(OtherCreature))
			{
				// TODO: Attack
				return;
			}

			// Keep track of new neighbor
			ACreatureAIController* OtherController = Cast<ACreatureAIController>(OtherCreature->GetController());
			if (OtherController)
			{
				Neighbors.Add(OtherController);
				Challenge(OtherController);
			}
		}
		else 
		{
			// Lose track of old neighbor
			ACreatureAIController* OtherController = Cast<ACreatureAIController>(OtherCreature->GetController());
			if (OtherController)
			{
				Neighbors.Remove(OtherController);
			}
		}
	}
}

void ACreatureAIController::Attack()
{

}

void ACreatureAIController::Challenge(ACreatureAIController * AIToChallenge)
{
	// Call ProcessChallenge on the AI
	AIToChallenge->ProcessChallenge(GetLeader());
}

void ACreatureAIController::ProcessChallenge(ACreatureAIController * Challenger)
{
	// If challenger's power level is greater than current leader's power level, make challenger the leader
	if (Challenger->GetControlledCreature()->GetPowerLevel() > GetLeader()->GetControlledCreature()->GetPowerLevel())
	{
		// The controlled creature gets subjugated by the challenger
		ProcessSubjugate(Challenger);
	}
	else
	{
		// Challenger gets subjugated by the current leader
		Subjugate(Challenger);
	}
}

void ACreatureAIController::Subjugate(ACreatureAIController * AIToSubjugate)
{
	// Call ProcessSubjugate on the AI
	AIToSubjugate->ProcessSubjugate(GetLeader());
}

void ACreatureAIController::ProcessSubjugate(ACreatureAIController * Subjugator)
{
	// If Subjugator is already the leader, return
	if (Subjugator == GetLeader())
		return;

	// If Leader wants you as a follower...
	if (Subjugator->Notify(ECommRequests::RequestFollowership, this) == ECommResponses::OK)
	{
		// If it is a Leader, Relinquish leadership i.e disband the followers
		if (IsLeader())
		{
			// Tell all followers that you have relinquished leadership
			for (auto& Follower : Followers)
			{
				Follower->Notify(ECommRequests::RelinquishLeadership, this);
			}
		}
	
		// ... Make subjugator the leader ...
		InitLeadership(Subjugator);
		// ... and extend the challenge to the neighbors
		for (ACreatureAIController* Neighbor : Neighbors)
		{
			Challenge(Neighbor);
		}
	}
}

void ACreatureAIController::InitLeadership(ACreatureAIController * NewLeader)
{
	// If new Leader is the same as old leader, return
	if (NewLeader == GetLeader())
		return;

	// If we aren't a leader and a leader exists...
	if (!IsLeader() && GetLeader())
	{
		// ... notify current leader that we are changing our leader
		GetLeader()->Notify(ECommRequests::AbandonLeader, this);
	}
	else
	{
		// If we are a leader...
		// ... purge all followers
		Followers.Empty();
	}

	// Set the new leader
	SetLeader(NewLeader);

}

ECommResponses ACreatureAIController::Notify(ECommRequests Request, ACreatureAIController* Originator)
{
	// Prevent Leader from Notifying himself :P
	if (Originator != this)
	{
		bool allow = false;

		switch (Request)
		{
		case ECommRequests::RequestLeadership:
			break;
		case ECommRequests::RelinquishLeadership:
			// Check if originator is the current leader
			if (GetLeader() == Originator)
			{
				InitLeadership(this);
			}
			break;
		case ECommRequests::RequestFollowership:
			// If we don't have maximum number of followers, allow
			if (Followers.Num() < MaxFollowers)
			{
				allow = true;
			}
			else
			{
				// If we have maximum number of followers, 
				// Check new followers power level against other followers
				for (auto& Follower : Followers)
				{
					// If there exists a follower with power level less than the new follower
					if (Follower->GetControlledCreature()->GetPowerLevel() < Originator->GetControlledCreature()->GetPowerLevel())
					{
						// Remove the follower
						Follower->Notify(ECommRequests::RelinquishLeadership, this);
						allow = true;
						break;
					}
				}
			}
			if (allow)
			{
				Followers.Add(Originator);
				return ECommResponses::OK;
			}
			break;
		case ECommRequests::AbandonLeader:
			Followers.Remove(Originator);
			return ECommResponses::OK;
			break;
		default:
			break;
		}
	}

	return ECommResponses::N_OK;
}