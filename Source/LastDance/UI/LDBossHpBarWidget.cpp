// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LDBossHpBarWidget.h"
#include "Character/LDBossCharacter.h"
#include "Component/LDStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Log/LDLog.h"

void ULDBossHpBarWidget::NativeDestruct()
{
	Unbind();
	Super::NativeDestruct();
}

void ULDBossHpBarWidget::BindToBoss(ALDBossCharacter* Boss)
{
	if (!Boss)
	{
		return;
	}

	// 다른 보스가 이미 바인딩돼 있다면 먼저 해제
	if (BoundBoss.IsValid() && BoundBoss.Get() != Boss)
	{
		Unbind();
	}

	auto* Stat = Boss->GetStatComponent();
	if (!Stat)
	{
		return;
	}

	BoundBoss = Boss;

	// 중복 바인딩 방지 — 동일 위젯이 같은 보스에 두 번 바인딩되는 케이스
	Stat->OnCurrentHPChanged.RemoveDynamic(this, &ULDBossHpBarWidget::HandleHPChanged);
	Stat->OnCurrentHPChanged.AddDynamic(this, &ULDBossHpBarWidget::HandleHPChanged);

	Stat->OnDeath.RemoveDynamic(this, &ULDBossHpBarWidget::HandleBossDeath);
	Stat->OnDeath.AddDynamic(this, &ULDBossHpBarWidget::HandleBossDeath);

	// 첫 표시 — 현재 값으로 즉시 갱신
	HandleHPChanged(Stat->GetCurrentHP(), Stat->GetMaxHP());

	if (BossName)
	{
		// 보스 표시 이름. 임시로 액터 이름. 나중에 데이터 에셋에서 끌어와도 좋음.
		BossName->SetText(FText::FromString(Boss->GetName()));
	}
}

void ULDBossHpBarWidget::Unbind()
{
	if (auto* Boss = BoundBoss.Get())
	{
		if (auto* Stat = Boss->GetStatComponent())
		{
			Stat->OnCurrentHPChanged.RemoveDynamic(this, &ULDBossHpBarWidget::HandleHPChanged);
			Stat->OnDeath.RemoveDynamic(this, &ULDBossHpBarWidget::HandleBossDeath);
		}
	}

	BoundBoss = nullptr;
}

void ULDBossHpBarWidget::HandleHPChanged(float CurrentHP, float MaxHP)
{
	if (BossHpBar && MaxHP > 0.f)
	{
		BossHpBar->SetPercent(CurrentHP / MaxHP);
	}
}

void ULDBossHpBarWidget::HandleBossDeath()
{
	if (BossHpBar)
	{
		BossHpBar->SetPercent(0.f);
	}
}
