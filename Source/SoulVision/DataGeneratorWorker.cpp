// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"
#include "SoulVisionFunctionLibrary.h"
#include "DataGeneratorWorker.h"

//Start Thread Worker as NULL
FDataGeneratorWorker* FDataGeneratorWorker::Runnable = NULL;

FDataGeneratorWorker::FDataGeneratorWorker(
	ADataGeneratorPlayerController* PC,
	int32 LevelStart, int32 LevelEnd, int32 LevelIncrement,
	int32 HealthStart, int32 HealthEnd, int32 HealthIncrement,
	int32 DistanceStart, int32 DistanceEnd, int32 DistanceIncrement,
	float FleeBiasMultiplier, int32 FleeBiasMinimumHealth,
	FString FileName)
	: PlayerController(PC)
	, LevelStart(LevelStart), LevelEnd(LevelEnd), LevelIncrement(LevelIncrement)
	, HealthStart(HealthStart), HealthEnd(HealthEnd), HealthIncrement(HealthIncrement)
	, DistanceStart(DistanceStart), DistanceEnd(DistanceEnd), DistanceIncrement(DistanceIncrement)
	, FleeBiasMultiplier(FleeBiasMultiplier), FleeBiasMinimumHealth(FleeBiasMinimumHealth)
	, FileName(FileName)
	, StopTaskCounter(0)
{	
	Thread = FRunnableThread::Create(this, TEXT("FDataGeneratorWorker"), 0, TPri_BelowNormal);
}

FDataGeneratorWorker::~FDataGeneratorWorker()
{
	delete Thread;
	Thread = NULL;
}

bool FDataGeneratorWorker::Init()
{
	if (PlayerController)
	{
		PlayerController->ClientMessage("===============================");
		PlayerController->ClientMessage("Data Generator Thread Started!");
		PlayerController->ClientMessage("===============================");
	}

	return true;
}

