
#include "BinaryGameInstance.h"

UBinaryGameInstance::UBinaryGameInstance()
{
	LED_Array.Init(EFactionColor::None, 10);
}

void UBinaryGameInstance::UpdateLED(EFactionColor WinFaction)
{
	for (int32 i = 0; i < LED_Array.Num(); ++i)
	{
		if (LED_Array[i] == EFactionColor::None)
		{
			LED_Array[i] = WinFaction;
			break;
		}
	}
}