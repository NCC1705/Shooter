// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugSound.generated.h"

UCLASS()
class SHOOTER_API ADebugSound : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugSound();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void StartTimer();
	UFUNCTION()
	void AutoTimerReset();//timer callback - must be UFUNCTION
	void PlaySound();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	

/* EFFECTS */

	FTimerHandle SoundRepeatTimer;
	float SoundRepeatRate;
	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	class USoundCue* RepeatSound;



/* DEBUG */

	FDateTime LastFireTime;
};
