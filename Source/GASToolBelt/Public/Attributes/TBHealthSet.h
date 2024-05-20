// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TBAttributeSet.h"
#include "TBHealthSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthDepleted, FGameplayAttributeData, healthAttribute, float, previousValue);

/**
 * 
 */
UCLASS()
class GASTOOLBELT_API UTBHealthSet : public UTBAttributeSet
{
	GENERATED_BODY()

public:
	// Delegate called when the Health Reaches 0
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category="Health")
	FOnHealthDepleted OnHealthDepletedDelegate;
	
	// AttributeSet Overrides
	virtual void PreAttributeChange(const FGameplayAttribute& attribute, float& newValue) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& attribute, float& newValue) const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Current Health, when 0 we expect owner to die unless prevented by an ability. Capped by MaxHealth.
	// Positive changes can directly use this.
	// Negative changes to Health should go through Damage meta attribute.
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTBHealthSet, Health)

	// MaxHealth is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTBHealthSet, MaxHealth)

	// Check if Health has been depleted and call the delegate. 
	virtual void PostAttributeChange(const FGameplayAttribute& attribute, float oldValue, float newValue) override;
	
	/**
	* These OnRep functions exist to make sure that the ability system internal representations are synchronized properly during replication
	**/
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& oldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& oldMaxHealth);
};
