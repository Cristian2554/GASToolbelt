// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** Enum listing all possible ability activation input trigger event. */
UENUM(BlueprintType)
enum class ETBAbilityTriggerEvent : uint8
{
	/** The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button. */
	Started UMETA(DisplayName="Activate on Action Started (recommended)"),

	/**
	 * Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: This value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with ability activation (since you'll be
	 * trying to activate abilities every frame). When in doubt, use the default Started value.
	 */
	Triggered UMETA(DisplayName="Activate on Action Triggered (use with caution)"),
};