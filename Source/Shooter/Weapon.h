// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	EWT_SubmachineGun UMETA(DisplayName ="SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
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

/* EQUIP */

	void StopFalling();//get into pickup state

private://private variables

	

/* EQUIP */

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;



/* AMMO */

	/** Ammo count for this Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	int32 Ammo;



/* SPECS */
	/** Type of Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;
	/** Type of Ammo used by this Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	/** FName for the Reload Montage section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

public://getters setters

/* EQUIP */

	/** Adds an impulse to the weapon */
	void ThrowWeapon();



/* AMMO */

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	/** Call from Character class when firing Weapon */
	void DecrementAmmo();



/* SPECS */
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
};
