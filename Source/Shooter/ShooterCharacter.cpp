// Fill out your copyright notice in the Description page of Project Settings.



#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"//for weapon equip disable collision
#include "Components/BoxComponent.h"//for weapon equip disable collision
#include "Components/CapsuleComponent.h"
#include "Ammo.h"
//DEBUG
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"

// Sets default values
AShooterCharacter::AShooterCharacter() ://initialize values with an initialize list
	//Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(200.f),
	StandingCapsuleHalfHeight(88.f),//88
	CrouchingCapsuleHalfHeight(44.f),//44
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	//turn rates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	//Mouse look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
	//true when aiming the weapon
	bAiming(false),
	bAimingButtonPressed(false),
	//Camera FOV values
	CameraDefaultFOV(0.f),//set in BeginPlay
	CameraZoomedFOV(25.f),//35
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(10.f),
	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	//Bullet fire timer varialbes
	ShootTimeDuration(0.05f),//0.05
	bFiringBullet(false),
	//Automatic fire variables; remember to make this greater than crosshair interpolation time 0.05f
	AutomaticFireRate(0.1f),//0.1
	bShouldFire(true),
	bFireButtonPressed(false),
	//Item trace variables
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	//Camera interp location variables for picking up objects
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	//Starting ammo amounts
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	//Sound variables to prevent audio spam while picking up items
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	//Combat variables
	CombatState(ECombatState::ECS_Unoccupied)		
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.0f;//180 character follow distance
	CameraBoom->bUsePawnControlRotation = true;//rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 35.f, 80.f);//50,70

	//pleasant when moving, detrimental when aiming
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->bEnableCameraRotationLag = true;
	//CameraBoom->CameraLagSpeed = 2.f;
	//CameraBoom->CameraRotationLagSpeed = 2.f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);//attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false;//camera does not rotate relative to arm

	// Don't rotate when the controller rotates - let the controller only affect camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;//false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;//true;// Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);// ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	// Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// Create interpolation component for weapon
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	// Create interpolation components for ammo	
	
	AmmoInterpComp0 = CreateDefaultSubobject<USceneComponent>(TEXT("Ammo Interpolation Component 0"));
	AmmoInterpComp0->SetupAttachment(GetFollowCamera());
	AmmoInterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Ammo Interpolation Component 1"));
	AmmoInterpComp1->SetupAttachment(GetFollowCamera());
	AmmoInterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Ammo Interpolation Component 2"));
	AmmoInterpComp2->SetupAttachment(GetFollowCamera());
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	//Spawn the default weapon and equip it (attach it to the mesh)
	EquipWeapon (SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	//Create FInterpLocation structs for each interp location. Add to array.
	InitializeInterpLocations();

	//UE_LOG Examples:
	/*UE_LOG(LogTemp, Warning, TEXT("BeginPlay() called!"));//TEXT macro encoded in unicode - more characters

	int myInt{ 42 };
	UE_LOG(LogTemp, Warning, TEXT("int myInt: %d"), myInt);//%d = format specifier decimal?
	//braced initialize guarantees initialization empty {} = initialized at 0
	//narrowing conversions are avoided {42.54} compile error; {(int)42.54} compiles

	float myFloat{3.14159f};//uses f otherwise double causes narrowing coversion + braced initialization = error
	UE_LOG(LogTemp, Warning, TEXT("float myFloat: %f"), myFloat);

	double myDouble{ 0.000756 };
	UE_LOG(LogTemp, Warning, TEXT("double myDouble: %lf"), myDouble);

	char myChar{ 'J' };//char needs single quotes
	UE_LOG(LogTemp, Warning, TEXT("char myChar: %c"), myChar);

	wchar_t wideChar{ L'J' };//L = wide character literal
	UE_LOG(LogTemp, Warning, TEXT("wchar_t wideChar: %lc"), wideChar);

	bool myBool{ true };
	UE_LOG(LogTemp, Warning, TEXT("bool MyBool: %d"), myBool);//gets int equivalent for bool 0 or 1
	//no format specifier for bool, but we use %d for int, 
	//booleans are readily convertible to integer types

	UE_LOG(LogTemp, Warning, TEXT("int: %d, float: %f, bool %d"), myInt, myFloat, myBool);
	UE_LOG(LogTemp, Warning, TEXT("int: %d, float: %f, bool %d"), 5, 2.5f, true);

	FString myString{ TEXT("MyString!!!") };//Fstring is an object/struct
	UE_LOG(LogTemp, Warning, TEXT("FString myString: %s"), *myString);//not a pointer, overload on string

	UE_LOG(LogTemp, Warning, TEXT("Name of instance: %s"), *GetName());*/

}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); //DeltaTime= amount of time since last frame

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);

	//Change look sensitivity based on aiming
	SetLookRates();

	//Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	// Check OverlappedItemCount, then trace for items
	TraceForItems();
	//Interpolate the capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);//halts execution if not valid

	//WASD Move
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	//Arrows turn/look up
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	//Mouse turn/look up
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn); //&APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp); //&APawn::AddControllerPitchInput);

	//Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);//&ACharacter::Jump
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//Fire
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);//FireWeapon);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	//Aim
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AiminigButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	//Select
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	//Reload
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	//Crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	//Inventory Weapon select
	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}



