// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedStruct.h"
#include "SoulVisionStructures.generated.h"

UENUM(BlueprintType)
enum class EElementalTypes : uint8
{
	Fire		UMETA(DisplayName = "Fire"),
	Water		UMETA(DisplayName = "Water"),
	Earth		UMETA(DisplayName = "Earth"),
	Air			UMETA(DisplayName = "Air"),

	Max			UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EExecutionTypes : uint8
{
	Melee		UMETA(DisplayName = "Melee"),
	Ranged		UMETA(DisplayName = "Ranged"),

	Max			UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EStatusTypes : uint8
{
	None		UMETA(DisplayName = "None"),
	Asleep		UMETA(DisplayName = "Asleep"),
	Paralyzed	UMETA(DisplayName = "Paralyzed"),
	Frozen		UMETA(DisplayName = "Frozen"),
	Poisoned	UMETA(DisplayName = "Poisoned"),
	Burnt		UMETA(DisplayName = "Burnt"),

	Max			UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETargetTypes : uint8
{
	Self		UMETA(DisplayName = "Self"),
	Enemy		UMETA(DisplayName = "Enemy"),

	Max			UMETA(Hidden)
};


USTRUCT(BlueprintType, meta = (ToolTip = "Data structure for the creature instance"))
struct FLearnableMove
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learn set data structure")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learn set data structure")
	FName MoveName;

	FLearnableMove()
	{
		Level = 0;
		MoveName = FName("None");
	}
};

USTRUCT(BlueprintType, meta = (ToolTip = "Data structure for storing base values for the creature"))
struct FBaseCreatureData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	TArray<EElementalTypes> Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BaseHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BaseAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BaseDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BaseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BaseExperienceYield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	int32 BasePossessionProbability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Creature Data")
	TArray<FLearnableMove> LearnSet;
	
	FBaseCreatureData()
	{
		
		BaseHealth = 0;
		BaseAttack = 0;
		BaseDefense = 0;
		BaseSpeed = 0;
		BaseExperienceYield = 0;
		BasePossessionProbability = 0;

	}
};

USTRUCT(BlueprintType, meta = (ToolTip = "Data structure for storing information for moves performed by a creature"))
struct FMoveData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	EElementalTypes Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	EExecutionTypes Execution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	int32 BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	int32 Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	int32 Accuracy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	ETargetTypes Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	EStatusTypes StatusEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Data")
	float statusDuration;

	FMoveData()
	{
		Target = ETargetTypes::Enemy;
	}

};

USTRUCT(BlueprintType, meta = (ToolTip = "Data structure for the creature instance"))
struct FCreatureData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	EStatusTypes Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 Experience;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	int32 Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Data")
	TArray<FName> MoveSet;

	FCreatureData()
	{
		Level = 5;
	}
};

USTRUCT(BlueprintType, meta = (ToolTip = "Data structure for tensorflow communication"))
struct FTensorFlowData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float DeltaHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float AttackerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float AttackerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float AttackerStatus;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float DefenderType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float DefenderHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float DefenderStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float Distance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	TArray<float> MoveSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	float FleeProbability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Data")
	TArray<float> MoveProbability;
};