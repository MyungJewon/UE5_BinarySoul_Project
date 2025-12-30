#include "BinaryAnimInstance.h"
#include "ABinaryCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBinaryAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AABinaryCharacter>(TryGetPawnOwner());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UBinaryAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || !MovementComponent)
	{
		Character = Cast<AABinaryCharacter>(TryGetPawnOwner());
		if(Character) MovementComponent = Character->GetCharacterMovement();
		return;
	}

	FVector Velocity = Character->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	bShouldMove = (GroundSpeed > 3.0f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector);

	bIsFalling = MovementComponent->IsFalling();
}