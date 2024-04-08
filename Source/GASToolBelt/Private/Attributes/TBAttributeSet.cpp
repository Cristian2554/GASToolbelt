// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/TBAttributeSet.h"
#include "AbilitySystemComponent.h"

void UTBAttributeSet::AdjustAttributeForMaxChange(const FGameplayAttributeData& affectedAttribute,
                                                  const FGameplayAttributeData& maxAttribute,
                                                  const float newMaxValue,
                                                  const FGameplayAttribute& affectedAttributeProperty) const
{
	UAbilitySystemComponent* abilityComp = GetOwningAbilitySystemComponent();
	const float currentMaxValue = maxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(currentMaxValue, newMaxValue) && abilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float currentValue = affectedAttribute.GetCurrentValue();
		const float newDelta = (currentMaxValue > 0.f) ? (currentValue * newMaxValue / currentMaxValue) - currentValue : newMaxValue;

		abilityComp->ApplyModToAttributeUnsafe(affectedAttributeProperty, EGameplayModOp::Additive, newDelta);
	}
}
