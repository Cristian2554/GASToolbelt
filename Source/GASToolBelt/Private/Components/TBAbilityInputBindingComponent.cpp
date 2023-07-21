// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TBAbilityInputBindingComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "AbilitySystemGlobals.h"
#include "ToolBeltLog.h"

namespace TBAbilityInputBindingComponent_Impl
{
	constexpr int32 InvalidInputID = 0;
	int32 IncrementingInputID = InvalidInputID;

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UTBAbilityInputBindingComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* playerInputComponent)
{
	ResetBindings();

	for (const auto& ability : MappedAbilities)
	{
		UInputAction* inputAction = ability.Key;
		const FTBAbilityInputBinding abilityInputBinding = ability.Value;

		// Convert out internal enum to the real Input Trigger Event for Enhanced Input
		const ETriggerEvent triggerEvent = GetInputActionTriggerEvent(abilityInputBinding.TriggerEvent);
		
		// Pressed event
		InputComponent->BindAction(inputAction, triggerEvent, this, &UTBAbilityInputBindingComponent::OnAbilityInputPressed, inputAction);

		// Released event
		InputComponent->BindAction(inputAction, ETriggerEvent::Completed, this, &UTBAbilityInputBindingComponent::OnAbilityInputReleased, inputAction);
	}

	if (TargetInputConfirm)
	{
		// Convert out internal enum to the real Input Trigger Event for Enhanced Input
		const ETriggerEvent triggerEvent = GetInputActionTriggerEvent(TargetConfirmTriggerEvent);
		OnConfirmHandle = InputComponent->BindAction(TargetInputConfirm, triggerEvent, this, &UTBAbilityInputBindingComponent::OnLocalInputConfirm).GetHandle();
	}

	if (TargetInputCancel)
	{
		// Convert out internal enum to the real Input Trigger Event for Enhanced Input
		const ETriggerEvent TriggerEvent = GetInputActionTriggerEvent(TargetCancelTriggerEvent);
		OnCancelHandle = InputComponent->BindAction(TargetInputCancel, TriggerEvent, this, &UTBAbilityInputBindingComponent::OnLocalInputCancel).GetHandle();
	}

	RunAbilitySystemSetup();
}

void UTBAbilityInputBindingComponent::ReleaseInputComponent(AController* oldController)
{
	ResetBindings();

	Super::ReleaseInputComponent();
}

