// Fill out your copyright notice in the Description page of Project Settings.


#include "MGGameMode.h"

#include "NativeGameplayTags.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AMGGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	TArray<AActor*> playerStartArray;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(),APlayerStart::StaticClass(), playerStartArray );

	if(playerStartArray.IsEmpty())
		return;

	for (AActor* actorRef : playerStartArray)
	{
		if (APlayerStart* psRef = Cast<APlayerStart>(actorRef))
		{
			int playerStartIndex = 1;

			if (psRef->PlayerStartTag == FName("Player2"))
				playerStartIndex = 2;
			else if (psRef->PlayerStartTag == FName("Player3"))
				playerStartIndex = 3;
			else if (psRef->PlayerStartTag == FName("Player4"))
				playerStartIndex = 4;
			
			const int32 numPlayers = UGameplayStatics::GetNumPlayerStates(GetWorld());

			if (playerStartIndex == numPlayers)
			{
				RestartPlayerAtPlayerStart(NewPlayer, psRef);
				break;
			}
		}
	}
	
}
