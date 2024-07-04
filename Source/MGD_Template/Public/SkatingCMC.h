// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SkatingCMC.generated.h"

/**
 * 
 */
UENUM()
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Skate UMETA(DisplayName = "Skating"),
	CMOVE_MAX	UMETA(Hidden),
};

UCLASS()
class MGD_TEMPLATE_API USkatingCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Skate : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToSkate:1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
		
	};

	class FNetworkPredictionData_Client_Skate : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Skate(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
		
	};

	//Transient
	UPROPERTY(Transient) ACharacter* SkatingCharacterOwner;
	bool Safe_bWantsToSkate;

	

	virtual void InitializeComponent() override;
	//Network Functions
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void MaintainHorizontalGroundVelocity() override;
	

	// Custom Movement Mode Stuff

	
public:
	virtual bool IsMovingOnGround() const override;
	
	UFUNCTION(BlueprintPure)
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const ;

	UFUNCTION(BlueprintCallable, Category = "Custom")
	void ExitSlide();

	UFUNCTION(BlueprintCallable, Category = "Custom")
	void EnterSlide();

	
	

	bool GetSkateSurface(FHitResult& Hit) const;
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	//params
	UPROPERTY(EditDefaultsOnly) float Skate_GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly) float MaxDistToFloor = 300;
	UPROPERTY(EditDefaultsOnly) float Skate_Friction = 0.5;
	UPROPERTY(EditDefaultsOnly) FVector GravityDir = {0,0,-1};

	UPROPERTY(EditDefaultsOnly) float SkateMovementForce = 100;
	
private:
	void PhysSkate(float deltaTime, int32 Iterations);
	void PhysCustom(float deltaTime, int32 Iterations) override;
};