void UTBAbilityInputBindingComponent::SetInputBinding(UInputAction* inputAction, const ETBAbilityTriggerEvent triggerEvent, const FGameplayAbilitySpecHandle abilityHandle)
{
	using namespace TBAbilityInputBindingComponent_Impl;

	FGameplayAbilitySpec* bindingAbility = FindAbilitySpec(abilityHandle);

	FTBAbilityInputBinding* abilityInputBinding = MappedAbilities.Find(inputAction);
	if (abilityInputBinding)
	{
		FGameplayAbilitySpec* oldBoundAbility = FindAbilitySpec(abilityInputBinding->BoundAbilitiesStack.Top());
		if (oldBoundAbility && oldBoundAbility->InputID == abilityInputBinding->InputID)
		{
			oldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		abilityInputBinding = &MappedAbilities.Add(inputAction);
		abilityInputBinding->InputID = GetNextInputID();
		abilityInputBinding->TriggerEvent = triggerEvent;
	}

	if (bindingAbility)
	{
		bindingAbility->InputID = abilityInputBinding->InputID;
	}

	abilityInputBinding->BoundAbilitiesStack.Push(abilityHandle);
	TryBindAbilityInput(inputAction, *abilityInputBinding);
}

void UTBAbilityInputBindingComponent::ClearInputBinding(const FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace TBAbilityInputBindingComponent_Impl;

	FGameplayAbilitySpec* foundAbility = FindAbilitySpec(AbilityHandle);
	if (!foundAbility)
	{
		return;
	}

	// Find the mapping for this ability
	auto mappedIterator = MappedAbilities.CreateIterator();
	while (mappedIterator)
	{
		if (mappedIterator.Value().InputID == foundAbility->InputID)
		{
			break;
		}

		++mappedIterator;
	}

	if (mappedIterator)
	{
		FTBAbilityInputBinding& AbilityInputBinding = mappedIterator.Value();

		if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
		{
			if (AbilityInputBinding.BoundAbilitiesStack.Num() > 0)
			{
				FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
				if (StackedAbility && StackedAbility->InputID == 0)
				{
					StackedAbility->InputID = AbilityInputBinding.InputID;
				}
			}
			else
			{
				// NOTE: This will invalidate the `AbilityInputBinding` ref above
				RemoveEntry(mappedIterator.Key());
			}
			// DO NOT act on `AbilityInputBinding` after here (it could have been removed)


			foundAbility->InputID = InvalidInputID;
		}
	}
}

void UTBAbilityInputBindingComponent::ClearAbilityBindings(UInputAction* inputAction)
{
	RemoveEntry(inputAction);
}

UInputAction* UTBAbilityInputBindingComponent::GetBoundInputActionForAbility(const UGameplayAbility* ability)
{
	if (!ability)
	{
		TB_LOG(Error, TEXT("GetBoundInputActionForAbility - Passed in ability is null."))
		return nullptr;
	}

	UAbilitySystemComponent* abilitySystemComponent = ability->GetAbilitySystemComponentFromActorInfo();
	if (!abilitySystemComponent)
	{
		TB_LOG(Error, TEXT("GetBoundInputActionForAbility - Trying to find input action for %s but AbilitySystemComponent from ActorInfo is null. (Ability's)"), *GetNameSafe(ability))
		return nullptr;
	}

	// Ensure and update inputs ID for specs based on mapped abilities.
	UpdateAbilitySystemBindings(abilitySystemComponent);

	const FGameplayAbilitySpec* abilitySpec = abilitySystemComponent->FindAbilitySpecFromClass(ability->GetClass());
	if (!abilitySpec)
	{
		TB_LOG(Error, TEXT("GetBoundInputActionForAbility - AbilitySystemComponent could not return Ability Spec for %s."), *GetNameSafe(ability->GetClass()))
		return nullptr;
	}

	return GetBoundInputActionForAbilitySpec(abilitySpec);
}


UInputAction* UTBAbilityInputBindingComponent::GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* abilitySpec) const
{
	check(abilitySpec);

	UInputAction* foundInputAction = nullptr;
	for (const TTuple<UInputAction*, FTBAbilityInputBinding>& mappedAbility : MappedAbilities)
	{
		const FTBAbilityInputBinding abilityInputBinding = mappedAbility.Value;
		if (abilityInputBinding.InputID == abilitySpec->InputID)
		{
			foundInputAction = mappedAbility.Key;
			break;
		}
	}

	return foundInputAction;
}

void UTBAbilityInputBindingComponent::ResetBindings()
{
	for (auto& InputBinding : MappedAbilities)
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnPressedHandle);
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnReleasedHandle);
		}

		if (AbilityComponent)
		{
			const int32 expectedInputID = InputBinding.Value.InputID;

			for (const FGameplayAbilitySpecHandle abilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* foundAbility = AbilityComponent->FindAbilitySpecFromHandle(abilityHandle);
				if (foundAbility && foundAbility->InputID == expectedInputID)
				{
					foundAbility->InputID = TBAbilityInputBindingComponent_Impl::InvalidInputID;
				}
			}
		}
	}

	if (InputComponent)
	{
		InputComponent->RemoveBindingByHandle(OnConfirmHandle);
		InputComponent->RemoveBindingByHandle(OnCancelHandle);
	}

	AbilityComponent = nullptr;
}

void UTBAbilityInputBindingComponent::RunAbilitySystemSetup()
{
	const AActor* myOwner = GetOwner();
	check(myOwner);

	AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(myOwner);
	if (AbilityComponent)
	{
		for (auto& inputBinding : MappedAbilities)
		{
			const int32 newInputID = TBAbilityInputBindingComponent_Impl::GetNextInputID();
			inputBinding.Value.InputID = newInputID;

			for (const FGameplayAbilitySpecHandle abilityHandle : inputBinding.Value.BoundAbilitiesStack)
			{
				if (FGameplayAbilitySpec* foundAbility = AbilityComponent->FindAbilitySpecFromHandle(abilityHandle))
				{
					foundAbility->InputID = newInputID;
				}
			}
		}
	}
}

