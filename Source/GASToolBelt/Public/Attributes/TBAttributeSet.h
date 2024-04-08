// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "TBAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class GASTOOLBELT_API UTBAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	// Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
	// (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
	void AdjustAttributeForMaxChange(const FGameplayAttributeData& affectedAttribute,
		const FGameplayAttributeData& maxAttribute,
		float newMaxValue,
		const FGameplayAttribute& affectedAttributeProperty) const;
};