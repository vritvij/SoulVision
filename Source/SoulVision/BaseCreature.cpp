// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "BaseCreature.h"
#include "CreatureAIController.h"
#include "Perception/AIPerceptionSystem.h"
#include "BattleInterface.h"
#include "SoulVisionFunctionLibrary.h"


// Sets default values
ABaseCreature::ABaseCreature()
{
	AIControllerClass = ACreatureAIController::StaticClass();

 	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Make the AI Controller possess the creature if its spawned or placed in the world
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Set Collision preset to Ragdoll
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));



	// Calculate creature defaults
	CalculateCreatureData();

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; //Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->TargetOffset = FVector(0.f, 0.f, 0.f);
	//CameraBoom->SetRelativeLocation(FVector(-40.f, 0.f, 160.f));
	//CameraBoom->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;
}

void ABaseCreature::MoveForward_Implementation(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABaseCreature::MoveRight_Implementation(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

	}
}

void ABaseCreature::PostInitProperties()
{
	Super::PostInitProperties();
	CalculateCreatureData();
}

#if WITH_EDITOR
void ABaseCreature::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	CalculateCreatureData();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ABaseCreature::CalculateCreatureData()
{
	//Update Base Data
	Base.Name = Name;
	Base.Level = Level;
	UpdateLevel(Level);
	UpdateMoveSet();
}
		
void ABaseCreature::UpdateBase(const FCreatureData& NewBase)
{
	Name = Base.Name;
	Level = Base.Level;
	Base = NewBase;
}

FLevelChangeData ABaseCreature::UpdateLevel(const int32& NewLevel)
{
	FLevelChangeData Output;
	FCreatureData OldBase = Base;

	//Ensure internal variable Level is the same as NewLevel
	Level = NewLevel;
	Base.Level = NewLevel;
	
	//Set Experience
	USoulVisionFunctionLibrary::GetExperienceAtLevel(Level, Base.Experience);
	//Set Stats
	USoulVisionFunctionLibrary::GetStatsAtLevel(Base.Name, Level, Base.MaxHealth, Base.Attack, Base.Defense, Base.Speed);
	//Set Current Health
	Base.CurrentHealth += (Base.MaxHealth - OldBase.MaxHealth);

	//Calculate Deltas
	Output.DeltaLevel = Base.Level - OldBase.Level;
	Output.DeltaExperience = Base.Experience - OldBase.Experience;
	Output.DeltaHealth = Base.MaxHealth - OldBase.MaxHealth;
	Output.DeltaAttack = Base.Attack - OldBase.Attack;
	Output.DeltaDefense = Base.Defense - OldBase.Defense;
	Output.DeltaSpeed = Base.Speed - OldBase.Speed;

	return Output;
}

void ABaseCreature::UpdateStatus(const EStatusTypes& Status)
{
	Base.Status = Status;
}

void ABaseCreature::UpdateMoveSet()
{
	USoulVisionFunctionLibrary::GetMoveSetAtLevel(Base.Name, Level, Base.MoveSet);
}

TArray<FName> ABaseCreature::GetAvailableMoves()
{
	//TODO: Should only return moves that are not blocked by a cooldown period
	return Base.MoveSet;
}

void ABaseCreature::PerformAttack_Implementation(FName Move)
{
	FMoveData* MoveData = USoulVisionFunctionLibrary::GetMoveData(Move);
	if (MoveData)
	{
		// Check if move is valid
		if (GetAvailableMoves().Find(Move) != INDEX_NONE) 
		{
			FHitResult HitResult;

			// Trace start and end location
			FVector StartLocation = this->GetActorLocation();
			FVector EndLocation = StartLocation + (this->GetActorForwardVector() * MoveData->Range * 10.f);

			// Trace shape
			FCollisionShape CollisionShape;
			CollisionShape.ShapeType = ECollisionShape::Sphere;
			CollisionShape.SetSphere(20);

			// Trace debug lines
			const FName TraceTag("AttackTrace");
			GetWorld()->DebugDrawTraceTag = TraceTag;
			FCollisionQueryParams CollisionParams;
			CollisionParams.TraceTag = TraceTag;
			CollisionParams.AddIgnoredActor(this);

			// Execute trace
			bool bHitSomething = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::FQuat(), ECollisionChannel::ECC_Visibility, CollisionShape, CollisionParams);

			// Check if something was hit
			if (bHitSomething)
			{
				// Check if a creature is hit
				ABaseCreature* Opponent = Cast<ABaseCreature>(HitResult.GetActor());
				
				if (Opponent) {
					float Damage = 0;
					EStatusTypes Status = EStatusTypes::None;
					ETargetTypes Target = ETargetTypes::Enemy;

					USoulVisionFunctionLibrary::GetDamage(Base, Opponent->Base, Move, Damage, Status, Target);

					UGameplayStatics::ApplyDamage(Opponent, Damage, GetController(), this, UDamageType::StaticClass());
				
					UE_LOG(General, Log, TEXT("%s Successful"), *Move.ToString());

					return;
				}
			}
		}
	}

	UE_LOG(General, Log, TEXT("%s Failed"), *Move.ToString());
}

void ABaseCreature::FinishAttack()
{
	if (AttackCompleteNotify.IsBound())
	{
		AttackCompleteNotify.Broadcast();
	}
}

float ABaseCreature::TakeDamage_Implementation(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Base.CurrentHealth = FMath::Clamp(Base.CurrentHealth - Damage, 0.f, (float)Base.MaxHealth);
	if (Base.CurrentHealth == 0)
	{
		// If health decreases to 0...
		// ...Enable Ragdoll
		GetMesh()->SetSimulatePhysics(true);
		// ...and destroy the actor after 2 seconds
		FTimerHandle DeathTimer;
		GetWorldTimerManager().SetTimer(DeathTimer, this, &ABaseCreature::Death, 2.f, false);
		
		UE_LOG(General, Log, TEXT("%s fainted"), *Base.Name.ToString());
	}
	else 
	{
		UE_LOG(General, Log, TEXT("%s's health decreased by %f and is %d"), *Base.Name.ToString(), Damage, Base.CurrentHealth);
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void ABaseCreature::Death_Implementation()
{
	IBattleInterface* Controller = Cast<IBattleInterface>(GetController());
	if (Controller)
	{
		Controller->Execute_Death(GetController());
	}
	
	Destroy();
}