void UTBAbilityInputBindingComponent::UpdateAbilitySystemBindings(UAbilitySystemComponent* abilitySystemComponent)
{
	if (!abilitySystemComponent)
	{
		TB_LOG(Error, TEXT("UTBAbilityInputBindingComponent::UpdateAbilitySystemBindings - Passed in ASC is invalid"))
		return;
	}

	for (auto& InputBinding : MappedAbilities)
	{
		const int32 InputID = InputBinding.Value.InputID;
		if (InputID <= 0)
		{
			continue;
		}

		for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
		{
			FGameplayAbilitySpec* FoundAbility = abilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
			if (FoundAbility != nullptr)
			{
				FoundAbility->InputID = InputID;
			}
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UTBAbilityInputBindingComponent::OnAbilityInputPressed(UInputAction* inputAction)
{
	// The AbilitySystemComponent may not have been valid when we first bound input... try again.
	if (AbilityComponent)
	{
		UpdateAbilitySystemBindings(AbilityComponent);
	}
	else
	{
		RunAbilitySystemSetup();
	}

	if (AbilityComponent)
	{
		using namespace TBAbilityInputBindingComponent_Impl;

		const FTBAbilityInputBinding* FoundBinding = MappedAbilities.Find(inputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputPressed(FoundBinding->InputID);
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UTBAbilityInputBindingComponent::OnAbilityInputReleased(UInputAction* inputAction)
{
	// The AbilitySystemComponent may need to have specs inputID updated here for clients... try again.
	UpdateAbilitySystemBindings(AbilityComponent);

	if (AbilityComponent)
	{
		using namespace TBAbilityInputBindingComponent_Impl;

		const FTBAbilityInputBinding* FoundBinding = MappedAbilities.Find(inputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputReleased(FoundBinding->InputID);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UTBAbilityInputBindingComponent::OnLocalInputConfirm()
{
	TB_LOG(Verbose, TEXT("UTBAbilityInputBindingComponent::OnLocalInputConfirm"))

	if (AbilityComponent)
	{
		AbilityComponent->LocalInputConfirm();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UTBAbilityInputBindingComponent::OnLocalInputCancel()
{
	TB_LOG(Verbose, TEXT("UTBAbilityInputBindingComponent::OnLocalInputCancel"))
	if (AbilityComponent)
	{
		AbilityComponent->LocalInputCancel();
	}
}

void UTBAbilityInputBindingComponent::RemoveEntry(const UInputAction* inputAction)
{
	if (FTBAbilityInputBinding* Bindings = MappedAbilities.Find(inputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (const FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace TBAbilityInputBindingComponent_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(inputAction);
	}
}

FGameplayAbilitySpec* UTBAbilityInputBindingComponent::FindAbilitySpec(const FGameplayAbilitySpecHandle handle) const
{
	FGameplayAbilitySpec* foundAbility = nullptr;
	if (AbilityComponent)
	{
		foundAbility = AbilityComponent->FindAbilitySpecFromHandle(handle);
	}
	return foundAbility;
}

void UTBAbilityInputBindingComponent::TryBindAbilityInput(UInputAction* inputAction, FTBAbilityInputBinding& abilityInputBinding)
{
	if (InputComponent)
	{
		// Pressed event
		if (abilityInputBinding.OnPressedHandle == 0)
		{
			// Convert out internal enum to the real Input Trigger Event for Enhanced Input
			const ETriggerEvent triggerEvent = GetInputActionTriggerEvent(abilityInputBinding.TriggerEvent);

			abilityInputBinding.OnPressedHandle = InputComponent->BindAction(inputAction, triggerEvent, this, &UTBAbilityInputBindingComponent::OnAbilityInputPressed, inputAction).GetHandle();
		}

		// Released event
		if (abilityInputBinding.OnReleasedHandle == 0)
		{
			abilityInputBinding.OnReleasedHandle = InputComponent->BindAction(inputAction, ETriggerEvent::Completed, this, &UTBAbilityInputBindingComponent::OnAbilityInputReleased, inputAction).GetHandle();
		}
	}
}

ETriggerEvent UTBAbilityInputBindingComponent::GetInputActionTriggerEvent(const ETBAbilityTriggerEvent triggerEvent)
{
	return triggerEvent == ETBAbilityTriggerEvent::Started ? ETriggerEvent::Started :
		triggerEvent == ETBAbilityTriggerEvent::Triggered ? ETriggerEvent::Triggered :
		ETriggerEvent::Started;
}