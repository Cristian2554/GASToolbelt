// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TBCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/TBMovementSet.h"

// Called when the game starts
void UTBCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get a Reference of the ASC from the Owner Actor
	if (const IAbilitySystemInterface* ownerASI = Cast<IAbilitySystemInterface> (GetOwner()))
	{
		OwnerAbilitySystemComponent = ownerASI->GetAbilitySystemComponent();
	}

	if (OwnerAbilitySystemComponent.IsValid())
	{
		if (const UTBMovementSet* movementSet = Cast<UTBMovementSet> (OwnerAbilitySystemComponent->GetAttributeSet(UTBMovementSet::StaticClass())))
		{
			OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(movementSet->GetMaxWalkSpeedAttribute()).AddUObject(this, &UTBCharacterMovementComponent::MaxWalkSpeedChanged);
			OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(movementSet->GetJumpZVelocityAttribute()).AddUObject(this, &UTBCharacterMovementComponent::JumpZVelocityChanged);
			OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(movementSet->GetMaxAccelerationAttribute()).AddUObject(this, &UTBCharacterMovementComponent::MaxAccelerationChanged);

			RefreshAllAttributeValues();
		}
	}
}

void UTBCharacterMovementComponent::RefreshAllAttributeValues()
{
	if (OwnerAbilitySystemComponent.IsValid())
	{
		if (const UTBMovementSet* movementSet = Cast<UTBMovementSet> (OwnerAbilitySystemComponent->GetAttributeSet(UTBMovementSet::StaticClass())))
		{
			MaxWalkSpeed = movementSet->GetMaxWalkSpeed();
			JumpZVelocity = movementSet->GetJumpZVelocity();
			MaxAcceleration = movementSet->GetMaxAcceleration();
		}
	}
}

void UTBCharacterMovementComponent::MaxWalkSpeedChanged(const FOnAttributeChangeData& data)
{
	MaxWalkSpeed = data.NewValue;
}

void UTBCharacterMovementComponent::JumpZVelocityChanged(const FOnAttributeChangeData& data)
{
	JumpZVelocity = data.NewValue;
}

void UTBCharacterMovementComponent::MaxAccelerationChanged(const FOnAttributeChangeData& data)
{
	MaxAcceleration = data.NewValue;
}

