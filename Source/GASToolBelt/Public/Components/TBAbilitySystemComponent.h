// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/TBAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "TBAbilitySystemComponent.generated.h"

class UTBAbilityInputBindingComponent;
class UInputAction;

USTRUCT(BlueprintType)
struct FTBAbilityInputMapping
{
	GENERATED_BODY()

	/** Type of ability to grant */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TSubclassOf<UGameplayAbility> Ability;

	/** Input action to bind the ability to, if any (can be left unset) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	UInputAction* InputAction = nullptr;

	/**
	 * The EnhancedInput Trigger Event type to use for the ability activation on pressed handle.
	 *
	 * The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button,
	 * and Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: The Triggered value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with ability activation (since you'll be
	 * trying to activate abilities every frame). When in doubt, use the default Started value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability, meta=(EditCondition = "InputAction != nullptr", EditConditionHides))
	ETBAbilityTriggerEvent TriggerEvent = ETBAbilityTriggerEvent::Started;
};

USTRUCT(BlueprintType)
struct FTBAttributeSetDefinition
{
	GENERATED_BODY()

	/** Attribute Set to grant */
	UPROPERTY(EditAnywhere, Category=Attributes)
	TSubclassOf<UAttributeSet> AttributeSet;

	/** Data table reference to initialize the attributes with, if any (can be left unset) */
	UPROPERTY(EditAnywhere, Category=Attributes, meta = (RequiredAssetDataTags = "RowStructure=/Script/GameplayAbilities.AttributeMetaData"))
	UDataTable* InitializationData = nullptr;
};

USTRUCT()
struct FTBMappedAbility
{
	GENERATED_BODY()

	FGameplayAbilitySpecHandle Handle;
	FGameplayAbilitySpec Spec;

	UPROPERTY(Transient)
	UInputAction* InputAction;

	FTBMappedAbility(): InputAction(nullptr)
	{
	}

	FTBMappedAbility(const FGameplayAbilitySpecHandle& handle, const FGameplayAbilitySpec& spec, UInputAction* const inputAction)
		: Handle(handle),
		  Spec(spec),
		  InputAction(inputAction)
	{
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FTBOnGiveAbility, FGameplayAbilitySpec&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTBOnInitAbilityActorInfo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASTOOLBELT_API UTBAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** List of Gameplay Abilities to grant when the Ability System Component is initialized */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Abilities")
	TArray<FTBAbilityInputMapping> GrantedAbilities;

	/** List of Attribute Sets to grant when the Ability System Component is initialized, with optional initialization data */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Abilities")
	TArray<FTBAttributeSetDefinition> GrantedAttributes;

	/** List of GameplayEffects to apply when the Ability System Component is initialized (typically on begin play) */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/**
	 * Specifically set abilities to persist across deaths / respawns or possessions (Default is true)
	 *
	 * When this is set to false, abilities will only be granted the first time InitAbilityActor is called. This is the default
	 * behavior for ASC living on Player States (GSCModularPlayerState specifically).
	 *
	 * Do not set it true for ASC living on Player States if you're using ability input binding. Only ASC living on Pawns supports this.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Abilities")
	bool bResetAbilitiesOnSpawn = true;

	/**
	 * Specifically set attributes to persist across deaths / respawns or possessions (Default is true)
	 *
	 * When this is set to false, attributes are only granted the first time InitAbilityActor is called. This is the default
	 * behavior for ASC living on Player States (GSCModularPlayerState specifically).
	 *
	 * Set it (or leave it) to true if you want attribute values to be re-initialized to their default values.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Abilities")
	bool bResetAttributesOnSpawn = true;

	/** Delegate invoked OnGiveAbility (when an ability is granted and available) */
	FTBOnGiveAbility OnGiveAbilityDelegate;

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	//~ End UActorComponent interface

	//~ Begin UObject interface
	virtual void BeginDestroy() override;
	//~ End UObject interface

	//~ Begin UAbilitySystemComponent interface
	virtual void InitAbilityActorInfo(AActor* inOwnerActor, AActor* inAvatarActor) override;

	/**
	 * Overrides InputPressed to conditionally ActivateComboAbility or regular TryActivateAbility based on AbilitySpec Ability CDO
	 * (if child of GSCMeleeAbility, will activate combo via combo component)
	 */
	virtual void AbilityLocalInputPressed(int32 inputID) override;
	//~ End UAbilitySystemComponent interface

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tool Belt|Abilities")
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> ability, bool bRemoveAfterActivation);

	//~ Those are Delegate Callbacks register with this ASC to trigger corresponding events on the Owning Character (mainly for ability queuing)
	virtual void OnAbilityActivatedCallback(UGameplayAbility* ability);
	virtual void OnAbilityFailedCallback(const UGameplayAbility* ability, const FGameplayTagContainer& tags);
	virtual void OnAbilityEndedCallback(UGameplayAbility* ability);

	/** Called when Ability System Component is initialized from InitAbilityActorInfo */
	virtual void GrantDefaultAbilitiesAndAttributes(AActor* inOwnerActor, AActor* inAvatarActor);

	/** Called from GrantDefaultAbilitiesAndAttributes. Determine if ability should be granted, prevents re-adding an ability previously granted in case bResetAbilitiesOnSpawn is set to false */
	virtual bool ShouldGrantAbility(TSubclassOf<UGameplayAbility> ability);

	/**
	 * Event called just after InitAbilityActorInfo, once abilities and attributes have been granted.
	 *
	 * This will happen multiple times for both client / server:
	 *
	 * - Once for Server after component initialization
	 * - Once for Server after replication of owning actor (Possessed by for Player State)
	 * - Once for Client after component initialization
	 * - Once for Client after replication of owning actor (Once more for Player State OnRep_PlayerState)
	 *
	 * Also depends on whether ASC lives on Pawns or Player States.
	 */
	UPROPERTY(BlueprintAssignable, Category="Tool Belt|Abilities")
	FTBOnInitAbilityActorInfo OnInitAbilityActorInfo;

protected:
	UPROPERTY(transient)
	TArray<FTBMappedAbility> DefaultAbilityHandles;

	// Cached granted AttributeSets
	UPROPERTY(transient)
	TArray<UAttributeSet*> AddedAttributes;

	// Cached applied Startup Effects
	UPROPERTY(transient)
	TArray<FActiveGameplayEffectHandle> AddedEffects;

	// Keep track of OnGiveAbility handles bound to handle input binding on clients
	TArray<FDelegateHandle> InputBindingDelegateHandles;

	//~ Begin UAbilitySystemComponent interface
	virtual void OnGiveAbility(FGameplayAbilitySpec& abilitySpec) override;
	//~ End UAbilitySystemComponent interface

	/** Called when Ability System Component is initialized */
	void GrantStartupEffects();

	/** Reinit the cached ability actor info (specifically the player controller) */
	UFUNCTION()
	void OnPawnControllerChanged(APawn* pawn,AController* newController);

	/** Handler for AbilitySystem OnGiveAbility delegate. Sets up input binding for clients (not authority) when ability is granted and available for binding. */
	virtual void HandleOnGiveAbility(FGameplayAbilitySpec& abilitySpec, UTBAbilityInputBindingComponent* inputComponent,
		UInputAction* inputAction, ETBAbilityTriggerEvent triggerEvent, FGameplayAbilitySpec newAbilitySpec);
};
