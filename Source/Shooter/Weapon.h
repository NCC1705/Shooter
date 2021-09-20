// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

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

public://getters setters

/* EQUIP */

	/** Adds an impulse to the weapon */
	void ThrowWeapon();
};
