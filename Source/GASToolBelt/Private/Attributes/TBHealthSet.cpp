// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/TBHealthSet.h"
#include "Net/UnrealNetwork.h"

void UTBHealthSet::PreAttributeChange(const FGameplayAttribute& attribute, float& newValue)
{
	Super::PreAttributeChange(attribute, newValue);
	
	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (attribute == GetMaxHealthAttribute()) // GetMaxHealthAttribute comes from the Macros defined at the top of the header
		{
		AdjustAttributeForMaxChange(Health, MaxHealth, newValue, GetHealthAttribute());
		}
}

void UTBHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTBHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTBHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UTBHealthSet::OnRep_Health(const FGameplayAttributeData& oldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBHealthSet, Health, oldHealth);
}

void UTBHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& oldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBHealthSet, MaxHealth, oldMaxHealth);
}
