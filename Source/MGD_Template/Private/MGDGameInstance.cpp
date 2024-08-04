// Fill out your copyright notice in the Description page of Project Settings.


#include "MGDGameInstance.h"
#include  "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/GameModeBase.h"
#include "Online/OnlineSessionNames.h"

UMGDGameInstance::UMGDGameInstance()
{
	FoundSessions = MakeShareable(new FOnlineSessionSearch());
	FoundSessions->MaxSearchResults = 10;
	FoundSessions->bIsLanQuery = true;
	FoundSessions->QuerySettings.Set(SEARCH_LOBBIES,true,EOnlineComparisonOp::Equals);
}

void UMGDGameInstance::LoginEQS()
{
	const IOnlineSubsystem* ossRef = Online::GetSubsystem(GetWorld());

	if (!ossRef)
	{
		UE_LOG(LogTemp, Error, TEXT("No Online Susbsytem Ref"));
		return;
	}

	const IOnlineIdentityPtr identityref = ossRef->GetIdentityInterface();

	if (!identityref)
	{
		UE_LOG(LogTemp, Error, TEXT("No identity ref found"))
		return;
	}

	FOnlineAccountCredentials accCreds;
	accCreds.Id = FString();
	accCreds.Token = FString();
	accCreds.Type = FString(TEXT("accountportal"));
	
	identityref->Login(0, accCreds );
}

void UMGDGameInstance::Init()
{
	Super::Init();

	const IOnlineSubsystem* ossRef = Online::GetSubsystem(GetWorld());

	if (!ossRef)
		return;

	const IOnlineIdentityPtr identityref = ossRef->GetIdentityInterface();

	if (!identityref)
		return;

	identityref->OnLoginCompleteDelegates->AddUObject(this, &UMGDGameInstance::EOSLoginComplete);

	const IOnlineSessionPtr sessionRef = ossRef->GetSessionInterface();

	if (!sessionRef)
		return;

	//delegate runs a function that runs another function which has parameters
	sessionRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UMGDGameInstance::SessionCreatecomplete);

	sessionRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UMGDGameInstance::SessionFindComplete);

	sessionRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UMGDGameInstance::SessionJoinComplete);
}

bool UMGDGameInstance::IsLoggedIn() const
{
	const IOnlineSubsystem* ossRef = Online::GetSubsystem(GetWorld());

	if (!ossRef)
		return false;

	const IOnlineIdentityPtr identityref = ossRef->GetIdentityInterface();

	if (!identityref)
		return false;

	return identityref->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

FString UMGDGameInstance::GetDisplayName() const
{
	if (!IsLoggedIn())
		return FString(TEXT("DisplayName"));

	const IOnlineIdentityPtr identityref = Online::GetSubsystem(GetWorld())->GetIdentityInterface();
	
	return identityref->GetPlayerNickname(0);
}

bool UMGDGameInstance::IsInSession() const
{
	if (!IsLoggedIn())
		return false;

	const IOnlineSessionPtr sessionRef = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	if (!sessionRef)
		return false;

	const EOnlineSessionState::Type state = sessionRef->GetSessionState(MGSESSION_NAME);
	
		return (state != EOnlineSessionState::NoSession);
}

void UMGDGameInstance::HostGame(bool lan)
{
	if (!IsLoggedIn())
		return;

	const IOnlineSessionPtr sessionRef = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	if (!sessionRef)
		return;

	FOnlineSessionSettings settings;
	settings.NumPublicConnections = 4;
	settings.bIsLANMatch = true;
	settings.bIsDedicated = false;
	settings.bAllowInvites = true;
	settings.bShouldAdvertise = true;
	settings.bUsesPresence = true;
	settings.bUseLobbiesIfAvailable = true;
	settings.Set(SEARCH_KEYWORDS, MGSESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
	settings.Set(SEARCH_LOBBIES, true, EOnlineDataAdvertisementType::ViaOnlineService);
	

	sessionRef->CreateSession(0, MGSESSION_NAME, settings );
}

void UMGDGameInstance::FindAndJoinSession()
{
	if (!IsLoggedIn())
		return;

	const IOnlineSessionPtr sessionRef = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	if (!sessionRef)
		return;

	sessionRef->FindSessions(0, FoundSessions.ToSharedRef());
}

void UMGDGameInstance::StartLobbyGame()
{
	GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = true;
	GetWorld()->ServerTravel("Game/Content/MyContent/Maps_Lvl_Test", false);
}

void UMGDGameInstance::EOSLoginComplete(int32 LocalUsernum, bool bwWasSuccessful, const FUniqueNetId& UserId,
                                        const FString& Error)
{
	OnLoginComplete(bwWasSuccessful, Error);
}

void UMGDGameInstance::SessionCreatecomplete(FName SessionName, bool bWasSuccessful)
{
	EnableListenServer(true);
	OnSessionCreateComplete(bWasSuccessful);
}

void UMGDGameInstance::SessionFindComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find sessions"))
		OnSessionJoinComplete(false);
		return;
	}

	const IOnlineSubsystem* ossRef = Online::GetSubsystem(GetWorld());

	if (!ossRef)
		return;

	const IOnlineSessionPtr sessionRef = Online::GetSubsystem(GetWorld())->GetSessionInterface();

	if (!sessionRef)
		return;

	if (FoundSessions->SearchResults.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No Sessions Found"))
		OnSessionJoinComplete(false);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Found Sessions attempting to join") )

	sessionRef->JoinSession(0,MGSESSION_NAME,FoundSessions->SearchResults[0]);
}

void UMGDGameInstance::SessionJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	OnSessionJoinComplete(Result == EOnJoinSessionCompleteResult::Success);
	
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't Join Session"))
		return;
	}

	ClientTravelToSession(0,SessionName);
}
