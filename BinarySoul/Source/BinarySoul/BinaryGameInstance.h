#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BinarySoulTypes.h"
#include "BinaryGameInstance.generated.h"

UCLASS()
class BINARYSOUL_API UBinaryGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UBinaryGameInstance();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|Meta")
	TArray<EFactionColor> LED_Array;
	
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|Meta")
	void UpdateLED(EFactionColor WinFaction);
};

