// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Attack.h"
#include "Enemy.h"
#include "AIController.h"

UBTT_Attack::UBTT_Attack(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("BTT_Attack_C++");//the title that appears on the node in the Behavior Tree
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	if (AAIController* const AIController = OwnerComponent.GetAIOwner())// get the NPC
	{		
		if (AEnemy* const Enemy = Cast<AEnemy>(AIController->GetPawn()))
		{		
			if (GetMontageFinished(Enemy))
			{
				Enemy->PlayAttackMontageC();
			}
			
		}
		// finish with success
		FinishLatentTask(OwnerComponent, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

bool UBTT_Attack::GetMontageFinished(AEnemy* const Enemy)
{
	return Enemy->GetMesh()->GetAnimInstance()->Montage_GetIsStopped(Enemy->GetAttackMontage());
}
