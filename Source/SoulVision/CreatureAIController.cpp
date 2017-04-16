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
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;	


	// Setup perception component
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component")));
	GetAIPerceptionComponent()->ConfigureSense(*SightConfig);
	GetAIPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());

	GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ACreatureAIController::UpdateSenses);

	// Setup default values for the loyalty system
	bCanSubjugate = true;
	SubjugateTimerValue = 5.f;
}

void ACreatureAIController::Tick(float DeltaSeconds)
{
	if (IsLeader())
	{
		DrawDebugSphere(
			GetWorld(),
			GetControlledCreature()->GetActorLocation(),
			30.f, 10,
			FColor(0, 255, 0),
			false, -1.f, 0, 5.f
		);
	}

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
			false, -1.f, 0, 2.f
		);
	}

	for (ACreatureAIController* Neighbor : Neighbors)
	{
		Neighbor->ProcessChallenge(GetLeader(), 0);
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

	// Since subjugations are disabled for certain amount of time after Initializing Leadership...
	// ... enable subjugation on start
	AllowSubjugation();
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
				// OtherController->ProcessChallenge(GetLeader(), 0);
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

void ACreatureAIController::ProcessChallenge(ACreatureAIController * Challenger, int32 Depth)
{
	// If Creature is at max communication depth or If Challenger has the same leader or If the creature has disabled subjugation, return
	if (Depth >= MaxCommunicationDepth || Challenger->GetLeader() == GetLeader() || !bCanSubjugate)
		return;

	// If challenger's leader's power level is greater than current leader's power level, make challenger the leader
	if (Challenger->GetLeader()->GetControlledCreature()->GetPowerLevel() > GetLeader()->GetControlledCreature()->GetPowerLevel())
	{
		// The controlled creature gets subjugated by the challenger
		ProcessSubjugate(Challenger->GetLeader(), Depth);
	}
	else
	{
		// Challenger gets subjugated by the current leader
		Challenger->ProcessSubjugate(GetLeader(), Depth + 1);
	}
}

void ACreatureAIController::ProcessSubjugate(ACreatureAIController * Subjugator, int32 Depth)
{
	// If Creature is at max communication depth or If Subjugator has the same leader or If the creature has disabled subjugation, return
	if (Depth >= MaxCommunicationDepth || Subjugator->GetLeader() == GetLeader() || !bCanSubjugate)
		return;

	// Check if Subjugator's leader wants you as a follower...
	if (Subjugator->GetLeader()->Notify(ECommRequests::RequestFollowership, this) == ECommResponses::OK)
	{
		// Store whether you were a leader
		bool bWasLeader = IsLeader();

		// Store copy of current followers
		TSet<ACreatureAIController*> FormerFollowers;
		FormerFollowers.Append(Followers);

		// Make subjugator the leader ...
		InitLeadership(Subjugator->GetLeader());

		// If creature was a Leader
		if (bWasLeader)
		{
			// ... and extend the challenge to the followers
			for (ACreatureAIController* Follower : FormerFollowers)
			{
				Follower->ProcessSubjugate(Subjugator->GetLeader(), Depth + 1);
			}
			// Tell all remaining followers that you have relinquished leadership
			for (ACreatureAIController* Follower : FormerFollowers)
			{
				if (Follower->GetLeader() != GetLeader())
				{
					Follower->Notify(ECommRequests::RelinquishLeadership, this);
				}
			}
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

	// Show loyalty to new leader by blocking subjugations for a certain amount of time
	BlockSubjugation();

	// Notify new leader that we have confirmed to become a follower
	GetLeader()->Notify(ECommRequests::ConfirmFollowership, this);
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
			
			// If Originator is already a follower, don't allow
			if (Followers.Contains(Originator))
			{
				allow = false;
			}
			// If we don't have maximum number of followers, allow
			else if (Followers.Num() < MaxFollowers)
			{
				allow = true;
			}
			// If we have maximum number of followers, 
			else
			{
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
				return ECommResponses::OK;
			}
			break;
		case ECommRequests::ConfirmFollowership:
			if (Originator != this)
			{
				Followers.Add(Originator);
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

void ACreatureAIController::BlockSubjugation()
{
	bCanSubjugate = false;
	GetWorldTimerManager().SetTimer(SubjugateTimeoutHandle, this, &ACreatureAIController::AllowSubjugation ,SubjugateTimerValue, false, -1.f);
}

void ACreatureAIController::AllowSubjugation()
{
	bCanSubjugate = true;
	GetWorldTimerManager().ClearTimer(SubjugateTimeoutHandle);
}