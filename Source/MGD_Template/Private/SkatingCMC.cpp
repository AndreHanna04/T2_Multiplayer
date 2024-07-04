// Fill out your copyright notice in the Description page of Project Settings.


#include "SkatingCMC.h"

#include "AsyncTreeDifferences.h"
#include "Chaos/ChaosPerfTest.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

// checks if the saved move is already equal to the new move and if so, will save bandwidth.
bool USkatingCMC::FSavedMove_Skate::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
                                                   float MaxDelta) const
{
	FSavedMove_Skate* NewSkateMove = static_cast<FSavedMove_Skate*>(NewMove.Get());

	if (Saved_bWantsToSkate != NewSkateMove->Saved_bWantsToSkate)
		return false;

	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}


void USkatingCMC::FSavedMove_Skate::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSkate = 0;
}

// Set Compressed flags for wanting to skate
uint8 USkatingCMC::FSavedMove_Skate::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
		if (Saved_bWantsToSkate)
			Result = FLAG_Custom_0;
	
	return Result;
}

//Update the saved move to the safe variable
void USkatingCMC::FSavedMove_Skate::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	//execute previous code
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const USkatingCMC* CharacterMovement = Cast<USkatingCMC>(C->GetCharacterMovement());
	//update
	Saved_bWantsToSkate = CharacterMovement->Safe_bWantsToSkate;
		
		
}

void USkatingCMC::FSavedMove_Skate::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	USkatingCMC* CharacterMovement = Cast<USkatingCMC>(C->GetCharacterMovement());
	CharacterMovement ->Safe_bWantsToSkate = Saved_bWantsToSkate;
}

USkatingCMC::FNetworkPredictionData_Client_Skate::FNetworkPredictionData_Client_Skate(
	const UCharacterMovementComponent& ClientMovement)  : Super (ClientMovement)
{
}


FSavedMovePtr USkatingCMC::FNetworkPredictionData_Client_Skate::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Skate());
}

FNetworkPredictionData_Client* USkatingCMC::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if(ClientPredictionData == nullptr)
	{
	USkatingCMC* MutableThis = const_cast<USkatingCMC*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Skate(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		
	}
	return ClientPredictionData;
}

void USkatingCMC::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	Safe_bWantsToSkate = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void USkatingCMC::MaintainHorizontalGroundVelocity()
{
	
}


void USkatingCMC::InitializeComponent()
{
	Super::InitializeComponent();
	SkatingCharacterOwner = Cast<ACharacter>(GetOwner());
}

 bool USkatingCMC::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return (MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Skate);
}

bool USkatingCMC::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Skate)  ;
}

void USkatingCMC::ExitSlide()
{
	Safe_bWantsToSkate = false;
	FQuat PassingRot = UpdatedComponent->GetComponentQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector,PassingRot, true, Hit);
	SetMovementMode(MOVE_Falling);
}

void USkatingCMC::EnterSlide()
{
	Safe_bWantsToSkate = true;
	SetMovementMode(MOVE_Custom, CMOVE_Skate);
}

FCollisionQueryParams USkatingCMC::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren = GetOwner()->Children;
	Params.AddIgnoredActors(CharacterChildren);
	return Params;
	
}

bool USkatingCMC::GetSkateSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * GravityDir;
	FName ProfileName = TEXT("BlockAll");
	
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, GetIgnoreCharacterParams());
}



void USkatingCMC::PhysSkate(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	RestorePreAdditiveRootMotionVelocity();
	

	
	//Acceleration += GravityDir * Skate_GravityForce;
	
	
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, Skate_Friction, true, GetMaxBrakingDeceleration());
	}
	
	ApplyRootMotionToVelocity(deltaTime);
	AddForce(FVector(0,0,-1) * Skate_GravityForce);


	//Perform Move

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FVector DeltaLoc = Velocity * deltaTime;
	FQuat NewRot = UpdatedComponent->GetComponentQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(DeltaLoc,NewRot , true, Hit);

	if(Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, DeltaLoc);
		SlideAlongSurface(DeltaLoc, deltaTime,Hit.Normal, Hit, true);
		
	}
	
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation)/ deltaTime;
	}

	

	
}

void USkatingCMC::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	PhysSkate(deltaTime, Iterations);
	/*
	switch (CustomMovementMode)
	{
	case (CMOVE_Skate):
		
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("INVALID MOVEMENT MODE"))
			
	}
	
	*/
}
	


	