uint32 FDataGeneratorWorker::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//Fetch Creature and Move data tables
	UDataTable* CreaturesDataTable = USoulVisionFunctionLibrary::LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Creatures_DT.Creatures_DT'"));
	UDataTable* MovesDataTable = USoulVisionFunctionLibrary::LoadObjFromPath(TEXT("DataTable'/Game/DataTables/Moves_DT.Moves_DT'"));
	
	//Delete previous file
	FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*(SaveDirectory + PATH_SEPARATOR + FileName));
	//Open save file for writing
	IFileHandle* DataFile = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*(SaveDirectory + PATH_SEPARATOR + FileName), true);

	if (CreaturesDataTable && MovesDataTable && DataFile)
	{
		const TArray<FName> CreatureNames = CreaturesDataTable->GetRowNames();

		//For all possible combination of attacking creatures
		for (FName AttackerName : CreatureNames)
		{
			//Set Attacking Creature Name
			Attacker.Name = AttackerName;

			//For all possible combination of defending creatures
			for (FName DefenderName : CreatureNames)
			{
				//Set Defending Creature Name
				Defender.Name = DefenderName;

				//For different possible levels of attacking creatures
				for (int AttackerLevel = LevelStart; AttackerLevel <= LevelEnd; AttackerLevel += LevelIncrement)
				{
					//Set Attacking Creature Level, stats and moves
					Attacker.Level = AttackerLevel;
					USoulVisionFunctionLibrary::GetStatsAtLevel(
						AttackerName, 
						AttackerLevel, 
						Attacker.MaxHealth, 
						Attacker.Attack, 
						Attacker.Defense, 
						Attacker.Speed);
					USoulVisionFunctionLibrary::GetMoveSetAtLevel(
						AttackerName,
						AttackerLevel,
						Attacker.MoveSet);

					//For different possible levels of defending creatures
					for (int DefenderLevel = LevelStart; DefenderLevel <= LevelEnd; DefenderLevel += LevelIncrement)
					{
						//Set Defending Creature Level, stats and moves
						Defender.Level = DefenderLevel;
						USoulVisionFunctionLibrary::GetStatsAtLevel(
							DefenderName,
							DefenderLevel,
							Defender.MaxHealth,
							Defender.Attack,
							Defender.Defense,
							Defender.Speed);
						USoulVisionFunctionLibrary::GetMoveSetAtLevel(
							DefenderName,
							DefenderLevel,
							Defender.MoveSet
						);

						//For different possible health percentage of attacking creature
						for (int AttackerHealth = HealthStart; AttackerHealth <= HealthEnd; AttackerHealth += HealthIncrement)
						{
							//Set Attacking creature current health
							Attacker.CurrentHealth = ((float)AttackerHealth / HealthEnd) * Attacker.MaxHealth;

							//For different possible health percentage of defending creature
							for (int DefenderHealth = HealthStart; DefenderHealth <= HealthEnd; DefenderHealth += HealthIncrement)
							{
								//Set defending creature current health
								Defender.CurrentHealth = ((float)DefenderHealth / HealthEnd) * Defender.MaxHealth;

								//For different possible status conditions of attacking creature
								for (EStatusTypes AttackerStatus = EStatusTypes::None; AttackerStatus < EStatusTypes::Max; AttackerStatus = (EStatusTypes)((uint8)AttackerStatus + 1))
								{
									//Set attacking creature status
									Attacker.Status = AttackerStatus;

									//For different possible status conditions of defending creature
									for (EStatusTypes DefenderStatus = EStatusTypes::None; DefenderStatus < EStatusTypes::Max; DefenderStatus = (EStatusTypes)((uint8)DefenderStatus + 1))
									{
										//Set defending creature status
										Defender.Status = DefenderStatus;

										//For different distances between the two creatures
										for (int Distance = DistanceStart; Distance <= DistanceEnd; Distance += DistanceIncrement)
										{
											//If told to stop exit
											if (StopTaskCounter.GetValue() != 0)
											{
												return 0;
											}

											//Initialize variables
											FName AttackerBestMove = "None";
											float AttackerBestDamage = 0.0f;
											float AttackerBestStatusBonus = 0.0f;

											FName DefenderBestMove = "None";
											float DefenderBestDamage = 0.0f;
											float DefenderBestStatusBonus = 0.0f;

											//Find attackers best move from all available moves
											for (FName Move : Attacker.MoveSet)
											{
												//Initialize Variables
												float CurrentDamage = 0.0f;
												EStatusTypes CurrentStatus = EStatusTypes::None;
												ETargetTypes CurrentTarget = ETargetTypes::Enemy;

												//Fetch the move data
												FMoveData* MoveData = MovesDataTable->FindRow<FMoveData>(Move, TEXT("Fetch Move Data"));
												if (Distance <= MoveData->Range)
												{
													USoulVisionFunctionLibrary::GetDamage(Attacker, Defender, Move, CurrentDamage, CurrentStatus, CurrentTarget);
												}

												//TODO: Remove bias towards status moves
												if (CurrentDamage >= AttackerBestDamage && USoulVisionFunctionLibrary::GetStatusBonus(CurrentStatus) > AttackerBestStatusBonus)
												{
													AttackerBestMove = Move;
													AttackerBestDamage = CurrentDamage;
													AttackerBestStatusBonus = USoulVisionFunctionLibrary::GetStatusBonus(CurrentStatus);
												}
											}

											//Find defenders best move from all available moves
											for (FName Move : Defender.MoveSet)
											{
												//Initialize Variables
												float CurrentDamage = 0.0f;
												EStatusTypes CurrentStatus = EStatusTypes::None;
												ETargetTypes CurrentTarget = ETargetTypes::Enemy;

												//Fetch the move data
												FMoveData* MoveData = MovesDataTable->FindRow<FMoveData>(Move, TEXT("Fetch Move Data"));
												if (Distance <= MoveData->Range)
												{
													USoulVisionFunctionLibrary::GetDamage(Defender, Attacker, Move, CurrentDamage, CurrentStatus, CurrentTarget);
												}

												//TODO: Remove bias towards status moves
												if (CurrentDamage >= DefenderBestDamage && USoulVisionFunctionLibrary::GetStatusBonus(CurrentStatus) > DefenderBestStatusBonus)
												{
													DefenderBestMove = Move;
													DefenderBestDamage = CurrentDamage;
													DefenderBestStatusBonus = USoulVisionFunctionLibrary::GetStatusBonus(CurrentStatus);
												}
											}

											//Set Flee Variable
											bool Flee = (FleeBiasMultiplier*DefenderBestDamage) > (Attacker.CurrentHealth + FleeBiasMinimumHealth);

											//Input Move Probability Array
											TArray<float> InputMPA;
											//1.0f because all available moves are equally likely to be executed
											InputMPA.Init(1.0f, Attacker.MoveSet.Num());

											//Output Move Probability Array
											TArray<float> OutputMPA;
											//Only BestMove should have been performed
											OutputMPA.Init(0.0f, Attacker.MoveSet.Num());
											OutputMPA[Attacker.MoveSet.Find(AttackerBestMove)] = 1.0f;

											FString CSVRow =
												FString::SanitizeFloat((AttackerLevel - DefenderLevel) / 100.0f) + ", " +
												USoulVisionFunctionLibrary::HashCreatureTypeArray(USoulVisionFunctionLibrary::GetCreatureType(AttackerName)) + ", " +
												FString::SanitizeFloat((float)Attacker.CurrentHealth / Attacker.MaxHealth) + ", " +
												USoulVisionFunctionLibrary::HashCreatureStatus(AttackerStatus) + ", " +
												USoulVisionFunctionLibrary::HashCreatureTypeArray(USoulVisionFunctionLibrary::GetCreatureType(DefenderName)) + ", " +
												FString::SanitizeFloat((float)Defender.CurrentHealth / Defender.MaxHealth) + ", " +
												USoulVisionFunctionLibrary::HashCreatureStatus(DefenderStatus) + ", " +
												FString::FromInt(Distance) + ", " +
												USoulVisionFunctionLibrary::FloatArrayToString(USoulVisionFunctionLibrary::ConvertToGlobalMovesProbabilityArray(InputMPA, Attacker.MoveSet), ", ") + ", " +
												FString::FromInt((int)Flee) + ", " +
												USoulVisionFunctionLibrary::FloatArrayToString(USoulVisionFunctionLibrary::ConvertToGlobalMovesProbabilityArray(OutputMPA, Attacker.MoveSet), ", ") + 
												LINE_TERMINATOR;

											DataFile->Write((const uint8*)TCHAR_TO_ANSI(*CSVRow), CSVRow.Len());
										}
									}
								}
							}
						}
					}
				}		
			}
		}
	}

	//Cleanup
	delete DataFile;
	DataFile = NULL;

	//Indicate that the thread has finished
	Finished = true;

	PlayerController->ClientMessage("===============================");
	PlayerController->ClientMessage("Data Generator Thread Finished!");
	PlayerController->ClientMessage("===============================");

	return 0;
}

void FDataGeneratorWorker::Stop()
{
	StopTaskCounter.Increment();
}

FDataGeneratorWorker* FDataGeneratorWorker::GenerateData(
	ADataGeneratorPlayerController* PC,
	int32 LevelStart, int32 LevelEnd, int32 LevelIncrement,
	int32 HealthStart, int32 HealthEnd, int32 HealthIncrement,
	int32 DistanceStart, int32 DistanceEnd, int32 DistanceIncrement,
	float FleeBiasMultiplier, int32 FleeBiasMinimumHealth,
	FString FileName)
{
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FDataGeneratorWorker(
			PC, 
			LevelStart, LevelEnd, LevelIncrement, 
			HealthStart, HealthEnd, HealthIncrement, 
			DistanceStart, DistanceEnd, DistanceIncrement, 
			FleeBiasMultiplier, FleeBiasMinimumHealth, 
			FileName);
	}
	return Runnable;
}

void FDataGeneratorWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FDataGeneratorWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FDataGeneratorWorker::IsThreadFinished()
{
	if (Runnable) 
		return Runnable->IsFinished();
	return true;
}
