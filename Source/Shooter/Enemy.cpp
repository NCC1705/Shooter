// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
AEnemy::AEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bCanHitReact(true),
	HitReactTimeMin(.5f),
	HitReactTimeMax(1.f),
	HitNumberDestroyTime(1.5f),
	bStunned(false),
	StunChance(.5f),
	Attack_LFast(TEXT("Attack_LFast")),
	Attack_RFast(TEXT("Attack_RFast")),
	Attack_L(TEXT("Attack_L")),
	Attack_R(TEXT("Attack_R")),
	BaseDamage(20.f),
	LeftWeaponSocket(TEXT("FX_Trail_L_01")),
	RightWeaponSocket(TEXT("FX_Trail_R_01")),
	bCanAttack(true),
	AttackWaitTime(1.f),
	bDying(false),
	DeathTime(4.f)

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create the AgroSphere
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	// create the CombatRangeSphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	//construct left and right collision boxes
	WeaponLeftCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponLeftCollision"));
	WeaponLeftCollision->SetupAttachment(GetMesh(), FName("WeaponLeftSocket"));

	WeaponRightCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRightCollision"));
	WeaponRightCollision->SetupAttachment(GetMesh(), FName("WeaponRightSocket"));


}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAgroSphereOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeEndOverlap);

	//Bind functions to overlap events for weapon boxes
	WeaponLeftCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnWeaponLeftOverlap);
	WeaponRightCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnWeaponRightOverlap);
	//Set collision presets for weapon boxes
	WeaponLeftCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponLeftCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	WeaponLeftCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponLeftCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	WeaponRightCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponRightCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	WeaponRightCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponRightCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);



	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);


	//Get the AI controller
	EnemyController = Cast<AEnemyController>(GetController());
	//converts from local/relative to world space
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);

	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Red, true);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}		
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(
		HealthBarTimer, 
		this, 
		&AEnemy::HideHealthBar, 
		HealthBarDisplayTime);
}

void AEnemy::Die()
{
	if (bDying) return;//already dying, abort
	bDying = true;//first time this is called

	HideHealthBar();
	//PlayHitMontage(rand() % 2 == 0 ? FName("Death_A") : FName("Death_B"));
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		EnemyController->StopMovement();
	}
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{

	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && HitMontage)//HitMontage check added by me
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		bCanHitReact = false;
		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin,HitReactTimeMax) };
		GetWorldTimerManager().SetTimer(
			HitReactTimer, 
			this, 
			&AEnemy::ResetHitReactTimer, 
			HitReactTime);
	}

}

void AEnemy::ResetHitReactTimer()
{	
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);
	
	FTimerHandle HitNumberTimer;//not member variable to set a timer for each number
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	GetWorld()->GetTimerManager().SetTimer(
		HitNumberTimer, 
		HitNumberDelegate, 
		HitNumberDestroyTime, 
		false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);//removes it from the map
	HitNumber->RemoveFromParent();//removes it from the viewport
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{ HitPair.Key };
		const FVector Location{ HitPair.Value };
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(
			GetWorld()->GetFirstPlayerController(),
			Location,
			ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}

}

void AEnemy::OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;//assymetry if null return vs if(otheractor)

	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		//set the value of the target blackboard key
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), ShooterCharacter);
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}	
}

void AEnemy::OnCombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}
}

void AEnemy::OnCombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;

	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	//Attack_LFast, Attack_RFast, Attack_L, Attack_R;
	FName SectionName;
	const int32 Section{ FMath::RandRange(0,3) };
	switch (Section)
	{

	case 0:
		SectionName = Attack_LFast;
			break;
	case 1:
		SectionName = Attack_RFast;
			break;
	case 2:
		SectionName = Attack_L;
			break;
	case 3:
		SectionName = Attack_R;
			break;
	default:
		SectionName = Attack_LFast;
		break;
	}
	return SectionName;

	//return FName();
}

void AEnemy::DoDamage(AShooterCharacter* Character)//(AActor* OtherActor)
{
	//if (Character == nullptr) return;//OtherActor
	//auto Character = Cast<AShooterCharacter>(OtherActor);
	//if (Character) {}

	UGameplayStatics::ApplyDamage(
		Character,
		BaseDamage,
		EnemyController,
		this,//damage causer
		UDamageType::StaticClass()
	);
	if (Character->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Character->GetMeleeImpactSound(), GetActorLocation());
	}
}

void AEnemy::SpawnBlood(AShooterCharacter* Character, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };
	if (TipSocket)
	{
		const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
		if (Character->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Character->GetBloodParticles(), SocketTransform);
		}
	}
}

void AEnemy::OnWeaponLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character) 
	{ 
		DoDamage(Character); 
		SpawnBlood(Character, LeftWeaponSocket);	
		StunCharacter(Character);
	}
	//DoDamage(OtherActor);
}

void AEnemy::OnWeaponRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character) 
	{ 
		DoDamage(Character);
		SpawnBlood(Character, RightWeaponSocket);
		StunCharacter(Character);
	}
	//DoDamage(OtherActor);
}

void AEnemy::ActivateLeftWeapon()//called from blueprints anim notify
{
	//QueryOnly generates overlap events but we dont need to collide/physics
	WeaponLeftCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()//called from blueprints anim notify
{	
	WeaponLeftCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()//called from blueprints anim notify
{
	//QueryOnly generates overlap events but we dont need to collide/physics
	WeaponRightCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()//called from blueprints anim notify
{
	WeaponRightCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::StunCharacter(AShooterCharacter* Character)
{
	if (Character)
	{
		const float StunProbability{ FMath::FRandRange(0.f,1.f) };
		if (StunProbability <= Character->GetStunChance())\
		{
			Character->Stun();
		}
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;//I think this is wrong; try removing duplicate variable and use the blackboard, having 2 might create uncertainties
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	//Destroy();
	GetMesh()->bPauseAnims = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}



// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}

	if (bDying) return;//already dying, abort

	ShowHealthBar();

	//ddtermine if bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// stun the enemy		
		PlayHitMontage(FName("HitReact_Front"));
		SetStunned(true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//Set Target blackboard key to agro character
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	}

	if (Health - DamageAmount <= 0.f)
	{
		Health = 0;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}
		
	return DamageAmount; //0.0f;
}




// C++ AI

void AEnemy::PlayAttackMontageC()
{
	PlayAttackMontage(GetAttackSectionName());
}

