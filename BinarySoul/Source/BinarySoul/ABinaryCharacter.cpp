// Fill out your copyright notice in the Description page of Project Settings.


#include "ABinaryCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABinaryChoiceButton.h"
#include "BinaryGameInstance.h"

// Sets default values
AABinaryCharacter::AABinaryCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	PlayerStats.MaxHealth = 100.0f;
	PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
}

void AABinaryCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController ->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
			PlayerStats = GI->SavedPlayerStats;
		UE_LOG(LogTemp, Warning, TEXT("Stats Loaded! HP: %f"), PlayerStats.CurrentHealth);
	}
	
}
void AABinaryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AABinaryCharacter::Interact);
		
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AABinaryCharacter::Attack);
	}

}

void AABinaryCharacter::Move(const FInputActionValue& Value)
{
	if (bIsAttacking) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AABinaryCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AABinaryCharacter::Interact()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector Forward = FollowCamera->GetForwardVector();
	FVector End = Start + (Forward * 800.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			ABinaryChoiceButton* Button = Cast<ABinaryChoiceButton>(HitActor);
			if (Button)
			{
				Button->OnInteracted(this);
			}
		}
	}
}
void AABinaryCharacter::Attack()
{
	if (bIsDead || bIsAttacking) return;

	if (AttackAnim)
	{
		// 2. 공격 상태로 변경 (이동 차단용)
		bIsAttacking = true;

		// 3. 움직임 강제 정지 (소울라이크 맛)
		GetCharacterMovement()->StopMovementImmediately();

		// 4. [핵심] 몽타주 없이 애니메이션 '직접' 재생!
		// (Looping은 false로 설정해서 한 번만 재생)
		GetMesh()->PlayAnimation(AttackAnim, false);

		// 5. 애니메이션 길이만큼 타이머 설정
		// (애니메이션이 1.5초짜리면, 1.5초 뒤에 OnAttackFinished를 실행해라!)
		float AnimDuration = AttackAnim->GetPlayLength();
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AABinaryCharacter::OnAttackFinished, AnimDuration, false);

		UE_LOG(LogTemp, Warning, TEXT("Attack Start! (Duration: %f)"), AnimDuration);
	}
}
void AABinaryCharacter::OnAttackFinished()
{
	// 공격 상태 해제 -> 이제 다시 움직일 수 있음
	bIsAttacking = false;
    
	// 원래의 애니메이션 블루프린트 상태(Idle/Run)로 복귀
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    
	UE_LOG(LogTemp, Warning, TEXT("Attack Finished!"));
}
void AABinaryCharacter::UpdateHealth(float HealthAmount)
{
	if (bIsDead) return;
	
	PlayerStats.CurrentHealth += HealthAmount;
	PlayerStats.CurrentHealth = FMath::Clamp(PlayerStats.CurrentHealth, 0.0f, PlayerStats.MaxHealth);
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Health Changed: %.1f / %.1f (Amount: %.1f)"), PlayerStats.CurrentHealth, PlayerStats.MaxHealth, HealthAmount);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
	}
	if (PlayerStats.CurrentHealth <= 0.0f)
	{
		bIsDead=true;
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			DisableInput(PC);
		}
		OnDeath();
	}
}

void AABinaryCharacter::UpdateMaxHealth(float Amount)
{
	PlayerStats.MaxHealth += Amount;
	if (Amount > PlayerStats.MaxHealth)
	{
		UpdateHealth(Amount);
	}
	else
	{
		PlayerStats.CurrentHealth = FMath::Clamp(PlayerStats.CurrentHealth, 0.0f, PlayerStats.MaxHealth);
	}
}

void AABinaryCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
