// Fill out your copyright notice in the Description page of Project Settings.


#include "World/LDBossTrigger.h"
#include "Components/BoxComponent.h"
#include "Character/LDBossCharacter.h"
#include "Character/LDPlayerCharacter.h"
#include "Player/LDPlayerController.h"
#include "Engine/NetConnection.h"
#include "Engine/ActorChannel.h"
#include "Log/LDLog.h"


// Sets default values
ALDBossTrigger::ALDBossTrigger()
{
	bReplicates = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void ALDBossTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ALDBossTrigger::OnTriggerBeginOverlap);
}

void ALDBossTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || State == EBattleState::Cooldown)
    {
        return;
    }

    ALDPlayerCharacter* PlayerChar = Cast<ALDPlayerCharacter>(OtherActor);
    if (!PlayerChar)
    {
        return;
    }

    ALDPlayerController* PC = Cast<ALDPlayerController>(PlayerChar->GetController());
    if (!PC)
    {
        return;
    }

    if (State == EBattleState::Idle)
    {
        SpawnBoss();
    }

    if (ActiveBoss)
    {
		TryRegisterWhenReplicated(ActiveBoss, PC, 0);
    }
}

void ALDBossTrigger::SpawnBoss()
{
    if (!BossClass)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ActiveBoss = GetWorld()->SpawnActor<ALDBossCharacter>(BossClass, BossSpawnTransform, Params);

    if (!ActiveBoss)
    {
        return;
    }

    State = EBattleState::Active;

	ActiveBoss->OnDeath.AddUObject(this, &ALDBossTrigger::BossDeath);
}

void ALDBossTrigger::BossDeath(ALDBaseCharacter* DeadCharacter)
{
    if (!HasAuthority())
    {
        return;
    }

    State = EBattleState::Cooldown;
    ActiveBoss = nullptr;

    GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ALDBossTrigger::EndCooldown, ResetCooldown, false);
}

void ALDBossTrigger::TryRegisterWhenReplicated(TWeakObjectPtr<ALDBossCharacter> WeakBoss, TWeakObjectPtr<ALDPlayerController> WeakPC, int32 RetryCount)
{
    ALDBossCharacter* Boss = WeakBoss.Get();
    ALDPlayerController* PC = WeakPC.Get();
    if (!Boss || !PC) return;

    UNetConnection* Conn = PC->GetNetConnection();
    if (!Conn)
    {
        // Standalone/리슨 — 즉시 등록 가능
        Boss->RegisterController(PC);
        return;
    }

    // 이 보스가 PC의 연결에 ActorChannel을 가졌는지 = 복제 시작됨
    UActorChannel** Channel = Conn->FindActorChannel(Boss);
    if (Channel && (*Channel)->OpenAcked)   // OpenAcked: 클라가 채널 오픈 ack 보냄
    {
        Boss->RegisterController(PC);
        return;
    }

    // 아직 — 짧은 간격으로 재시도, 최대 N회까지
    if (RetryCount > 60)   // 약 1초 (60 * 16ms)
    {
        LD_LOG(LDLog, Warning, TEXT("Boss replication timed out for PC %s"), *GetNameSafe(PC));
        return;
    }

    GetWorldTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &ALDBossTrigger::TryRegisterWhenReplicated,
            WeakBoss, WeakPC, RetryCount + 1));
}

void ALDBossTrigger::EndCooldown()
{
    State = EBattleState::Idle;
}



