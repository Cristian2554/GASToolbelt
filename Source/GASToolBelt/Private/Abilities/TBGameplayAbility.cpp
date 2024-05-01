// Fill out your copyright notice in the Description page of Project Settings.


#include "TBGameplayAbility.h"
#include "AbilitySystemComponent.h"

void UTBGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilitySpec& spec)
{
	Super::OnAvatarSet(actorInfo, spec);

	if (bActivateOnGranted)
	{
		actorInfo->AbilitySystemComponent->TryActivateAbility(spec.Handle, false);
	}
}
