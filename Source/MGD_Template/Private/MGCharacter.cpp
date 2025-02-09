// Fill out your copyright notice in the Description page of Project Settings.


#include "MGCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AMGCharacter::AMGCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bReplicates = true;
	DashDelay = 3;
	pRepPitch = 0.0f;

}

void AMGCharacter::BeginPlay()
{
	Super::BeginPlay();
	pCanDash = true;
}

inline void AMGCharacter::Pure_MoveCharacter(const FVector Axis)
{
	
	AddMovementInput(GetControlRotation().Vector(), Axis.Y);
	
	AddMovementInput(UKismetMathLibrary::GetRightVector(GetControlRotation()), Axis.X) ;
	
	AddMovementInput(UKismetMathLibrary::GetUpVector(GetControlRotation()), Axis.Z) ;
}

void AMGCharacter::Pure_RotateCharacter(const FVector2D Axis)
{
	AddControllerYawInput(Axis.X);

	AddControllerPitchInput(-Axis.Y);

	// is the pRepPitch rep 10 units different from the controller's pitch
	if (FMath::Abs( pRepPitch - GetControlRotation().Pitch) >= 10 || pRepYaw - GetControlRotation().Yaw >= 10  )
	{
		//are we the server?
		if (HasAuthority())
		{
			Multi_RepPitch(GetControlRotation().Pitch, GetControlRotation().Yaw);
		}
		else
		{
			Server_RepPitch(GetControlRotation().Pitch, GetControlRotation().Yaw);
		}
	};
	
}


void AMGCharacter::Pure_Dash( FVector Direction)
{
	if (Direction.Equals(FVector(0,0,0)))
	{
		 Direction.X =  DefaultDashDirection.X ;
	}
		GetCharacterMovement()->StopMovementImmediately();
	
		GetCharacterMovement()->AddImpulse(GetControlRotation().Vector() * Direction.Y * Dash_Force, true);
		GetCharacterMovement()->AddImpulse(UKismetMathLibrary::GetRightVector(GetControlRotation()) * Direction.X * Dash_Force, true);
		GetCharacterMovement()->AddImpulse(UKismetMathLibrary::GetUpVector(GetControlRotation()) * Direction.Z * Dash_Force, true);
	
	
}

FVector AMGCharacter::WorldSpaceUnitVector() const
{
	FVector Value = FVector(0,0,0);
	Value += GetControlRotation().Vector() + UKismetMathLibrary::GetRightVector(GetControlRotation()) + UKismetMathLibrary::GetUpVector(GetControlRotation());
	return  Value;
}

void AMGCharacter::TryDash(const FVector Direction)
{
	if(HasAuthority())
	{
		if(pCanDash)
		{
			Pure_Dash(Direction);
			pCanDash = false;
			GetWorldTimerManager().SetTimer(CanDashTimer, this, &AMGCharacter::Server_SetDash ,DashDelay, false );
		}
	}
	else
	{
		if(pCanDash)
		{
			Pure_Dash(Direction);
			Server_Dash(Direction);
		}
	}
	
}

void AMGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//Each replicated parameter needs to be declared here like this
	DOREPLIFETIME(AMGCharacter, pCanDash)
}

void AMGCharacter::Server_SetDash_Implementation()
{
	pCanDash = true;
}

void AMGCharacter::Multi_SetDash_Implementation(const bool CanDash)
{
	pCanDash = CanDash;
}

void AMGCharacter::Server_Dash_Implementation(const FVector Direction)
{
	TryDash(Direction);
}

void AMGCharacter::Server_RepPitch_Implementation(const float& Pitch, const float& Yaw)
{
	//Run the multicast to update all of the clients
 Multi_RepPitch(Pitch, Yaw);
}


void AMGCharacter::Multi_RepPitch_Implementation(const float& Pitch, const float& Yaw)
{
	pRepPitch = Pitch;
	pRepYaw = Yaw;
};
