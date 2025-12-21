
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BinarySoulTypes.generated.h"

UENUM(BlueprintType)
enum class EFactionColor : uint8
{
	None = 0 UMETA( DisplayName = "미점등" ),
	Red = 1 UMETA( DisplayName = "빨강 진영" ),
	Blue = 2 UMETA( DisplayName = "파란 진영")
};

USTRUCT(BlueprintType)
struct FChoiceData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthCost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactionColor FactionType;
};
