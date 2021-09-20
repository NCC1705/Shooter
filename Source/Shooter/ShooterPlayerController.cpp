// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController()
{

}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Check our HUDOverlayClass TSubclassOf variable
	if (HUDOverlayClass)//has been set in blueprints
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);//like spawn actor
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);//HitTestInvisible would not obstruct though
		}
	}
}
