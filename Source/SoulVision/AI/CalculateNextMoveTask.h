// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CreatureAIController.h"
#include "SoulVisionFunctionLibrary.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "CalculateNextMoveTask.generated.h"

UCLASS()
class SOULVISION_API UCalculateNextMoveTask : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UBehaviorTreeComponent* BTComp;
	ABaseCreature* Attacker;
	ABaseCreature* Defender;

	/* Tensorflow Helpers */

	FTensorFlowData GenerateTensorFlowPayload(const FCreatureData& AttackerBase, const FCreatureData& DefenderBase, const float& Distance);

	void MakeTensorFlowRequest(FTensorFlowData Data);

	void OnTensorFlowResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