/* MOVE */

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		//find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation }.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
	//DeltaTime = time between frames; speed*DeltaTime = correct movement 
}
void AShooterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		//find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation }.GetUnitAxis(EAxis::Y) };//right vector
		AddMovementInput(Direction, Value);
	}
}
void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}	
}
void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}
void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight{
		FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight,
		DeltaTime,20.f) };

	//Negative value if crouching, positive if standing
	const float DeltaCapsuleHalfHeigh{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f,0.f,-DeltaCapsuleHalfHeigh };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);

}




/* TURN LOOK UP*/

void AShooterCharacter::TurnAtRate(float Rate)//yaw = look left/right
{
	//calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	// deg/sec * sec/frame = deg/frame
}
void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	// deg/sec * sec/frame = deg/frame
}
void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(Value * TurnScaleFactor);

	//AddControllerYawInput(Value * (bAiming ? MouseAimingTurnRate : MouseHipTurnRate));
}
void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}	

	AddControllerPitchInput(Value * LookUpScaleFactor);

	//AddControllerPitchInput(Value * (bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate));
}



/* AIM & FIRE */

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{	
	//FVector2D WalkSpeedRange{ 0.f,GetCharacterMovement()->MaxWalkSpeed };
	FVector2D WalkSpeedRange{ 0.f,600.f };//hardwired why ???		
	FVector2D VelocityMultiplierRange{ 0.f,1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	//Crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, 
		VelocityMultiplierRange, 
		Velocity.Size());

	//Crosshair in air factor
	if (GetCharacterMovement()->IsFalling())// is in air?
	{
		//Spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor, 
			2.25f, 
			DeltaTime, 
			2.25f);
	}
	else//Character is on the ground
	{
		//shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			0.f,
			DeltaTime,
			10.f);
	}

	//Crosshair aim factor
	if (bAiming)// is aiming?
	{
		//Shrink crosshairs a small amount quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.5f,
			DeltaTime,
			10.f);
	}
	else//Not aiming
	{
		// Spread crosshairs back to normal quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			10.f);
	}

	//Crosshair firing factor - true for 0.05 seconds after firing
	if (bFiringBullet)// is aiming?
	{
		//Shrink crosshairs a small amount quickly
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.3f,
			DeltaTime,
			20.0f);
	}
	else
	{
		// Spread crosshairs back to normal quickly
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.f,
			DeltaTime,
			20.f);
	}


	CrosshairSpreadMultiplier = //0.5~1.5
		0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor-
		CrosshairAimFactor+
		CrosshairShootingFactor;

}
void AShooterCharacter::StartCrosshairBulletFire()
{//prevents click spam, firing as fast as user can click
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}
void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}
void AShooterCharacter::FireButtonPressed()
{	
	
	/*int myInt{(uint8)CombatState};
	UE_LOG(LogTemp, Warning, TEXT("int myInt: %d"), myInt);
	checked the state, was ok; forgot to set the ReloadAnimationMontage in ShooterCharacterBP,
	was getting stuck at Reload
	*/

	//UE_LOG(LogTemp, Warning, TEXT("bool MyBool: %d"), bFireButtonPressed);
	//UE_LOG(LogTemp, Warning, TEXT("Fire Button Pressed"));

	bFireButtonPressed = true;//this should ignore if we have ammo

	FireWeapon();	
}
void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}
void AShooterCharacter::StartFireTimer()
{//prevents button spam, firing as fast as user can click
	CombatState = ECombatState::ECS_FireTimerInProgress;

	GetWorldTimerManager().SetTimer(
		AutoFireTimer,
		this,
		&AShooterCharacter::AutoFireReset,
		AutomaticFireRate);
	
	/*FTimespan TimeSpan = LastFireTime - FDateTime().Now();
	int32 TotalMilliseconds = TimeSpan.GetTotalMilliseconds();
	UE_LOG(LogTemp, Warning, TEXT("elapsed between shots: %d"), TotalMilliseconds);
	LastFireTime = FDateTime().Now();*/

}
void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}
float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}
void AShooterCharacter::FireWeapon()
{
	//UE_LOG(LogTemp, Warning, TEXT("Fire Weapon."));
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();
		//Sunstract 1 from the Weapon's Ammo
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		//StartCrosshairBulletFire();//start bullet fire timer for crosshairs
	}
}
void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}

	//BaseTurnRate = bAiming? AimingTurnRate: HipTurnRate;
	//BaseLookUpRate = bAiming? AimingLookUpRate: HipLookUpRate;
}
bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if (bCrosshairHit)
	{
		//Tentative beam location - still need to trace from gun for obstacles
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else//no crosshair trace hit
	{
		//OutBeamLocation is the End location for the line trace
	}

	// Perform second trace from gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };//SocketTransform.GetLocation() };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };//line trace 25% longer
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit)//object between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}

	return false;
		
}
void AShooterCharacter::AiminigButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading)
	{
		Aim();
	}
	
	//removed - called in tick anyway; also, bad practice - should not be here
	//GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
}
void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
	//removed - called in tick anyway; also, bad practice - should not be here
	//GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
}
void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}
void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}
void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Set current camera field of view1
	if (bAiming)
	{
		// interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}
void AShooterCharacter::PlayFireSound()
{
	//Play fire sound
	if (FireSound)
	{
		//Ctrl Shift Space to see the parameters
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}
void AShooterCharacter::SendBullet()
{
	//Send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("WeaponBarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			//Spawn impact particles after updating beam end point - either on target or on obstacle
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEnd);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					BeamParticles,
					SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}

		// Linetracing from the crosshair - moved to function / refactored
		/*
		// Get current viewport size
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		//Get screen space location of crosshairs
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		CrosshairLocation.Y -= 50.f;
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;

		// Get world position and direction of crosshairs
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);

		if (bScreenToWorld)//was deprojection successful?
		{
			FHitResult ScreenTraceHit;
			const FVector Start{ CrosshairWorldPosition };
			const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

			// Set bean end point to line trace end point
			FVector BeamEndPoint{ End };

			// Trace outward from crosshair world location
			GetWorld()->LineTraceSingleByChannel(
				ScreenTraceHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility);

			if (ScreenTraceHit.bBlockingHit)// was there a trace hit?
			{
				// Beam end point is now trace hit location
				BeamEndPoint = ScreenTraceHit.Location;
				//instantiated later due to obstacle check
				//if (ImpactParticles){UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,ScreenTraceHit.Location);}
			}

			// Perform second trace from gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart{ SocketTransform.GetLocation() };
			const FVector WeaponTraceEnd{ BeamEndPoint };
			GetWorld()->LineTraceSingleByChannel(
				WeaponTraceHit,
				WeaponTraceStart,
				WeaponTraceEnd,
				ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit)//object between barrel and BeamEndPoint?
			{
				BeamEndPoint = WeaponTraceHit.Location;
			}

			//Spawn impact particles after updating beam end point - either on target or on obstacle
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEndPoint);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					BeamParticles,
					SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
		}*/
		// linetracing from the barrel forward - deprecated
		/*
		FHitResult FireHit;
		const FVector Start{ SocketTransform.GetLocation() };
		const FQuat Rotation{ SocketTransform.GetRotation() };//quaternion
		const FVector RotationAxis{ Rotation.GetAxisX() };//x axis is facing forward from the barrel socket
		const FVector End{ Start + RotationAxis * 50'000.f };

		FVector BeamEndPoint{ End };//big as line trace if we don't hit anything

		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		//last 2 params have default values, we can skip

		if (FireHit.bBlockingHit)
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
			//last 2 have default values  0, 2.f
			//DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);

			BeamEndPoint = FireHit.Location;

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
		*/

	}
}
void AShooterCharacter::PlayGunfireMontage()
{
	//Play Hip Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}
void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();//In the future, might contain other stuff
}
void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (EquippedWeapon == nullptr) return;

	// Do we have ammo of the correct type?	
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if (bAiming)
		{
			StopAiming();
		}

		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(
				EquippedWeapon->GetReloadMontageSection());
		}
	}
}
void AShooterCharacter::FinishReloading()
{
	//Update combat state
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon == nullptr) return;

	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	//Update AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		// Ammount of ammo the Character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		//Space left in the magazine of EquippedWeapon
		const int32 MagEmptySpace = 
			EquippedWeapon->GetMagazineCapacity() - 
			EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)//30 capacity only have 5 bullets
		{
			//Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);//map doesnt have duplicate keys
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}

	ContinueFiringAfterReload();
}
void AShooterCharacter::FinishEquipping()
{
	//Update combat state
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}
void AShooterCharacter::ContinueFiringAfterReload()
{
	if (WeaponHasAmmo())//My addition - continue firing after reload if button still pressed
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
}
void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	// Index for the clip bone of the equipped weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	// Store the transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	//KeepRelative - we are attaching the SceneComponent to the Mesh on the hand bone 
	//but we want to keep the relative location because we want that offset from the hand to the clip
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	
	EquippedWeapon->SetMovingClip(true);
}
void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}





