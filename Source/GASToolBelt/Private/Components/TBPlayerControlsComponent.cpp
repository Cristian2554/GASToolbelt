// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TBPlayerControlsComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void UTBPlayerControlsComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* world = GetWorld();
	APawn* myOwner = GetPawn<APawn>();

	if (ensure(myOwner) && world->IsGameWorld())
	{
		myOwner->ReceiveRestartedDelegate.AddDynamic(this, &UTBPlayerControlsComponent::OnPawnRestarted);
		myOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &UTBPlayerControlsComponent::OnControllerChanged);

		// If our pawn has an input component we were added after restart
		if (myOwner->InputComponent)
		{
			OnPawnRestarted(myOwner);
		}
	}
}

void UTBPlayerControlsComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		ReleaseInputComponent();

		if (APawn* myOwner = GetPawn<APawn>())
		{
			myOwner->ReceiveRestartedDelegate.RemoveAll(this);
			myOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}
	}

	Super::OnUnregister();
}

void UTBPlayerControlsComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* playerInputComponent)
{
}

void UTBPlayerControlsComponent::TeardownPlayerControls_Implementation(UEnhancedInputComponent* playerInputComponent)
{
}

void UTBPlayerControlsComponent::OnPawnRestarted(APawn* pawn)
{
	if (ensure(pawn && pawn == GetOwner()) && pawn->InputComponent)
	{
		ReleaseInputComponent();

		if (pawn->InputComponent)
		{
			SetupInputComponent(pawn);
		}
	}
}

void UTBPlayerControlsComponent::OnControllerChanged(APawn* pawn, AController* oldController, AController* newController)
{
	// Only handle releasing, restart is a better time to handle binding
	if (ensure(pawn && pawn == GetOwner()) && oldController)
	{
		ReleaseInputComponent(oldController);
	}
}

void UTBPlayerControlsComponent::SetupInputComponent(APawn* pawn)
{
	InputComponent = Cast<UEnhancedInputComponent>(pawn->InputComponent);

	if (ensureMsgf(InputComponent, TEXT("Project must use EnhancedInputComponent to support PlayerControlsComponent")))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = GetEnhancedInputSubsystem(); subsystem && InputMappingContext)
		{
			subsystem->AddMappingContext(InputMappingContext, InputPriority);
		}

		SetupPlayerControls(InputComponent);
	}
}

void UTBPlayerControlsComponent::ReleaseInputComponent(AController* oldController)
{
	UEnhancedInputLocalPlayerSubsystem* subsystem = GetEnhancedInputSubsystem(oldController);
	if (subsystem && InputComponent)
	{
		TeardownPlayerControls(InputComponent);

		if (InputMappingContext)
		{
			subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	InputComponent = nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UTBPlayerControlsComponent::GetEnhancedInputSubsystem(AController* oldController) const
{
	// Fail safe check that this component was added to pawn, and avoid CastChecked if component was added to a non pawn
	// Might happen with Game Feature trying to add ability input binding on a PlayerState
	if (!GetPawn<APawn>())
	{
		return nullptr;
	}
	
	const APlayerController* playerController = GetController<APlayerController>();
	if (!playerController)
	{
		playerController = Cast<APlayerController>(oldController);
		if (!playerController)
		{
			return nullptr;
		}
	}

	const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
	if (!localPlayer)
	{
		return nullptr;
	}

	return localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

