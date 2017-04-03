
#pragma once

#include "SoulVisionStructures.h"

class SOULVISION_API FDataGeneratorWorker : public FRunnable
{
	//Singleton instance to access thread any time via static accessor, if it is active
	static FDataGeneratorWorker* Runnable;

	//Thread to run the worker on
	FRunnableThread* Thread;
	
	//Thread Safe Counter used to stop this thread
	FThreadSafeCounter StopTaskCounter;

private:
	FCreatureData Attacker, Defender;

	bool Finished = false;


	FString PATH_SEPARATOR = FGenericPlatformMisc::GetDefaultPathSeparator();
	FString SaveDirectory = FPaths::GameContentDir() + PATH_SEPARATOR + "Data" + PATH_SEPARATOR;

	//Configurable properties
	int32 LevelStart, LevelEnd, LevelIncrement;
	int32 HealthStart, HealthEnd, HealthIncrement;
	int32 DistanceStart, DistanceEnd, DistanceIncrement;
	float FleeBiasMultiplier;
	int32 FleeBiasMinimumHealth;
	FString FileName;

public:
	//Done?
	bool IsFinished() const
	{
		return Finished;
	}

	//Constructor and Destructor
	FDataGeneratorWorker(
		int32 LevelStart, int32 LevelEnd, int32 LevelIncrement,
		int32 HealthStart, int32 HealthEnd, int32 HealthIncrement,
		int32 DistanceStart, int32 DistanceEnd, int32 DistanceIncrement,
		float FleeBiasMultiplier, int32 FleeBiasMinimumHealth,
		FString FileName
	);
	virtual ~FDataGeneratorWorker();


	//FRunnable interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	
	//Makes sure this thread has stopped completely
	void EnsureCompletion();

	//Starting the thread
	static FDataGeneratorWorker* GenerateData(
		int32 LevelStart, int32 LevelEnd, int32 LevelIncrement,
		int32 HealthStart, int32 HealthEnd, int32 HealthIncrement,
		int32 DistanceStart, int32 DistanceEnd, int32 DistanceIncrement,
		float FleeBiasMultiplier, int32 FleeBiasMinimumHealth,
		FString FileName
	);

	//Stopping the thread
	static void Shutdown();

	//Checking if thread is finished
	static bool IsThreadFinished();

};
