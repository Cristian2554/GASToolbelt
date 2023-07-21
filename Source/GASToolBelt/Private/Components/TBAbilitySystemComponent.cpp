// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TBAbilitySystemComponent.h"
#include "Components/TBAbilityInputBindingComponent.h"
#include "GameFramework/PlayerState.h"
#include "ToolBeltLog.h"

void UTBAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	AbilityActivatedCallbacks.AddUObject(this, &UTBAbilitySystemComponent::OnAbilityActivatedCallback);
	AbilityFailedCallbacks.AddUObject(this, &UTBAbilitySystemComponent::OnAbilityFailedCallback);
	AbilityEndedCallbacks.AddUObject(this, &UTBAbilitySystemComponent::OnAbilityEndedCallback);

	// Grant startup effects on begin play instead of from within InitAbilityActorInfo to avoid
	// "ticking" periodic effects when BP is first opened
	GrantStartupEffects();
}

void UTBAbilitySystemComponent::BeginDestroy()
{
	// Reset ...

	// Clear any delegate handled bound previously for this component
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor.IsValid())
	{
		if (UGameInstance* gameInstance = AbilityActorInfo->OwnerActor->GetGameInstance())
		{
			gameInstance->GetOnPawnControllerChanged().RemoveAll(this);
		}
	}

	OnGiveAbilityDelegate.RemoveAll(this);

	// Remove any added attributes
	for (UAttributeSet* AttribSetInstance : AddedAttributes)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		RemoveSpawnedAttribute(AttribSetInstance);
#else
		GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
#endif
	}


	// Clear up abilities / bindings
	UTBAbilityInputBindingComponent* inputComponent = AbilityActorInfo && AbilityActorInfo->AvatarActor.IsValid() ? AbilityActorInfo->AvatarActor->FindComponentByClass<UTBAbilityInputBindingComponent>() : nullptr;

	for (const FTBMappedAbility& defaultAbilityHandle : DefaultAbilityHandles)
	{
		if (inputComponent)
		{
			inputComponent->ClearInputBinding(defaultAbilityHandle.Handle);
		}

		// Only Clear abilities on authority
		if (IsOwnerActorAuthoritative())
		{
			SetRemoveAbilityOnEnd(defaultAbilityHandle.Handle);
		}
	}

	Super::BeginDestroy();
}

void UTBAbilitySystemComponent::InitAbilityActorInfo(AActor* inOwnerActor, AActor* inAvatarActor)
{
	Super::InitAbilityActorInfo(inOwnerActor, inAvatarActor);

	TB_LOG(Log, TEXT("UGSCAbilitySystemComponent::InitAbilityActorInfo() - Owner: %s, Avatar: %s"), *GetNameSafe(inOwnerActor), *GetNameSafe(inAvatarActor))

	if (AbilityActorInfo)
	{
		if (AbilityActorInfo->AnimInstance == nullptr)
		{
			AbilityActorInfo->AnimInstance = AbilityActorInfo->GetAnimInstance();
		}

		if (UGameInstance* gameInstance = inOwnerActor->GetGameInstance())
		{
			// Sign up for possess / unpossess events so that we can update the cached AbilityActorInfo accordingly
			if (!gameInstance->GetOnPawnControllerChanged().Contains(this, TEXT("OnPawnControllerChanged")))
			{
				gameInstance->GetOnPawnControllerChanged().AddDynamic(this, &UTBAbilitySystemComponent::OnPawnControllerChanged);
			}
		}
	}

	GrantDefaultAbilitiesAndAttributes(inOwnerActor, inAvatarActor);
}

void UTBAbilitySystemComponent::AbilityLocalInputPressed(int32 inputID)
{
	// Consume the input if this inputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(inputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(inputID))
	{
		LocalInputCancel();
		return;
	}

	// ---------------------------------------------------------

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& spec : ActivatableAbilities.Items)
	{
		if (spec.InputID == inputID && spec.Ability)
		{
			spec.InputPressed = true;
			
			// Ability is not a combo ability, go through normal workflow
			if (spec.IsActive())
			{
				if (spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
				{
					ServerSetInputPressed(spec.Handle);
				}

				AbilitySpecInputPressed(spec);

				// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, spec.Handle, spec.ActivationInfo.GetActivationPredictionKey());
			}
			else
			{
				TryActivateAbility(spec.Handle);
			}
		}
	}
}

FGameplayAbilitySpecHandle UTBAbilitySystemComponent::GrantAbility(TSubclassOf<UGameplayAbility> ability,
	bool bRemoveAfterActivation)
{
	FGameplayAbilitySpecHandle abilityHandle;
	if (!IsOwnerActorAuthoritative())
	{
		TB_LOG(Error, TEXT("UTBAbilitySystemComponent::GrantAbility Called on non authority"));
		return abilityHandle;
	}

	if (ability)
	{
		FGameplayAbilitySpec abilitySpec(ability);
		abilitySpec.RemoveAfterActivation = bRemoveAfterActivation;

		abilityHandle = GiveAbility(abilitySpec);
	}
	return abilityHandle;
}

