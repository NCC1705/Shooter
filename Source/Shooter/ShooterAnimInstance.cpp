// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance():

	Speed(0.f),						//MOVE
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),	
	bAiming(false),					//AIM & FIRE
	CharacterYaw(0.f),				//TURN IN PLACE
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{

}

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
	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	
}



/* TURN IN PLACE */

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	if (Speed > 0)
	{
		//Don't turn in place if character is moving
		RootYawOffset = 0.f;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;//store last then update
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta{ CharacterYaw - CharacterYawLastFrame };
		
		//Root Yaw Offset, updated and clamped to [-180, 180]
		//RootYawOffset -= YawDelta;
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		//1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			/* 
			RootYawOffset > 0 -> Turning Left
			RootYawOffset < 0 -> Turning Right
			*/
			//Ternary operator
			/*if (RootYawOffset > 0) // Turning Left
			{
				RootYawOffset -= DeltaRotation;
			}
			else// Turning Right
			{
				RootYawOffset += DeltaRotation;
			}*/
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float AbsRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (AbsRootYawOffset > 90.f)
			{
				const float YawExcess{ AbsRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}

		//Print string examples
		/*if (GEngine) GEngine->AddOnScreenDebugMessage(
			1, // key for multiple messages if needed
			-1,// time to display
			FColor::Blue,
			FString::Printf(TEXT("CharacterYaw: %f"),
				CharacterYaw));//first input parameter is a key for multiple messages

		if (GEngine) GEngine->AddOnScreenDebugMessage(
			2, // key for multiple messages if needed
			-1,// time to display
			FColor::Red,
			FString::Printf(TEXT("RootYawOffset: %f"),
				RootYawOffset));//first input parameter is a key for multiple messages*/

	}
}
