// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Components/PawnComponent.h"
#include "TBPlayerControlsComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASTOOLBELT_API UTBPlayerControlsComponent : public UPawnComponent
{
	GENERATED_BODY()
public:
	//~ Begin UActorComponent interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//~ End UActorComponent interface

	/** Input mapping to add to the input system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls")
	UInputMappingContext* InputMappingContext = nullptr;

	/** Priority to bind mapping context with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls")
	int32 InputPriority = 0;

protected:

	/** Native/BP Event to set up player controls */
	UFUNCTION(BlueprintNativeEvent, Category = "Player Controls")
	void SetupPlayerControls(UEnhancedInputComponent* playerInputComponent);

	/** Native/BP Event to undo control setup */
	UFUNCTION(BlueprintNativeEvent, Category = "Player Controls")
	void TeardownPlayerControls(UEnhancedInputComponent* playerInputComponent);

	/** Wrapper function for binding to this input component */
	template<class UserClass, typename FuncType>
	bool BindInputAction(const UInputAction* action, const ETriggerEvent eventType, UserClass* object, FuncType func)
	{
		if (ensure(InputComponent != nullptr) && ensure(action != nullptr))
		{
			InputComponent->BindAction(action, eventType, object, func);
			return true;
		}

		return false;
	}

	/** Called when pawn restarts, bound to dynamic delegate */
	UFUNCTION()
	virtual void OnPawnRestarted(APawn* pawn);

	/** Called when pawn restarts, bound to dynamic delegate */
	UFUNCTION()
	virtual void OnControllerChanged(APawn* pawn, AController* oldController, AController* newController);

	virtual void SetupInputComponent(APawn* pawn);
	virtual void ReleaseInputComponent(AController* oldController = nullptr);
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* oldController = nullptr) const;

	/** The bound input component. */
	UPROPERTY(transient)
	UEnhancedInputComponent* InputComponent;
};
