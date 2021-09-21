// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSound.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


// FrameRate based (probably) fluctuation when firerate is at 0.1f and GraphicsCard fps is locked at 30 fps; 
// the fluctuation is almost inexistent at 0.05f firerate

// Sets default values
ADebugSound::ADebugSound():
	SoundRepeatRate(0.1f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADebugSound::BeginPlay()
{
	Super::BeginPlay();

	StartTimer();
}

void ADebugSound::StartTimer()
{
	PlaySound();
	GetWorldTimerManager().SetTimer(
		SoundRepeatTimer,
		this,
		&ADebugSound::AutoTimerReset,
		SoundRepeatRate);

	FTimespan TimeSpan = LastFireTime - FDateTime().Now();
	int32 TotalMilliseconds = TimeSpan.GetTotalMilliseconds();
	UE_LOG(LogTemp, Warning, TEXT("elapsed between shots: %d"), TotalMilliseconds);
	LastFireTime = FDateTime().Now();
}

void ADebugSound::AutoTimerReset()
{
	StartTimer();
}

void ADebugSound::PlaySound()
{
	//Play fire sound
	if (RepeatSound)
	{		
		UGameplayStatics::PlaySound2D(this, RepeatSound);
	}
}

// Called every frame
void ADebugSound::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

