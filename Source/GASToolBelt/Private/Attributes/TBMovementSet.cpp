// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/TBMovementSet.h"
#include "Net/UnrealNetwork.h"

void UTBMovementSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTBMovementSet, MaxWalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTBMovementSet, JumpZVelocity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTBMovementSet, MaxAcceleration, COND_None, REPNOTIFY_Always);
}

void UTBMovementSet::OnRep_MaxWalkSpeed(const FGameplayAttributeData& oldMaxWalkSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBMovementSet, MaxWalkSpeed, oldMaxWalkSpeed);
}

void UTBMovementSet::OnRep_JumpZVelocity(const FGameplayAttributeData& oldJumpZVelocity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBMovementSet, JumpZVelocity, oldJumpZVelocity);
}

void UTBMovementSet::OnRep_MaxAcceleration(const FGameplayAttributeData& oldMaxAcceleration)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTBMovementSet, MaxAcceleration, oldMaxAcceleration);
}
