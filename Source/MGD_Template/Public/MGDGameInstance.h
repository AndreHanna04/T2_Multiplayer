// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MGDGameInstance.generated.h"


#define MGSESSION_NAME FName(TEXT("MGSESSION"))
/**
 * 
 */
UCLASS()
class MGD_TEMPLATE_API UMGDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMGDGameInstance();

//	ULevel GameWorld;
	
	UFUNCTION(BlueprintCallable)
	void LoginEQS();

	virtual void Init() override;

	UFUNCTION(BlueprintPure)
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintPure)
	FString GetDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostGame(bool lan);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindAndJoinSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartLobbyGame();

	
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Login")
	void OnLoginComplete(bool success, const FString& error);

	UFUNCTION(BlueprintImplementableEvent, Category = "Session")
	void OnSessionCreateComplete(bool success);

	UFUNCTION(BlueprintImplementableEvent, Category = "Session")
	void OnSessionJoinComplete(bool success);
	
private:
	void EOSLoginComplete(int32 LocalUsernum, bool bwWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void SessionCreatecomplete(FName SessionName, bool bWasSuccessful);

	void SessionFindComplete(bool bWasSuccessful);

	void SessionJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	TSharedPtr<FOnlineSessionSearch> FoundSessions;


};
