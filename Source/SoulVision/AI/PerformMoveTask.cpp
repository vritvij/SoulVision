// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "CreatureAIController.h"
#include "BaseCreature.h"
#include "PerformMoveTask.h"

EBTNodeResult::Type UPerformMoveTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BTComp = Cast<UBehaviorTreeComponent>(&OwnerComp);
	ACreatureAIController* Controller = Cast<ACreatureAIController>(OwnerComp.GetAIOwner());
	ABaseCreature* Creature = Cast<ABaseCreature>(OwnerComp.GetAIOwner()->GetPawn());

	if (Controller && Creature)
	{
		FName Move = OwnerComp.GetBlackboardComponent()->GetValueAsName(FName("MoveToPerform"));
		// If its a valid move Perform the selected Move
		if (!Move.IsNone() && !Move.IsEqual(FName("Run")))
		{
			UE_LOG(General, Log, TEXT("AI Creature is performing %s"), *Move.ToString());
			Creature->AttackCompleteNotify.AddUObject(this, &UPerformMoveTask::OnAttackComplete);
			Creature->PerformAttack(Move);
			return EBTNodeResult::InProgress;
		}
		else
		{
			// Run away
			// TODO
			UE_LOG(General, Log, TEXT("Creature ran away"));
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}

void UPerformMoveTask::OnAttackComplete()
{
	FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
}