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
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f),
	CharacterYawTurn(0.f),			//TURN IN PLACE
	CharacterYawLastFrameTurn(0.f),
	RootYawOffset(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip)
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
		bCrouching = ShooterCharacter->GetCrouching();
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;

		if (bReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}
		/*FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);	
		FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		}*/
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	
}



/* TURN IN PLACE */

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;	

	if (Speed > 0 || bIsInAir)
	{
		//Don't turn in place if character is moving
		RootYawOffset = 0.f;
		CharacterYawTurn = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrameTurn = CharacterYawTurn;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		CharacterYawLastFrameTurn = CharacterYawTurn;//store last then update
		CharacterYawTurn = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDeltaTurn{ CharacterYawTurn - CharacterYawLastFrameTurn };
		
		//Root Yaw Offset, updated and clamped to [-180, 180]
		//RootYawOffset -= YawDelta;
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDeltaTurn);

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



/* LEAN */

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	/* Not working jerk at +/-180 degrees
	CharacterYawLastFrame = CharacterYaw;
	CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
	const float Target{ (CharacterYaw - CharacterYawLastFrame) / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
	*/



	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,CharacterRotationLastFrame) };

	const float Target{ Delta.Yaw / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	/*
	if (GEngine) GEngine->AddOnScreenDebugMessage(
		2, 
		-1, 
		FColor::Cyan, 
		FString::Printf(TEXT("YawDelta: %f"), YawDelta));

	if (GEngine) GEngine->AddOnScreenDebugMessage(
		3,
		-1,
		FColor::Cyan,
		FString::Printf(TEXT("YawDelta: %f"), Delta.Yaw));
		*/


}


