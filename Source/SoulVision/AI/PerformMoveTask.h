// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "PerformMoveTask.generated.h"

/**
 * 
 */
UCLASS()
class SOULVISION_API UPerformMoveTask : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	

	UBehaviorTreeComponent* BTComp;
	
	UFUNCTION()
	void OnAttackComplete();
};
