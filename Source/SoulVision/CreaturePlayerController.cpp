// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "CreatureAIController.h"
#include "CreaturePlayerController.h"

ACreaturePlayerController::ACreaturePlayerController()
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

void ACreaturePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Movement input bindings
	InputComponent->BindAxis("MoveForward", this, &ACreaturePlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACreaturePlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &ACreaturePlayerController::AddYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACreaturePlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ACreaturePlayerController::AddPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACreaturePlayerController::LookUpAtRate);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACreaturePlayerController::StartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACreaturePlayerController::StopJump);

	// Action input bindings
	InputComponent->BindAction("Possess", IE_Pressed, this, &ACreaturePlayerController::Possession);
	InputComponent->BindAction("Attack", IE_Pressed, this, &ACreaturePlayerController::Attack);
}

void ACreaturePlayerController::MoveForward(float Value)
{
	ABaseCreature* Creature = Cast<ABaseCreature>(this->GetPawn());
	if (Creature)
	{
		Creature->MoveForward(Value);
	}
}

void ACreaturePlayerController::MoveRight(float Value)
{
	ABaseCreature* Creature = Cast<ABaseCreature>(this->GetPawn());
	if (Creature)
	{
		Creature->MoveRight(Value);
	}
}

void ACreaturePlayerController::TurnAtRate(float Rate)
{
	AddYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACreaturePlayerController::LookUpAtRate(float Rate)
{
	AddPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACreaturePlayerController::StartJump()
{
	ABaseCreature* Creature = Cast<ABaseCreature>(this->GetPawn());
	if (Creature)
	{
		Creature->Jump();
	}
}

void ACreaturePlayerController::StopJump()
{
	ABaseCreature* Creature = Cast<ABaseCreature>(this->GetPawn());
	if (Creature)
	{
		Creature->StopJumping();
	}
}

void ACreaturePlayerController::Possession()
{

	ACharacter* ControlledCharacter = this->GetCharacter();

	if (ControlledCharacter) {
		FHitResult HitResult;

		// Trace start and end location
		FVector StartLocation = ControlledCharacter->GetActorLocation() + FVector(0, 0, ControlledCharacter->BaseEyeHeight);
		FVector EndLocation = StartLocation + (this->GetActorForwardVector() * 500.f) + FVector(0, 0, 20.f);

		// Trace shape
		FCollisionShape CollisionShape;
		CollisionShape.ShapeType = ECollisionShape::Sphere;
		CollisionShape.SetSphere(20);

		// Trace debug lines
		const FName TraceTag("PossessionTrace");
		GetWorld()->DebugDrawTraceTag = TraceTag;
		FCollisionQueryParams CollisionParams;
		CollisionParams.TraceTag = TraceTag;
		CollisionParams.AddIgnoredActor(ControlledCharacter);

		// Execute trace
		bool bHitSomething = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::FQuat(), ECollisionChannel::ECC_Visibility, CollisionShape, CollisionParams);

		// Check if something was hit
		if (bHitSomething)
		{
			// Only possess creatures
			ABaseCreature* OtherCreature = Cast<ABaseCreature>(HitResult.GetActor());

			if (OtherCreature) {
				// Destroy other creature's controller
				OtherCreature->GetController()->Possess(ControlledCharacter);

				// Possess other creature
				this->Possess(OtherCreature);
			}
		}
	}
}

void ACreaturePlayerController::Attack()
{
	ABaseCreature* ControlledCreature = Cast<ABaseCreature>(this->GetCharacter());

	if (ControlledCreature) {
		// Select Attack
		TArray<FName> Moves = ControlledCreature->GetAvailableMoves();
		FName SelectedMove = Moves[FMath::RandRange(0, Moves.Num() - 1)];
		UE_LOG(General, Log, TEXT("Attacking using: %s"), *SelectedMove.ToString());

		// Perform Attack
		ControlledCreature->PerformAttack(SelectedMove);
	}
}