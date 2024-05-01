// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/TBHealthSet.h"
#include "Net/UnrealNetwork.h"

void UTBHealthSet::PreAttributeChange(const FGameplayAttribute& attribute, float& newValue)
{
	Super::PreAttributeChange(attribute, newValue);

	// Make sure that Health is always within 0 and the Max Value. 
	if (GetHealthAttribute() == attribute)
	{
		newValue = FMath::Clamp(newValue, 0.0f, GetMaxHealth());
	}
	// If a Max value changes, adjust current to keep Current % of Current to Max
	else if (GetMaxHealthAttribute() == attribute)
	{
		AdjustAttributeForMaxChange(GetHealth(), GetMaxHealth(), newValue, GetHealthAttribute());
	}
}

void UTBHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTBHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTBHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UTBHealthSet::PostAttributeChange(const FGameplayAttribute& attribute, float oldValue, float newValue)
{
	Super::PostAttributeChange(attribute, oldValue, newValue);
	
	if(attribute == GetHealthAttribute())
	{
		if (newValue <= 0)
		{
			OnHealthDepletedDelegate.Broadcast(GetHealth(), oldValue);
		}
	}
}

void UTBHealthSet::OnRep_Health(const FGameplayAttributeData& oldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBHealthSet, Health, oldHealth);
}

void UTBHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& oldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBHealthSet, MaxHealth, oldMaxHealth);
}
