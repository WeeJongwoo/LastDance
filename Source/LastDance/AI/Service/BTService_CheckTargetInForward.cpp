// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_CheckTargetInForward.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"

UBTService_CheckTargetInForward::UBTService_CheckTargetInForward()
{
    NodeName = TEXT("CheckTargetInForward");
    Interval = 0.001;
    FOVAngle = 15.0f;
}

void UBTService_CheckTargetInForward::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (AIPawn == nullptr)
    {
        return;
    }

    APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetPlayerPawn")));
    if (Target == nullptr)
    {
        return;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector AIPawntoTarget = (TargetLocation - AIPawn->GetActorLocation()).GetSafeNormal2D();

    FRotator DesiredRotation = (TargetLocation - AIPawn->GetActorLocation()).Rotation();

    float DotProduct = FVector::DotProduct(AIPawn->GetActorForwardVector().GetSafeNormal2D(), AIPawntoTarget);

    float FOVAngleRadian = FMath::DegreesToRadians(FOVAngle);

    if (DotProduct > FMath::Cos(FOVAngleRadian))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("bTargetInForward"), true);
        UE_LOG(LogTemp, Warning, TEXT("Forward"));
        return;
    }

    OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("bTargetInForward"), false);
}
