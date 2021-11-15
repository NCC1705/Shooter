// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_Attack.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UBTT_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTT_Attack(FObjectInitializer const& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;

private:
	bool GetMontageFinished(class AEnemy* const Enemy);
};
