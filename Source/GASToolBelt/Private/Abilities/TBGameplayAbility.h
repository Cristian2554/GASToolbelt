// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TBGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class GASTOOLBELT_API UTBGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Ability)
	bool bActivateOnGranted;
	
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo * actorInfo, const FGameplayAbilitySpec & spec) override;
};
