// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPython, Log, All);


class UNREALENGINEPYTHON_API FUnrealEnginePythonModule : public IModuleInterface
{
public:

	bool PythonGILAcquire();
	void PythonGILRelease();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RunString(char *);
	void RunFile(char *);

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline FUnrealEnginePythonModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FUnrealEnginePythonModule >("UnrealEnginePython");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UnrealEnginePython");
	}

	//Add specified folder to the python systems path (e.g. from external plugins or projects allowing modular python plugin script content)
	void AddPathToSysPath(const FString& Path);

	void AddPythonDependentPlugin(const FString& PluginName);

private:
	void *ue_python_gil;
	// used by console
	void *main_dict;
	void *local_dict;
};

struct FScopePythonGIL {
	FScopePythonGIL()
	{
#if UEPY_THREADING
		UnrealEnginePythonModule = FModuleManager::LoadModuleChecked<FUnrealEnginePythonModule>("UnrealEnginePython");
		safeForRelease = UnrealEnginePythonModule.PythonGILAcquire();
#endif
	}

	~FScopePythonGIL()
	{
#if UEPY_THREADING
		if (safeForRelease) {
			UnrealEnginePythonModule.PythonGILRelease();
		}
#endif
	}

	FUnrealEnginePythonModule UnrealEnginePythonModule;
	bool safeForRelease;
};




