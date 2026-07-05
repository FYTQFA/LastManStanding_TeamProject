// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_ProjectileLaunch.h"
#include "HitBox_Projectile.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"

UGA_ProjectileLaunch::UGA_ProjectileLaunch()
{
	MuzzleSocketName = TEXT("hand_r");
}

void UGA_ProjectileLaunch::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 발사 위치 / 방향 결정
	FVector SpawnLocation = GetMuzzleLocation(ActorInfo);
	FVector TargetLocation = GetTargetLocation(ActorInfo);

	// 2. 발사체 Spawn (충돌 무시 후 위치 확정)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwningActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(GetOwningActorFromActorInfo());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	AHitBox_Projectile* Projectile = World->SpawnActor<AHitBox_Projectile>(
		ProjectileClass, SpawnTransform, SpawnParams);

	if (!Projectile)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 포물선 발사 — 실패 시 직선 발사로 폴백
	bool bLaunched = Projectile->LaunchToTarget(TargetLocation);
	if (!bLaunched)
	{
		FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
		Projectile->GetProjectileMovement()->Velocity = Direction * Projectile->InitialSpeed;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

FVector UGA_ProjectileLaunch::GetMuzzleLocation(const FGameplayAbilityActorInfo* ActorInfo) const
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (OwnerCharacter && OwnerCharacter->GetMesh() &&
		OwnerCharacter->GetMesh()->DoesSocketExist(MuzzleSocketName))
	{
		return OwnerCharacter->GetMesh()->GetSocketLocation(MuzzleSocketName);
	}

	return ActorInfo->AvatarActor.IsValid()
		? ActorInfo->AvatarActor->GetActorLocation() + FVector(0, 0, 50.f)
		: FVector::ZeroVector;
}

FVector UGA_ProjectileLaunch::GetTargetLocation(const FGameplayAbilityActorInfo* ActorInfo) const
{
	// TargetActor가 지정되어 있으면 우선 사용
	if (TargetActor)
	{
		return TargetActor->GetActorLocation();
	}

	// 플레이어 컨트롤러의 카메라 전방 2000 유닛 지점 사용
	APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get());
	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		return CamLoc + CamRot.Vector() * 2000.f;
	}

	// 폴백: 아바타 전방
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	return Avatar
		? Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 2000.f
		: FVector::ZeroVector;
}
