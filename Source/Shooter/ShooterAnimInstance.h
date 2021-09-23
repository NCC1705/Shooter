// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8 
{
	EOS_Aiming UMETA(DisplayName="Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")
};
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);//tick function for actor classes

	virtual void NativeInitializeAnimation() override;//begin play for actors

protected:

/* TURN IN PLACE */

	/** Handle turning in place variables */
	void TurnInPlace();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

/* MOVE */

	/** Character speed */
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category=Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	/** Character is in the air*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	/**  Character is moving, even constant movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	/** Offset yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;
	/** Offset yaw the frame before we stopped moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;



/* AIM & FIRE */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	


/* TURN IN PLACE */

	/** Yaw of the character this frame */
	float CharacterYaw;
	/** Yaw of the character the previous frame */
	float CharacterYawLastFrame;
	/** Keeps track of the offset used to rotate the root bone back */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;
	/** Rotation curve value this frame */
	float RotationCurve;
	/** Rotation curve value last frame */
	float RotationCurveLastFrame;
	/** The pitch for the aim rotation, used for Aim Offset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;
	/** True when reloading, used to prevent Aim Offset while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;
	/** Offset State used to determine which Offset State to use in animation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;
};
