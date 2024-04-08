// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TBCharacterMovementComponent.generated.h"

class UAbilitySystemComponent;
class UTBMovementSet;

/*
 * Class that adds listeners for the Attribute changes to the Movement Component. 
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASTOOLBELT_API UTBCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	TWeakObjectPtr<UAbilitySystemComponent> OwnerAbilitySystemComponent;
	
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Update all the attributes, commonly at the start of the game. 
	void RefreshAllAttributeValues();

	/*
	 * Callback Functions that update the values of the Movement Component Based on the Attributes
	 */
	virtual void MaxWalkSpeedChanged(const FOnAttributeChangeData& data);
	virtual void JumpZVelocityChanged(const FOnAttributeChangeData& data);
	virtual void MaxAccelerationChanged(const FOnAttributeChangeData& data);
};
