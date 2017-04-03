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

ABaseCreature* ACreatureAIController::GetLeader()
{
	return Cast<ABaseCreature>(Blackboard->GetValueAsObject(FName("Leader")));
}

void ACreatureAIController::SetLeader(ABaseCreature* NewLeader)
{
	Blackboard->SetValueAsObject(FName("Leader"), NewLeader);
}

bool ACreatureAIController::IsLeader()
{
	return ControlledCreature == GetLeader();
}

void ACreatureAIController::Possess(APawn* NewPawn)
{
	Super::Possess(NewPawn);

	UBehaviorTree* BehaviorTree = Cast<UBehaviorTree>(StaticLoadObject(UBehaviorTree::StaticClass(), NULL, *FString("BehaviorTree'/Game/AI/Creature_BT.Creature_BT'")));
	RunBehaviorTree(BehaviorTree);

	// Set controlled pawn
	ControlledCreature = Cast<ABaseCreature>(NewPawn);
}

void ACreatureAIController::UpdateSenses(AActor* Actor, FAIStimulus Stimulus)
{
	ABaseCreature* OtherCreature = Cast<ABaseCreature>(Actor);

	if (OtherCreature) {
		// If we see new creature and other creature is not of the same species.... 
		if (Stimulus.WasSuccessfullySensed() && !isSameSpecies(OtherCreature))
		{
			// TODO: Attack

		}
	}
}

void ACreatureAIController::Attack()
{

}

void ACreatureAIController::NotifyLeader(ELeaderEvent event)
{
	ACreatureAIController* LeaderController = Cast<ACreatureAIController>(GetLeader()->GetController());
	if (LeaderController)
	{
		LeaderController->ProcessNotify(event, this);
	}
}

void ACreatureAIController::ProcessNotify(ELeaderEvent event, ACreatureAIController* Originator)
{
	// Prevent Leader from Notifying himself :P
	if (Originator != this)
	{
		switch (event)
		{
		case ACreatureAIController::ELeaderEvent::HailLeader:
			Followers.Add(Originator);
			break;
		case ACreatureAIController::ELeaderEvent::ForgetLeader:
			Followers.Remove(Originator);
			break;
		}
	}
}