// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate.
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate.
	*/
	void LookUpAtRate(float Rate);

	/**
	* Rotate controller based on mouse X movement.
	* @param Value	The input value from mouse movement.
	*/
	void Turn(float Value);

	/**
	* Rotate controller based on mouse Y movement.
	* @param Value	The input value from mouse movement.
	*/
	void LookUp(float Value);



	/** Called when the fire button is pressed */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	//Out = input param to be changed

	/** Set bAiming to true or false with button press */
	void AiminigButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	/** Set BaseTurnRate and BaseLookUpRate based on aiming */
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	//callback function for FTimerHandle - has to be UFUNCTION
	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();//timer callback - must be UFUNCTION

	/** Line trace for items under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*
	Camera boom positioning the camera behind the character
	macro marks varialbe for garbage collection + exposes to BP using specifiers
	BlueprintReadOnly has get not set
	meta=(AllowPrivateAccess="true") = private variable exposed in BP
	forward declare = compiler, this class exists, you don't need the header now
	we have the variable, pointer to USpringArmComponent
	varialbe that holds an address, no data yet in it*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Camera that follows the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;



	/** Base turn rate in degrees per second. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	/** Base look up/down rate in degrees per second. Other scaling may affect final look up rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;

	/** Turn rate ewhile not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float HipTurnRate;

	/** LookUp rate while not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float HipLookUpRate;

	/** Turn rate ewhile aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float AimingTurnRate;

	/** LookUp rate while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float AimingLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera,
		meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))//value+slider clamp
		float MouseHipTurnRate;
	/** Scale factor for mouse look sensitivity. LookUp rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera,
		meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))//value+slider clamp
		float MouseHipLookUpRate;
	/** Scale factor for mouse look sensitivity. Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera,
		meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))//value+slider clamp
		float MouseAimingTurnRate;
	/** Scale factor for mouse look sensitivity. LookUp rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera,
		meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))//value+slider clamp
		float MouseAimingLookUpRate;


	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class USoundCue* FireSound;

	/** Flash spawned at barrel socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* MuzzleFlash;

	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* HipFireMontage;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ImpactParticles;//already forward declared 2 vars up

		/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* BeamParticles;

	/** True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bAiming;

	/** Default camera field of view value */
	float CameraDefaultFOV;

	/** Zoomed in camera field of view value */
	float CameraZoomedFOV;

	/** Current field of view this frame for interpolation */
	float CameraCurrentFOV;

	/** Interp speed for zooming when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float ZoomInterpSpeed;

	/** Determines the spread of the crosshairs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
		float CrosshairSpreadMultiplier;

	/** Velocity component for crosshairs spread; low when moving slow, high when moving quickly*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
		float CrosshairVelocityFactor;

	/** In air component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
		float CrosshairInAirFactor;

	/** Aim component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
		float CrosshairAimFactor;

	/** Shooting component for crosshair spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
		float CrosshairShootingFactor;
	
	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	/** Left mouse button or right console trigger pressed*/
	bool bFireButtonPressed;

	/** True when we can fire. False while waiting for the timer*/
	bool bShouldFire;

	/** Rate of automatic gun fire */
	float AutomaticFireRate;

	/** Sets a timer between gunshots */
	FTimerHandle AutoFireTimer;

public:
	/** Returns CameraBoom subobject */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/* in code when we call GetCameraBoom(), by the time compilation is complete, the function will be replaced
	with literally what's inside the function body {return CameraBoom;}
	at runtime there's less jumping around which can save performance
	use simple functions, compiler decides if the function is inlined or not*/

	/** Returns FollowCamera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;//not FORCEINLINE because it needs to be bp callable

};