void UTBAbilitySystemComponent::OnAbilityActivatedCallback(UGameplayAbility* ability)
{
	TB_LOG(Log, TEXT("UTBAbilitySystemComponent::OnAbilityActivatedCallback %s"), *ability->GetName());
	const AActor* avatar = GetAvatarActor();
	if (!avatar)
	{
		TB_LOG(Error, TEXT("UGSCAbilitySystemComponent::OnAbilityActivated No OwnerActor for this ability: %s"), *ability->GetName());
	}
}

void UTBAbilitySystemComponent::OnAbilityFailedCallback(const UGameplayAbility* ability,
	const FGameplayTagContainer& tags)
{
	TB_LOG(Log, TEXT("UGSCAbilitySystemComponent::OnAbilityFailedCallback %s"), *ability->GetName());

	const AActor* avatar = GetAvatarActor();
	if (!avatar)
	{
		TB_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityFailed No OwnerActor for this ability: %s Tags: %s"), *ability->GetName(), *tags.ToString());
	}
}

void UTBAbilitySystemComponent::OnAbilityEndedCallback(UGameplayAbility* ability)
{
	TB_LOG(Log, TEXT("UGSCAbilitySystemComponent::OnAbilityEndedCallback %s"), *ability->GetName());
	const AActor* avatar = GetAvatarActor();
	if (!avatar)
	{
		TB_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityEndedCallback No OwnerActor for this ability: %s"), *ability->GetName());
	}
}

void UTBAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes(AActor* inOwnerActor, AActor* inAvatarActor)
{
	TB_LOG(Log, TEXT("UGSCAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes() - Owner: %s, Avatar: %s"), *inOwnerActor->GetName(), *inAvatarActor->GetName())

	if (bResetAttributesOnSpawn)
	{
		// Reset/Remove abilities if we had already added them
		for (UAttributeSet* attributeSet : AddedAttributes)
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			RemoveSpawnedAttribute(attributeSet);
#else
			GetSpawnedAttributes_Mutable().Remove(AttributeSet);
#endif
		}

		AddedAttributes.Empty(GrantedAttributes.Num());
	}

	if (bResetAbilitiesOnSpawn)
	{
		for (const FTBMappedAbility& defaultAbilityHandle : DefaultAbilityHandles)
		{
			SetRemoveAbilityOnEnd(defaultAbilityHandle.Handle);
		}

		for (FDelegateHandle inputBindingDelegateHandle : InputBindingDelegateHandles)
		{
			// Clear any delegate handled bound previously for this actor
			OnGiveAbilityDelegate.Remove(inputBindingDelegateHandle);
			inputBindingDelegateHandle.Reset();
		}

		DefaultAbilityHandles.Empty(GrantedAbilities.Num());
		InputBindingDelegateHandles.Empty();
	}

	UTBAbilityInputBindingComponent* inputComponent = IsValid(inAvatarActor) ? inAvatarActor->FindComponentByClass<UTBAbilityInputBindingComponent>() : nullptr;

	// Startup abilities
	for (const FTBAbilityInputMapping grantedAbility : GrantedAbilities)
	{
		const TSubclassOf<UGameplayAbility> ability = grantedAbility.Ability;
		UInputAction* inputAction = grantedAbility.InputAction;

		if (!ability)
		{
			continue;
		}
		
		FGameplayAbilitySpec newAbilitySpec(ability);

		// Try to grant the ability first
		if (IsOwnerActorAuthoritative() && ShouldGrantAbility(ability))
		{
			// Only Grant abilities on authority
			TB_LOG(Log, TEXT("UTBAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes - Authority, Grant Ability (%s)"), *newAbilitySpec.Ability->GetClass()->GetName())
			FGameplayAbilitySpecHandle abilityHandle = GiveAbility(newAbilitySpec);
			DefaultAbilityHandles.Add(FTBMappedAbility(abilityHandle, newAbilitySpec, inputAction));
		}

		// We don't grant here but try to get the spec already granted or register delegate to handle input binding
		if (inputComponent && inputAction)
		{
			// Handle for server or standalone game, clients need to bind OnGiveAbility
			if (const FGameplayAbilitySpec* abilitySpec = FindAbilitySpecFromClass(ability))
			{
				inputComponent->SetInputBinding(inputAction, grantedAbility.TriggerEvent, abilitySpec->Handle);
			}
			else
			{
				// Register a delegate triggered when ability is granted and available on clients
				FDelegateHandle delegateHandle = OnGiveAbilityDelegate.AddUObject(this, &UTBAbilitySystemComponent::HandleOnGiveAbility,
					inputComponent, inputAction, grantedAbility.TriggerEvent, newAbilitySpec);
				InputBindingDelegateHandles.Add(delegateHandle);
			}
		}
	}

	// Startup attributes
	for (const FTBAttributeSetDefinition& attributeSetDefinition : GrantedAttributes)
	{
		if (attributeSetDefinition.AttributeSet)
		{
			const bool bHasAttributeSet = GetAttributeSubobject(attributeSetDefinition.AttributeSet) != nullptr;
			TB_LOG(
				Verbose,
				TEXT("UTBAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes - HasAttributeSet: %s (%s)"),
				bHasAttributeSet ? TEXT("true") : TEXT("false"),
				*GetNameSafe(attributeSetDefinition.AttributeSet)
			)

			// Prevent adding attribute set if already granted
			if (!bHasAttributeSet)
			{
				UAttributeSet* attributeSet = NewObject<UAttributeSet>(inOwnerActor, attributeSetDefinition.AttributeSet);
				if (attributeSetDefinition.InitializationData)
				{
					attributeSet->InitFromMetaDataTable(attributeSetDefinition.InitializationData);
				}
				AddedAttributes.Add(attributeSet);
				AddAttributeSetSubobject(attributeSet);
			}
		}
	}
}

