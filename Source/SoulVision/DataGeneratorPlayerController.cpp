// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "DataGeneratorWorker.h"
#include "DataGeneratorPlayerController.h"

void ADataGeneratorPlayerController::StartDataGeneration()
{
	FDataGeneratorWorker::GenerateData(
		this, 
		LevelStart, LevelEnd, LevelIncrement,
		HealthStart, HealthEnd, HealthIncrement,
		DistanceStart, DistanceEnd, DistanceIncrement,
		FleeBiasMultiplier, FleeBiasMinimumHealth,
		FileName
	);
}

void ADataGeneratorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FDataGeneratorWorker::Shutdown();
}
