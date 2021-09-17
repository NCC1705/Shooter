// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// UShooterAnimInstance::=function name is fully qualified
void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		//Get the lateral speed of the character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;//zero out the Z component because we want lateral component of velocity
		Speed = Velocity.Size();//size=magnitude

		//Is the character in the air
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//Is character accelerating (actually just moving - speed can be constant)
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f) 
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		if (ShooterCharacter->GetVelocity().Size() > 0.f)//register only if still moving
		{
			LastMovementOffsetYaw = MovementOffsetYaw;//to properly blend stop jog when no longer moving
		}
		
		bAiming = ShooterCharacter->GetAiming();

		/*FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);	
		FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		}*/
	}

}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	
}
