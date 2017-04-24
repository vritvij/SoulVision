// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "CalculateNextMoveTask.h"

EBTNodeResult::Type UCalculateNextMoveTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BTComp = Cast<UBehaviorTreeComponent>(&OwnerComp);
	Attacker = Cast<ABaseCreature>(OwnerComp.GetAIOwner()->GetPawn());
	Defender = Cast<ABaseCreature>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("Enemy")));

	if (Attacker && Defender)
	{
		const float Distance = FVector::Distance(Attacker->GetActorLocation(), Defender->GetActorLocation());

		MakeTensorFlowRequest(GenerateTensorFlowPayload(Attacker->Base, Defender->Base, Distance));
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

FTensorFlowData UCalculateNextMoveTask::GenerateTensorFlowPayload(const FCreatureData& AttackerBase, const FCreatureData& DefenderBase, const float& Distance)
{
	TArray<float> InputMPA;
	InputMPA.Init(1.f, AttackerBase.MoveSet.Num());

	FTensorFlowData Data;

	Data.DeltaLevel = (AttackerBase.Level - DefenderBase.Level) / 100.f;
	Data.AttackerType = FCString::Atof(*USoulVisionFunctionLibrary::HashCreatureTypeArray(USoulVisionFunctionLibrary::GetCreatureType(AttackerBase.Name)));
	Data.AttackerHealth = AttackerBase.CurrentHealth / AttackerBase.MaxHealth;
	Data.AttackerStatus = FCString::Atof(*USoulVisionFunctionLibrary::HashCreatureStatus(AttackerBase.Status));
	Data.DefenderType = FCString::Atof(*USoulVisionFunctionLibrary::HashCreatureTypeArray(USoulVisionFunctionLibrary::GetCreatureType(DefenderBase.Name)));
	Data.DefenderHealth = DefenderBase.CurrentHealth / DefenderBase.MaxHealth;
	Data.DefenderStatus = FCString::Atof(*USoulVisionFunctionLibrary::HashCreatureStatus(DefenderBase.Status));
	Data.Distance = Distance;
	Data.MoveSet = USoulVisionFunctionLibrary::ConvertToGlobalMovesProbabilityArray(InputMPA, AttackerBase.MoveSet);
	Data.FleeProbability = 0;
	Data.MoveProbability = { 0 };

	return Data;
}

void UCalculateNextMoveTask::MakeTensorFlowRequest(FTensorFlowData Data)
{
	// Create Data String
	FString DataString;
	FJsonObjectConverter::UStructToJsonObjectString(FTensorFlowData::StaticStruct(), &Data, DataString, 0, 0);

	FHttpModule* Http = &FHttpModule::Get();

	// Make Json request
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UCalculateNextMoveTask::OnTensorFlowResponse);
	Request->SetURL("localhost:8081");
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetContentAsString(DataString);
	Request->ProcessRequest();
}

void UCalculateNextMoveTask::OnTensorFlowResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	EBTNodeResult::Type NodeResult(bWasSuccessful ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);

	if (bWasSuccessful)
	{
		// Convert Response content to TensorFlow Data Structure
		FTensorFlowData Data;
		FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &Data, 0, 0);

		// Mask Move Probability array
		TArray<FName> AvailableMoves = Attacker->GetAvailableMoves();
		TArray<float> ProbableMoves = USoulVisionFunctionLibrary::ConvertToLocalMovesProbabilityArray(Data.MoveProbability, AvailableMoves);

		int BestMoveIndex = -1;
		for (int i = 0; i < AvailableMoves.Num(); i++)
		{
			if (BestMoveIndex == -1 || ProbableMoves[i] > ProbableMoves[BestMoveIndex])
			{
				BestMoveIndex = i;
			}
		}

		// If BestMoveIndex is -1 it means no moves are available in that case we may either wait for sometime or run
		// Currently we run, else we perform the move
		if (BestMoveIndex == -1)
		{
			BTComp->GetBlackboardComponent()->SetValueAsName(FName("MoveToPerform"), FName("Run"));
		}
		else
		{
			BTComp->GetBlackboardComponent()->SetValueAsName(FName("MoveToPerform"), AvailableMoves[BestMoveIndex]);
		}

		UE_LOG(General, Log, TEXT("Move Selected: %s"), *BTComp->GetBlackboardComponent()->GetValueAsName(FName("MoveToPerform")).ToString());

	}
	else
	{
		UE_LOG(General, Warning, TEXT("Request was unsuccessful"));
	}

	// Finish Execute
	FinishLatentTask(*BTComp, NodeResult);
}
