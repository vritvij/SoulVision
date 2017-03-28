// Fill out your copyright notice in the Description page of Project Settings.

#include "SoulVision.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SoulVision, "SoulVision" );

//General Log
DEFINE_LOG_CATEGORY(General);

//Logging during game startup
DEFINE_LOG_CATEGORY(Init);

//Logging for AI system
DEFINE_LOG_CATEGORY(AI);

//Logging for critical errors
DEFINE_LOG_CATEGORY(Critical);