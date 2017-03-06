// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "DataGeneratorPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOULVISION_API ADataGeneratorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Data Generation")
	void StartDataGeneration();
	
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	//Configurable properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 LevelStart = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 LevelEnd = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 LevelIncrement = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 HealthStart = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 HealthEnd = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 HealthIncrement = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 DistanceStart = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 DistanceEnd = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 DistanceIncrement = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		float FleeBiasMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		int32 FleeBiasMinimumHealth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Generator Configuration")
		FString FileName = "TrainingData.csv";
};
