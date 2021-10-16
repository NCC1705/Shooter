// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"//last one always

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()//get all built in reflection code generated
//
UPROPERTY(EditAnywhere, BlueprintReadWrite)
USkeletalMesh* ItemMesh;

	//Weapon Specs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 WeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MagazineCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AutoFireRate;


	// Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USoundCue* PickupSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* EquipSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* FireSound;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)//Set in BaseWeaponBP, invarialbe for all weapons
	//class UWidgetComponent* PickupWidget;
	

	// Animations	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)//reload
		FName ClipBoneName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ReloadMontageSection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UAnimInstance> AnimBP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;


	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)//item highlight
		UMaterialInstance* MaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaterialIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;

	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ItemIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* AmmoIcon;

	// UI Crosshairs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsMiddle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsBottom;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CrosshairsTop;


};


/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();//constructor
	virtual void Tick(float DeltaTime) override;

protected://protected functions

	void StopFalling();//get into pickup state
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	/* ANIMATION */
	void FinishMovingSlide();
	void UpdateSlideDisplacement();
private://private variables



/* EQUIP private */

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;



	/* AMMO private */

		/** Ammo count for this Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		int32 Ammo;
	/** Clip size - max Ammo the Weapon can hold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		int32 MagazineCapacity;
	/** True when moving the clip while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		bool bMovingClip;
	/** Name for the clip bone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		FName ClipBoneName;
	/** Material Index for clearing at weapon type change OnConstruct, otherwise the former dynamic material remains set */
	int32 PreviousMaterialIndex;

	/* SPECS private */

		/** Type of Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		EWeaponType WeaponType;
	/** Type of Ammo used by this Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		EAmmoType AmmoType;
	/** FName for the Reload Montage section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
		FName ReloadMontageSection;
	/** DataTable for weapon properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UDataTable* WeaponDataTable;
	/** Automatic fire speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		float AutoFireRate;

	/* UI Crosshair private */
		/** Weapon Crosshairs Textures */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsMiddle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsBottom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		UTexture2D* CrosshairsTop;

	// EFFECTS private
		/** Particle System spawned at the barrel socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* MuzzleFlash;

	// SOUND private
		/** Sound played when the weapon is fired */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		class USoundCue* FireSound;

	// ANIMATION private
		/** Name of the bone to hi */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
		FName BoneToHide;
	// Pistol animation variables
	// Amount that the pistol slide is pushed back during firing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
		float SlideDisplacement;
	// Curve for the pistol slide displacement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;	
	/** Timer handle for updating SlideDisplacement */
	FTimerHandle SlideTimer;
	/** Time for SlideDisplacement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacementTime;
	/** True when moving pistol slide*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide;
	/** Pistol Slide Displacement in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;
	/** Max rotation for pistol recoil */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxRecoilRotation;
	/* Pistol rotate amount during pistol fire */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
		float RecoilRotation;


public://getters setters

/* EQUIP public */
	/** Adds an impulse to the weapon */
	void ThrowWeapon();

	/* AMMO */
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	/** Call from Character class when firing Weapon */
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	bool ClipIsFull();

	/* SPECS */
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE void  SetReloadMontageSection(FName Name) { ReloadMontageSection = Name; }

	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	
	/* ANIMATION */
	void StartSlideTimer();
};
