// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "BaseCreature.h"
#include "CreatureAIController.h"
#include "CreatureSpawner.h"

// Sets default values
ACreatureSpawner::ACreatureSpawner()
{
	SpawnerBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnerBounds"));
	SpawnerBounds->SetBoxExtent(FVector(200, 200, 50));

	RootComponent = SpawnerBounds;
}

// Called when the game starts or when spawned
void ACreatureSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	InitSpawner();
}

void ACreatureSpawner::PostInitProperties()
{
	Super::PostInitProperties();
	ValidateSpawnerValues();
}

#if WITH_EDITOR
void ACreatureSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	ValidateSpawnerValues();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ACreatureSpawner::ValidateSpawnerValues()
{
	if (MinLevel > MaxLevel)
	{
		MaxLevel = MinLevel;
	}
}

void ACreatureSpawner::InitSpawner()
{
	/*
	Don't spawn anything if...
	- Spawner is already running
	- Creature to Spawn is not set
	- Creature to Spawn is set to the BaseCreature class
	- SpawnTries have exceeded the maximum Tries
	*/
	if (bIsRunning || CreatureToSpawn == NULL || CreatureToSpawn == ABaseCreature::StaticClass() || SpawnTries > MaxRetries)
		return;

	// Set running state
	bIsRunning = true;

	// Initialize counter for failed spawn attempts
	int32 SpawnFailed = 0;

	// Spawn more creatures
	while (SpawnedCreatures < NumberOfCreatures)
	{
		// Trace start and end location
		FVector Origin = GetActorLocation();
		FVector BoxExtent = SpawnerBounds->GetScaledBoxExtent();
		FVector MinExtent = Origin - BoxExtent;
		FVector MaxExtent = Origin + BoxExtent;
		MinExtent.Z = MaxExtent.Z;
		FVector TraceStart = FMath::RandPointInBox(FBox(MinExtent, MaxExtent));
		FVector TraceEnd = TraceStart - (GetActorUpVector() * 1000);

		FHitResult HitResult;

		//Trace shape
		FCollisionShape CollisionShape;
		CollisionShape.ShapeType = ECollisionShape::Line;

		// Trace debug lines
		const FName TraceTag("CreatureSpawnLocationTrace");
		GetWorld()->DebugDrawTraceTag = TraceTag;
		FCollisionQueryParams CollisionParams;
		CollisionParams.TraceTag = TraceTag;
		CollisionParams.AddIgnoredActor(this);

		bool bHitSomething = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::FQuat(), ECollisionChannel::ECC_Visibility, CollisionShape, CollisionParams);

		if (bHitSomething)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(HitResult.Location);
			SpawnTransform.SetRotation(FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f).Quaternion());
			ABaseCreature* SpawnedCreature = GetWorld()->SpawnActor<ABaseCreature>(CreatureToSpawn, SpawnTransform);

			if (SpawnedCreature)
			{
				ACreatureAIController* CreatureAIController = Cast<ACreatureAIController>(SpawnedCreature->GetController());
				if (CreatureAIController)
				{
					CreatureAIController->SetSpawner(this);
				}


				// Register the creature
				RegisterCreature();

				// Give creature a random level between Min and Max
				SpawnedCreature->UpdateLevel(FMath::RandRange(MinLevel, MaxLevel));
				SpawnedCreature->UpdateMoveSet();
			}
			else
			{
				SpawnFailed++;
				if (SpawnFailed >= 5)
				{
					SpawnTries++;
					GetWorldTimerManager().SetTimer(RetrySpawnTimer, this, &ACreatureSpawner::InitSpawner, 5.0f * SpawnTries, false);
					break;
				}
			}
		}
	}

	// Clear running state
	bIsRunning = false;
}