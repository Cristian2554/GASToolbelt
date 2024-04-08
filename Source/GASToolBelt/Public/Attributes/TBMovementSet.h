// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TBAttributeSet.h"
#include "TBMovementSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class GASTOOLBELT_API UTBMovementSet : public UTBAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MaxWalkSpeed)
	FGameplayAttributeData MaxWalkSpeed;
	ATTRIBUTE_ACCESSORS(UTBMovementSet, MaxWalkSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_JumpZVelocity)
    FGameplayAttributeData JumpZVelocity;
    ATTRIBUTE_ACCESSORS(UTBMovementSet, JumpZVelocity)

	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MaxAcceleration)
    FGameplayAttributeData MaxAcceleration;
    ATTRIBUTE_ACCESSORS(UTBMovementSet, MaxAcceleration)

	/**
	* These OnRep functions exist to make sure that the ability system internal representations are synchronized properly during replication
	**/
	UFUNCTION()
	virtual void OnRep_MaxWalkSpeed(const FGameplayAttributeData& oldMaxWalkSpeed);
	
	UFUNCTION()
	virtual void OnRep_JumpZVelocity(const FGameplayAttributeData& oldJumpZVelocity);
	
	UFUNCTION()
	virtual void OnRep_MaxAcceleration(const FGameplayAttributeData& oldMaxAcceleration);
};
