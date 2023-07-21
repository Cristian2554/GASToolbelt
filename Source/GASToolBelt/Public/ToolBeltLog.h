// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

// Intended categories:
//	Log - This happened. What gameplay programmers may care about to debug
//	Verbose - This is why this happened. What you may turn on to debug the ability system code.
//

GASTOOLBELT_API DECLARE_LOG_CATEGORY_EXTERN(LogToolBelt, Display, All);
GASTOOLBELT_API DECLARE_LOG_CATEGORY_EXTERN(LogToolBeltUI, Display, All);

class FTBScreenLogger
{
public:

	static FColor GetOnScreenVerbosityColor(const ELogVerbosity::Type verbosity)
	{
		return
			verbosity == ELogVerbosity::Fatal || verbosity == ELogVerbosity::Error ? FColor::Red :
			verbosity == ELogVerbosity::Warning ? FColor::Yellow :
			verbosity == ELogVerbosity::Display || verbosity == ELogVerbosity::Log ? FColor::Cyan :
			verbosity == ELogVerbosity::Verbose || verbosity == ELogVerbosity::VeryVerbose ? FColor::Orange :
			FColor::Cyan;
	}

	static void AddOnScreenDebugMessage(const ELogVerbosity::Type verbosity, const FString message)
	{
		if (GEngine)
		{
			const FColor color = GetOnScreenVerbosityColor(verbosity);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, color, message);
		}
	}
};

#define TB_LOG(verbosity, format, ...) \
{ \
UE_LOG(LogToolBelt, verbosity, format, ##__VA_ARGS__); \
}

#define TB_UI_LOG(verbosity, format, ...) \
{ \
UE_LOG(LogToolBeltUI, verbosity, format, ##__VA_ARGS__); \
}

#define TB_SLOG(verbosity, format, ...) \
{ \
FGSCScreenLogger::AddOnScreenDebugMessage(ELogVerbosity::verbosity, FString::Printf(format, ##__VA_ARGS__)); \
UE_LOG(LogToolBelt, verbosity, format, ##__VA_ARGS__); \
}