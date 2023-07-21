// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/TBAbilityTypes.h"
#include "TBPlayerControlsComponent.h"	
#include "TBAbilityInputBindingComponent.generated.h"

USTRUCT()
struct FTBAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;

	ETBAbilityTriggerEvent TriggerEvent = ETBAbilityTriggerEvent::Started;
};

/**
 * Modular pawn component that hooks up enhanced input to the ability system input logic
 *
 * Extends from GTBPlayerControlsComponent, so if your Pawn is dealing with Abilities use this component instead.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASTOOLBELT_API UTBAbilityInputBindingComponent : public UTBPlayerControlsComponent
{
	GENERATED_BODY()

public:
	/** Input action to handle Target Confirm for ASC */
	UPROPERTY(EditDefaultsOnly, Category = "Player Controls", meta=(DisplayAfter="InputPriority"))
	UInputAction* TargetInputConfirm = nullptr;

	/**
	 * The EnhancedInput Trigger Event type to use for the target confirm input.
	 *
	 * The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button,
	 * and Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: The Triggered value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with abilities. When in doubt, use the default Started value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Player Controls", meta=(DisplayAfter="InputPriority", EditCondition = "TargetInputConfirm != nullptr", EditConditionHides))
	ETBAbilityTriggerEvent TargetConfirmTriggerEvent = ETBAbilityTriggerEvent::Started;

	/** Input action to handle Target Cancel for ASC */
	UPROPERTY(EditDefaultsOnly, Category = "Player Controls", meta=(DisplayAfter="InputPriority"))
	UInputAction* TargetInputCancel = nullptr;

	/** 
	 * The EnhancedInput Trigger Event type to use for the target cancel input.
	 *
	 * The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button,
	 * and Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: The Triggered value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with abilities. When in doubt, use the default Started value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Player Controls", meta=(DisplayAfter="InputPriority", EditCondition = "TargetInputCancel != nullptr", EditConditionHides))
	ETBAbilityTriggerEvent TargetCancelTriggerEvent = ETBAbilityTriggerEvent::Started;

	//~ Begin UTBPlayerControlsComponent interface
	virtual void SetupPlayerControls_Implementation(UEnhancedInputComponent* playerInputComponent) override;
	virtual void ReleaseInputComponent(AController* oldController) override;
	//~ End UTBPlayerControlsComponent interface

	/**
	 * Updates the Ability Input Binding Component registered bindings or create a new one for the passed in Ability Handle.
	 *
	 * @param inputAction The Enhanced InputAction to bind to
	 * @param triggerEvent The trigger type to use for the ability pressed handle. The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button.
	 * @param abilityHandle The Gameplay Ability Spec handle to setup binding for (handle returned when granting abilities manually with GSCAbilitySystemComponent)
	 */
	UFUNCTION(BlueprintCallable, Category = "Tool Belt|Abilities")
	void SetInputBinding(UInputAction* inputAction, ETBAbilityTriggerEvent triggerEvent, FGameplayAbilitySpecHandle abilityHandle);

	/** Given a Gameplay Ability Spec handle (handle returned when granting abilities manually with GSCAbilitySystemComponent), clears up and reset the previously registered binding for that ability.  */
	UFUNCTION(BlueprintCallable, Category = "Tool Belt|Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	/** Given an Enhanced Input Action, clears up input binding delegates (On Pressed and Released) and resets any abilities' (that were bound to that action) InputId to none. */
	UFUNCTION(BlueprintCallable, Category = "Tool Belt|Abilities")
	void ClearAbilityBindings(UInputAction* inputAction);

	/**
	 * Given a Gameplay Ability, returns the bound InputAction from mapped abilities (previously bound abilities) that matches the Ability Spec InputID.
	 *
	 * Designed to be called from within a Gameplay Ability event graph, passing self reference for the Gameplay Ability parameter.
	 */
	UFUNCTION(BlueprintPure, Category = "Tool Belt|Abilities")
	UInputAction* GetBoundInputActionForAbility(UPARAM(ref) const UGameplayAbility* ability);

	/** Internal helper to return InputAction from MappedAbilities that match Ability Spec InputID */
	UInputAction* GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* abilitySpec) const;

private:
	UPROPERTY(transient)
	UAbilitySystemComponent* AbilityComponent;

	UPROPERTY(transient)
	TMap<UInputAction*, FTBAbilityInputBinding> MappedAbilities;

	uint32 OnConfirmHandle = 0;
	uint32 OnCancelHandle = 0;

	void ResetBindings();
	void RunAbilitySystemSetup();

	/**
	 * Runs on press / release, and updates inputs ID for specs based on mapped abilities.
	 *
	 * Needs to run everytime to handle the issue with lost inputID when playing as client after first PIE session if BP containing ASC is compiled in Editor. */
	void UpdateAbilitySystemBindings(UAbilitySystemComponent* abilitySystemComponent);

	void OnAbilityInputPressed(UInputAction* inputAction);
	void OnAbilityInputReleased(UInputAction* inputAction);
	void OnLocalInputConfirm();
	void OnLocalInputCancel();

	void RemoveEntry(const UInputAction* inputAction);

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle handle) const;
	void TryBindAbilityInput(UInputAction* inputAction, FTBAbilityInputBinding& abilityInputBinding);

	static ETriggerEvent GetInputActionTriggerEvent(ETBAbilityTriggerEvent triggerEvent);
};
