// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/TBAsyncTaskAttributeChanged.h"

UTBAsyncTaskAttributeChanged* UTBAsyncTaskAttributeChanged::ListenForAttributeChange(UAbilitySystemComponent* abilitySystemComponent, FGameplayAttribute attribute)
{
	UTBAsyncTaskAttributeChanged* waitForAttributeChangedTask = NewObject<UTBAsyncTaskAttributeChanged>();
	waitForAttributeChangedTask->AbilitySystemComponent = abilitySystemComponent;
	waitForAttributeChangedTask->AttributeToListenFor = attribute;

	if (!IsValid(abilitySystemComponent) || !attribute.IsValid())
	{
		waitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	abilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attribute).AddUObject(waitForAttributeChangedTask, &UTBAsyncTaskAttributeChanged::AttributeChanged);

	return waitForAttributeChangedTask;
}

UTBAsyncTaskAttributeChanged * UTBAsyncTaskAttributeChanged::ListenForAttributesChange(UAbilitySystemComponent * abilitySystemComponent, TArray<FGameplayAttribute> attributes)
{
	UTBAsyncTaskAttributeChanged* waitForAttributeChangedTask = NewObject<UTBAsyncTaskAttributeChanged>();
	waitForAttributeChangedTask->AbilitySystemComponent = abilitySystemComponent;
	waitForAttributeChangedTask->AttributesToListenFor = attributes;

	if (!IsValid(abilitySystemComponent) || attributes.Num() < 1)
	{
		waitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	for (const FGameplayAttribute attribute : attributes)
	{
		abilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attribute).AddUObject(waitForAttributeChangedTask, &UTBAsyncTaskAttributeChanged::AttributeChanged);
	}

	return waitForAttributeChangedTask;
}

void UTBAsyncTaskAttributeChanged::EndTask()
{
	if (IsValid(AbilitySystemComponent.Get()))
	{
		AbilitySystemComponent.Get()->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);

		for (const FGameplayAttribute attribute : AttributesToListenFor)
		{
			AbilitySystemComponent.Get()->GetGameplayAttributeValueChangeDelegate(attribute).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UTBAsyncTaskAttributeChanged::AttributeChanged(const FOnAttributeChangeData & data)
{
	OnAttributeChanged.Broadcast(data.Attribute, data.NewValue, data.OldValue);
}