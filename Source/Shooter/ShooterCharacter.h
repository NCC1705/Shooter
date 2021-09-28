// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"//generated.h must always be last



UENUM(BlueprintType)
enum class ECombatState :uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

		// Scene component to use for its location for interping
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	USceneComponent* SceneComponent;
	//Number of items interping to at this scene cimp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;

};

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


/* MOVE */

	/** Called for forwards/backwards input */
	void MoveForward(float Value);
	/** Called for side to side input */
	void MoveRight(float Value);
	void CrouchButtonPressed();
	virtual void Jump() override;
	/** Interps capsule half height when crouching/standing */
	void InterpCapsuleHalfHeight(float DeltaTime);

/** TURN LOOK UP */

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



/*  AIM & FIRE */

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
	/** Fire Weapon functions */
	void PlayFireSound();
	void SendBullet();
	void PlayGunfireMontage();
	/** Bound to the R key and Gamepad Face Button Left */
	void ReloadButtonPressed();
	/** Handle reloading of the Weapon */
	void ReloadWeapon();
	/** Continue firing after reload */
	void ContinueFiringAfterReload();//My addition
	/** Called from AnimBP with GrabClip notify */
	UFUNCTION(BlueprintCallable)
	void GrabClip();	
	/** Called from AnimBP with ReplaceClip notify */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();
	void Aim();
	void StopAiming();
	

/* EQUIP*/

	/** Trace for items if OverlappedItemCount >0 */
	void TraceForItems();
	/** Spawns a default weapon and equips it*/
	class AWeapon* SpawnDefaultWeapon();
	/** Takes a weapon and attaches it to the skeletal mesh */
	void EquipWeapon(AWeapon* WeaponToEquip );
	/** Detach weapon and let it fall to the ground */
	void DropWeapon();
	void SelectButtonPressed();
	void SelectButtonReleased();
	/** Drops currently equipped Weapon and equips TraceHitItem */
	void SwapWeapon(AWeapon* WeaponToSwap);
	/** Picks up ammo */
	void PickupAmmo(class AAmmo* Ammo);
	/** Initializes the loctions for item pickup and interpolation */
	void InitializeInterpLocations();
	

/* AMMO */

	/** Initialize the Ammo Map with ammo values */
	void InitializeAmmoMap();
	/** Check to make sure our Weapon has Ammo */
	bool WeaponHasAmmo();
	/** Check to see if we have Ammo of the EquippedWeapon's ammo type */
	bool CarryingAmmo();


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;//forward declare

private:



/* CAMERA */

	/*Camera boom positioning the camera behind the character
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



/* MOVE */

	/** True wen crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;	
	/** Regular movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;
	/** Crouch movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/** Current half height of the capsule */
	float CurrentCapsuleHalfHeight;
	/** Half height of the capsule when not crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;
	/** Half height of the capsule when crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;
	/** Ground friction while not crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;
	/**  Ground friction while crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

/** TURN LOOK UP */

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



/* EFFECTS */

	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;
	/** Flash spawned at barrel socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;	
	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;//already forward declared 2 vars up
	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	
	
/* ANIMATION */

	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;
	/** Montage for reloading the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	/** Transform of the clip when we first grab the clip during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;
	/** Scene component to attach to the character hand during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;



/* AIM & FIRE */

	/** True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;
	/** Default camera field of view value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;
	/** Zoomed in camera field of view value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
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
	/** Used for knowing when the aiming button is pressed */
	bool bAimingButtonPressed;
	

/* TRACE FOR ITEMS */

	/** True if we should trace every frame for items */
	bool bShouldTraceForItems;
	/** Number of overlapped AItems */
	int8 OverlappedItemCount;
	/** The AItem we hit last frame to disable its widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;
	/** Currently equipped Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;
	//To spawn something we need a UClass, a C++ variable that holds a reference to a blueprint
	/** Set this in Blueprints for the default Weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	/** The Item currently hit by our trace in TraceForItems (could be null)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;
	/** Distance outward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;
	/** Distance upward from the camera for the interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;



/* EQUIP */

	/** Interp location for weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;// 
	/** Interp locationd for ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* AmmoInterpComp0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* AmmoInterpComp1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* AmmoInterpComp2;
	/** Array of interp location structs - how many items are interping to these locations at any time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;	
	//Sound variables and timers to prevent audio spam while picking up items
	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;
	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;
	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();
	/** Time to wait before we play another pickup sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;
	/** Time to wait before we play another equip sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;



/* AMMO */

	/** Map to keep track of ammo of different types */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;
	/** Starting amount of 9mm ammo*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;
	/** Starting amount of AR ammo*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;	


/* COMBAT */

	/** Combat State, can only fire or reload if Unoccupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))//Set from C++
	ECombatState CombatState;



/* DEBUG */
	FDateTime LastFireTime;
	

public:

/* MOVE */

	FORCEINLINE bool GetCrouching() const { return bCrouching; }



/* CAMERA */

	/** Returns CameraBoom subobject */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/* in code when we call GetCameraBoom(), by the time compilation is complete, the function will be replaced
	with literally what's inside the function body {return CameraBoom;}
	at runtime there's less jumping around which can save performance
	use simple functions, compiler decides if the function is inlined or not*/
	/** Returns FollowCamera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }



/*  AIM */

	FORCEINLINE bool GetAiming() const { return bAiming; }
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;//not FORCEINLINE because it needs to be bp callable



/* EQUIP */

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	/** Adds/substracts to/from OverlappedItemCount and updates bShouldTraceForItems */
	void IncrementOverlappedItemCount(int8 Amount);
	/** A vector in front/up of the camera For Item Interp movement when picked up */
	// No longer needed; AItem has GetInterpLocation
	//FVector GetCameraInterpLocation();
	void GetPickupItem(AItem* Item);
	/** Used to prevent Aim Offset while reloading */
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FInterpLocation GetInterpLocation(int32 Index);
	/** Returns the index in interp location array with lowest item count (zero or minimum items interping to it) */
	int32 GetInterpLocationBestIndex();
	void IncrementInterpLocItemCount(int32 Index, int32 Amount);
	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
};