/* EQUIP */

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation )
{	
	// Get current viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);	
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//Trace from Crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}
void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;//dummy var for trace - we don't use it here
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);//UE5 ItemTraceResult.GetActor()
			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;//already interping, cancel, avoid button spam
			}
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				//Show Item's pickup widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();
			}	
			//we hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)//we are hitting a different item this frame, or AItem is null
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			//Store a reference to hit item for next frame
			TraceHitItemLastFrame = TraceHitItem;
			//	cast fails:		TraceHitItemLastFrame = nullptr 
			//  cast suceeds:	TraceHitItemLastFrame = AItem
			
		}
	}
	else if (TraceHitItemLastFrame)//we hit an AItem last frame, but bShouldTraceForItems is false- no longer inside trace sphere
	{
		//no longer ovelapping items
		//item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}
AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	//Check the TSubclass variable
	if (DefaultWeaponClass)
	{
		//Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

		/*// Refactored
		//Spawn the Weapon
		AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

		//Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			//Attach the Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(DefaultWeapon, GetMesh());
		}
		//Set equipped weapon to the newly spawned weapon
		EquippedWeapon = DefaultWeapon;*/
	}

	return nullptr;
}
void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{	
		//Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			//Attach the Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		if (EquippedWeapon==nullptr)
		{
			//-1 no equipped weapon yet; no need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		//Set equipped weapon to the newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

	}
}
void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld,true);//note that the obj will be modified
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}
void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		if (CombatState != ECombatState::ECS_Unoccupied) return;
		//auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		//SwapWeapon(TraceHitWeapon);
		if (TraceHitItem)
		{
			TraceHitItem->StartItemCurve(this);//shooter character pointer
			TraceHitItem = nullptr;//prevents select button again and restart item curve
		}
	}
	
}
void AShooterCharacter::SelectButtonReleased()
{

}
void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;//can no longer turn off label for item - so we must hide at equip
}
void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// Check to see if AmmoMap contrains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get ammount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		// Set the amount of ammo in the Map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
	Ammo->Destroy();
}
void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	//safety against negative numbers, strange; do they not update properly, is this unreliable?
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else //OverlappedItemCount + Amount > 0
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}

}
/*No longer needed; AItem has GetInterpLocation
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };
	//Desired = CameraWorldLocation + Forward*A + Up*B
	return CameraWorldLocation +
		CameraForward * CameraInterpDistance +
		FVector(0.f, 0.f, CameraInterpElevation);
	//camera roll will be always 0, levelled with the ground, not tilted, we dont need camera up vector

}*/
void AShooterCharacter::GetPickupItem(AItem* Item)
{

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else //Inventory is full, swap with EquippedWeapon
		{
			SwapWeapon(Weapon);
		}		
	}	

	Item->PlayEquipSound();

	/*if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}*/

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}
void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp,0 };//0- no item is interpolating when we first start
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc0{ AmmoInterpComp0, 0 };
	InterpLocations.Add(InterpLoc0);
	FInterpLocation InterpLoc1{ AmmoInterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);
	FInterpLocation InterpLoc2{ AmmoInterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

}
void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}
void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}
void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}
void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}
void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}
void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}
void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if (CurrentItemIndex == NewItemIndex || NewItemIndex >= Inventory.Num() || CombatState != ECombatState::ECS_Unoccupied) return;
	
	auto OndEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	OndEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);//(EquipMontage, 1.0f) but playrate is default value, not needed
		AnimInstance->Montage_JumpToSection(FName("Equip"));//EquippedWeapon->GetEquipMontageSection()
		
	}

}
int32 AShooterCharacter::GetInterpLocationBestIndex()
{//indexes 1 to 3 for ammo, 0 is for weapon in array

	int32 LowestIndex = 1;//start index
	int32 LowestCount = INT_MAX;//large number to overwrite with number of items interping to each InterpLocation

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;			
		}
	}

	return LowestIndex;
	//return int32();
}
void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount>1) return;
	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}
void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;

	GetWorldTimerManager().SetTimer(
		PickupSoundTimer,
		this,
		&AShooterCharacter::ResetPickupSoundTimer,
		PickupSoundResetTime);
}
void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;

	GetWorldTimerManager().SetTimer(
		EquipSoundTimer,
		this,
		&AShooterCharacter::ResetEquipSoundTimer,
		EquipSoundResetTime);
}
FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}

	return FInterpLocation();
}
void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}
void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}



/* AMMO */

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}
bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	return EquippedWeapon->GetAmmo() > 0;	
}
bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;	
	}

	return false;
}