bool UTBAbilitySystemComponent::ShouldGrantAbility(TSubclassOf<UGameplayAbility> ability)
{
	if (bResetAbilitiesOnSpawn)
	{
		// User wants abilities to be granted each time InitAbilityActor is called
		return true;
	}

	// Check for activatable abilities, if one is matching the given Ability type, prevent re adding again
	TArray<FGameplayAbilitySpec> abilitySpecs = GetActivatableAbilities();
	for (const FGameplayAbilitySpec& activatableAbility : abilitySpecs)
	{
		if (!activatableAbility.Ability)
		{
			continue;
		}

		if (activatableAbility.Ability->GetClass() == ability)
		{
			return false;
		}
	}

	return true;
}

void UTBAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& abilitySpec)
{
	Super::OnGiveAbility(abilitySpec);
	TB_LOG(Log, TEXT("UGSCAbilitySystemComponent::OnGiveAbility %s"), *abilitySpec.Ability->GetName());
	OnGiveAbilityDelegate.Broadcast(abilitySpec);}

void UTBAbilitySystemComponent::GrantStartupEffects()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	// Reset/Remove effects if we had already added them
	for (const FActiveGameplayEffectHandle addedEffect : AddedEffects)
	{
		RemoveActiveGameplayEffect(addedEffect);
	}

	FGameplayEffectContextHandle effectContext = MakeEffectContext();
	effectContext.AddSourceObject(this);

	AddedEffects.Empty(GrantedEffects.Num());

	for (const TSubclassOf<UGameplayEffect> gameplayEffect : GrantedEffects)
	{
		FGameplayEffectSpecHandle newHandle = MakeOutgoingSpec(gameplayEffect, 1, effectContext);
		if (newHandle.IsValid())
		{
			FActiveGameplayEffectHandle effectHandle = ApplyGameplayEffectSpecToTarget(*newHandle.Data.Get(), this);
			AddedEffects.Add(effectHandle);
		}
	}
}

void UTBAbilitySystemComponent::OnPawnControllerChanged(APawn* pawn,AController* newController)
{
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor == pawn && AbilityActorInfo->PlayerController != newController)
	{
		if (!newController)
		{
			// NewController null, prevent refresh actor info. Needed to ensure TargetActor EndPlay properly unbind from GenericLocalConfirmCallbacks/GenericLocalCancelCallbacks
			// and avoid an ensure error if ActorInfo PlayerController is invalid
			return;
		}

		AbilityActorInfo->InitFromActor(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get(), this);
	}
}

void UTBAbilitySystemComponent::HandleOnGiveAbility(FGameplayAbilitySpec& abilitySpec,
	UTBAbilityInputBindingComponent* inputComponent, UInputAction* inputAction, ETBAbilityTriggerEvent triggerEvent,
	FGameplayAbilitySpec newAbilitySpec)
{
	TB_LOG(
	Log,
	TEXT("UTBAbilitySystemComponent::HandleOnGiveAbility: %s, Ability: %s, Input: %s (TriggerEvent: %s) - (InputComponent: %s)"),
	*abilitySpec.Handle.ToString(),
	*GetNameSafe(abilitySpec.Ability),
	*GetNameSafe(inputAction),
	*UEnum::GetValueAsName(triggerEvent).ToString(),
	*GetNameSafe(inputComponent)
);

	if (inputComponent && inputAction && abilitySpec.Ability == newAbilitySpec.Ability)
	{
		inputComponent->SetInputBinding(inputAction, triggerEvent, abilitySpec.Handle);
	}
}
