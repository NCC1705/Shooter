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

// Sets default values
AShooterCharacter::AShooterCharacter() ://initialize values with an initialize list
	//Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//turn rates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	//Mouse look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	//true when aiming the weapon
	bAiming(false),
	//Camera FOV values
	CameraDefaultFOV(0.f),//set in BeginPlay
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(10.f),
	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	//Bullet fire timer varialbes
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	//Automatic fire variables; remember to make this greater than crosshair interpolation time 0.05f
	AutomaticFireRate(0.1f),
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
	StartingARAmmo(120)
	
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f;//character follow distance
	CameraBoom->bUsePawnControlRotation = true;//rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

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

	InitializeAmmoMap();

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
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
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



/* AIM */

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
{
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
	bFireButtonPressed = true;
	StartFireTimer();
}
void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}
void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this, 
			&AShooterCharacter::AutoFireReset,
			AutomaticFireRate);
	}
}
void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}
float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
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
void AShooterCharacter::FireWeapon()
{
	//UE_LOG(LogTemp, Warning, TEXT("Fire Weapon."));

	if (FireSound)
	{
		//Ctrl Shift Space to see the parameters
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("WeaponBarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrosshairBulletFire();//start bullet fire timer for crosshairs
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
	bAiming = true;
	//removed - called in tick anyway; also, bad practice - should not be here
	//GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
}
void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	//removed - called in tick anyway; also, bad practice - should not be here
	//GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
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
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				//Show Item's pickup widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}	
			//we hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)//we are hitting a different item this frame, or AItem is null
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			//Store a reference to hit item for next frame
			TraceHitItemLastFrame = TraceHitItem;
			//	cast fails:		TraceHitItemLastFrame = nullptr 
			//  cast suceeds:	TraceHitItemLastFrame = AItem
			
		}
	}
	else if (TraceHitItemLastFrame)//we hit an AItem last frame
	{
		//no longer ovelapping items
		//item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
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
		//auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		//SwapWeapon(TraceHitWeapon);

		TraceHitItem->StartItemCurve(this);//shooter character pointer
	}
	
}
void AShooterCharacter::SelectButtonReleased()
{

}
void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{

	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;//can no longer turn off label for item - so we must hide at equip
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
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };
	//Desired = CameraWorldLocation + Forward*A + Up*B
	return CameraWorldLocation +
		CameraForward * CameraInterpDistance +
		FVector(0.f, 0.f, CameraInterpElevation);
	//camera roll will be always 0, levelled with the ground, not tilted, we dont need camera up vector

}
void AShooterCharacter::GetPickupItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}



/* AMMO */

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}





