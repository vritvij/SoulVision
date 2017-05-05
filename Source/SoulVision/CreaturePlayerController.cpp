// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "CreatureAIController.h"
#include "CreaturePlayerController.h"
#include "SoulVisionFunctionLibrary.h"

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
	InputComponent->BindAction("PreviousMove", IE_Pressed, this, &ACreaturePlayerController::PreviousMove);
	InputComponent->BindAction("NextMove", IE_Pressed, this, &ACreaturePlayerController::NextMove);

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

			if (OtherCreature)
			{
				// If we are in battle with the creature
				if (InBattle() && EnemyCreature == OtherCreature)
				{
					// Check possibility of possession
					int32 SuccessfulTries;
					if (USoulVisionFunctionLibrary::CanPossess(OtherCreature->Base, 3, SuccessfulTries))
					{
						float TweenTime = 0.75f;

						// Smoothly blend camera view targets between the creatures
						SetViewTargetWithBlend(OtherCreature, TweenTime);

						PossessionTweenDelegate.BindUFunction(this, FName("OnPossessionAnimationComplete"), OtherCreature);

						GetWorldTimerManager().SetTimer(PossessionTweenTimer, PossessionTweenDelegate, TweenTime, false);
					}
					else {
						UE_LOG(General, Log, TEXT("Possession Failed"));
					}
				}
				else if(!InBattle())
				{
					// If we are not in battle, start one with the creature

					ABaseCreature* ControlledCreature = Cast<ABaseCreature>(GetPawn());
					AController* OtherController = OtherCreature->GetController();
					
					IBattleInterface* Controller = Cast<IBattleInterface>(OtherController);
					if (Controller && ControlledCreature && (Controller->Execute_StartBattle(OtherController, this, ControlledCreature)))
					{
						StartBattle(OtherController, OtherCreature);
					}
				}
			}
		}
	}
}

void ACreaturePlayerController::OnPossessionAnimationComplete(ABaseCreature* OtherCreature)
{
	SelectedMoveIndex = 0;

	AController* OtherController = OtherCreature->GetController();
	OtherController->Possess(GetPawn());

	IBattleInterface* Controller = Cast<IBattleInterface>(OtherController);
	if (Controller)
	{
		Controller->Execute_Possessed(OtherController);
	}

	// Possess other creature
	this->Possess(OtherCreature);
}

void ACreaturePlayerController::Attack()
{
	ABaseCreature* ControlledCreature = Cast<ABaseCreature>(this->GetCharacter());

	if (ControlledCreature && !bIsAttacking) {
		// Select Attack
		TArray<FName> Moves = ControlledCreature->GetAvailableMoves();
		FName SelectedMove = Moves[SelectedMoveIndex];
		UE_LOG(General, Log, TEXT("Attacking using: %s"), *SelectedMove.ToString());

		bIsAttacking = true;

		// Perform Attack
		ControlledCreature->PerformAttack(SelectedMove);
		// ControlledCreature->AttackCompleteNotify.AddUObject(this, &ACreaturePlayerController::OnAttackComplete);
		ControlledCreature->AttackCompleteNotify.AddUFunction(this, FName("OnAttackComplete"));
	}
}

void ACreaturePlayerController::OnAttackComplete()
{
	bIsAttacking = false;
	UE_LOG(General, Warning, TEXT("Attack Complete"));
}

bool ACreaturePlayerController::InBattle_Implementation()
{
	return bInBattle;
}

bool ACreaturePlayerController::StartBattle_Implementation(AController* Controller, ABaseCreature* Creature)
{
	// Only start battle if not in battle
	if (!bInBattle && Controller && Creature)
	{
		bInBattle = true;
		EnemyController = Controller;
		EnemyCreature = Creature;
		
		return true;
	}
	else return false;
}

bool ACreaturePlayerController::EndBattle_Implementation()
{
	// Only end battle if in battle
	if (bInBattle)
	{
		bInBattle = false;
		EnemyController = NULL;
		EnemyCreature = NULL;

		return true;
	}
	else return false;
}

void ACreaturePlayerController::Death_Implementation()
{
	if (bInBattle)
	{
		IBattleInterface* Controller = Cast<IBattleInterface>(EnemyController);
		if (Controller)
		{
			Controller->Execute_EndBattle(EnemyController);
		}
	}
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit);
}

void ACreaturePlayerController::Possessed_Implementation()
{
	// Player can never be possessed
	return;
}

void ACreaturePlayerController::PreviousMove()
{
	ABaseCreature* ControlledCreature = Cast<ABaseCreature>(GetPawn());
	if (ControlledCreature)
	{
		TArray<FName> Moves = ControlledCreature->GetAvailableMoves();
		SelectedMoveIndex = (SelectedMoveIndex + Moves.Num() - 1) % Moves.Num();
		UE_LOG(General, Log, TEXT("Selected Move is : %s"), *Moves[SelectedMoveIndex].ToString());
	}
}

void ACreaturePlayerController::NextMove()
{
	ABaseCreature* ControlledCreature = Cast<ABaseCreature>(GetPawn());
	if (ControlledCreature)
	{
		TArray<FName> Moves = ControlledCreature->GetAvailableMoves();
		SelectedMoveIndex = (SelectedMoveIndex + 1) % Moves.Num();
		UE_LOG(General, Log, TEXT("Selected Move is : %s"), *Moves[SelectedMoveIndex].ToString());
	}
}

ABaseCreature* ACreaturePlayerController::GetEnemyCreature()
{
	return EnemyCreature;
}

FName ACreaturePlayerController::GetSelectedMove()
{
	ABaseCreature* ControlledCreature = Cast<ABaseCreature>(GetPawn());
	if (ControlledCreature)
	{
		return ControlledCreature->GetAvailableMoves()[SelectedMoveIndex];
	}
	else return FName("");
}