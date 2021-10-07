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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)//Set in BaseWeaponBP, invarialbe for all weapons
	//class UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;
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
	virtual void OnConstruction(const FTransform& Transform) override;

private://private variables

	

/* EQUIP */

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;



/* AMMO */

	/** Ammo count for this Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
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
	/** DataTable for weapon properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;



public://getters setters

/* EQUIP */

	/** Adds an impulse to the weapon */
	void ThrowWeapon();



/* AMMO */

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	/** Call from Character class when firing Weapon */
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	bool ClipIsFull();

/* SPECS */
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }	
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
};
