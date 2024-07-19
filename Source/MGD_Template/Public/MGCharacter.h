// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MGCharacter.generated.h"

UCLASS()
class MGD_TEMPLATE_API AMGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMGCharacter();

	virtual void BeginPlay() override;

	UPROPERTY(Blueprintable, EditAnywhere, Category = "Parameters")
	float Dash_Force = 300;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Parameters")
	bool pCanDash = true;
	
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Parameters")
	FVector DefaultDashDirection = FVector(-1, 0, 0);

	// move character based on an axis
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Pure_MoveCharacter(const FVector Axis);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void Pure_RotateCharacter(const FVector2D Axis);

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector2D pMoveAxis;

	UPROPERTY(BlueprintReadOnly, Category = "Pitch")
	float pRepPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Pitch")
	float pRepYaw;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Pure_Dash( FVector Direction);

	FVector WorldSpaceUnitVector() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TryDash (const FVector Direction);

	UPROPERTY(Blueprintable, EditAnywhere, Category = "Parameters")
	float DashDelay;

	FTimerHandle CanDashTimer;

protected:
	UFUNCTION(Server, Reliable, Category= "Replication" )
	void Server_RepPitch(const float& Pitch, const float& Yaw);

	UFUNCTION(NetMulticast, Reliable, Category= "Replication" )
	void Multi_RepPitch(const float& Pitch, const float& Yaw);

	UFUNCTION(Server, Reliable, Category = "Replication" )
	void Server_Dash(const FVector Direction);

	UFUNCTION(NetMulticast, Reliable, Category = "Replication" )
	void Multi_SetDash(const bool CanDash);

	UFUNCTION(Server, Reliable, Category = "Replication" )
	void Server_SetDash();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};


