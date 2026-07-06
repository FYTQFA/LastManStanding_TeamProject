#include "LMSWeaponComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "LMSWeaponBase.h"
#include "LMSWeaponPrimaryAbility.h"
#include "LMSWeaponSecondaryAbility.h"
#include "TimerManager.h"
#include "../LMSGameplayAbility.h"

ULMSWeaponComponent::ULMSWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULMSWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!DefaultWeaponRowName.IsNone())
	{
		EquipWeaponByRowName(DefaultWeaponRowName);
	}
}

bool ULMSWeaponComponent::EquipWeaponFromData(const FWeaponData& WeaponData)
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !WeaponData.WeaponClass)
	{
		return false;
	}

	UnequipCurrentWeapon();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.Instigator = OwnerCharacter;

	CurrentWeapon = GetWorld()->SpawnActor<ALMSWeaponBase>(WeaponData.WeaponClass, SpawnParams);
	if (!CurrentWeapon)
	{
		return false;
	}

	CurrentWeaponData = WeaponData;
	CurrentWeapon->SetWeaponData(WeaponData);
	CurrentWeapon->Equip(OwnerCharacter, EquippedSocketName);

	AmmoInMagazine = CurrentWeaponData.MagazineSize;
	ReserveAmmo = CurrentWeaponData.MaxReserveAmmo;
	bIsReloading = false;
	bIsBlocking = false;
	bIsAiming = false;

	GrantCurrentWeaponAbilities();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Equipped weapon: %s GrantedAbilities=%d WeaponSkill=%s"),
		*CurrentWeaponData.WeaponID.ToString(),
		CurrentWeaponData.GrantedAbilities.Num(),
		*GetNameSafe(CurrentWeaponData.WeaponSkill));

	return true;
}

bool ULMSWeaponComponent::EquipWeaponByRowName(FName RowName)
{
	return EquipWeaponByID(RowName);
}

bool ULMSWeaponComponent::EquipWeaponByID(FName WeaponID)
{
	if (!WeaponDataTable || WeaponID.IsNone())
	{
		return false;
	}

	if (const FWeaponData* WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponID, TEXT("EquipWeaponByID"), false))
	{
		return EquipWeaponFromData(*WeaponData);
	}

	TArray<FWeaponData*> WeaponRows;
	WeaponDataTable->GetAllRows<FWeaponData>(TEXT("EquipWeaponByID"), WeaponRows);

	for (const FWeaponData* WeaponData : WeaponRows)
	{
		if (WeaponData && WeaponData->WeaponID == WeaponID)
		{
			return EquipWeaponFromData(*WeaponData);
		}
	}

	return false;
}

void ULMSWeaponComponent::UnequipCurrentWeapon()
{
	ClearGrantedWeaponAbilities();

	if (CurrentWeapon)
	{
		CurrentWeapon->Unequip();
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	CurrentWeaponData = FWeaponData();
	AmmoInMagazine = 0;
	ReserveAmmo = 0;
	bIsReloading = false;
	bIsBlocking = false;
	bIsAiming = false;
}

void ULMSWeaponComponent::StartAttack()
{
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartAttack ignored because CurrentWeapon is null."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("StartAttack: %s Type=%d"), *CurrentWeaponData.WeaponID.ToString(), static_cast<int32>(CurrentWeaponData.WeaponType));

	switch (CurrentWeaponData.WeaponType)
	{
	case ELMSWeaponType::Melee:
		StartMeleeAttack();
		break;
	case ELMSWeaponType::Ranged:
		StartRangedAttack();
		break;
	default:
		break;
	}
}

void ULMSWeaponComponent::StopAttack()
{
	UE_LOG(LogTemp, Log, TEXT("StopAttack: %s"), *CurrentWeaponData.WeaponID.ToString());
}

void ULMSWeaponComponent::StartSecondaryAction()
{
	if (!CurrentWeapon)
	{
		return;
	}

	switch (CurrentWeaponData.WeaponType)
	{
	case ELMSWeaponType::Melee:
		StartBlock();
		break;
	case ELMSWeaponType::Ranged:
		StartAim();
		break;
	default:
		break;
	}
}

void ULMSWeaponComponent::StopSecondaryAction()
{
	StopBlock();
	StopAim();
}

void ULMSWeaponComponent::Reload()
{
	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;
	UE_LOG(LogTemp, Log, TEXT("Reload started: %s"), *CurrentWeaponData.WeaponID.ToString());

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReloadTimerHandle, this, &ULMSWeaponComponent::FinishReload, CurrentWeaponData.ReloadTime, false);
	}
}

void ULMSWeaponComponent::RefreshGrantedAbilities()
{
	if (CurrentWeapon)
	{
		GrantCurrentWeaponAbilities();
	}
}

ACharacter* ULMSWeaponComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UAbilitySystemComponent* ULMSWeaponComponent::GetOwnerAbilitySystemComponent() const
{
	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	return AbilitySystemOwner ? AbilitySystemOwner->GetAbilitySystemComponent() : nullptr;
}

void ULMSWeaponComponent::GrantCurrentWeaponAbilities()
{
	ClearGrantedWeaponAbilities();

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : CurrentWeaponData.GrantedAbilities)
	{
		GrantWeaponAbility(AbilityClass);
	}

	GrantWeaponAbility(CurrentWeaponData.WeaponSkill);
}

void ULMSWeaponComponent::ClearGrantedWeaponAbilities()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	if (!AbilitySystemComponent || GrantedAbilityHandles.IsEmpty())
	{
		GrantedAbilityHandles.Reset();
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : GrantedAbilityHandles)
	{
		if (AbilityHandle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(AbilityHandle);
		}
	}

	GrantedAbilityHandles.Reset();
}

void ULMSWeaponComponent::GrantWeaponAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	if (!OwnerActor || !OwnerActor->HasAuthority() || !AbilitySystemComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GrantWeaponAbility failed. Owner=%s HasAuthority=%d ASC=%s AbilityClass=%s"),
			*GetNameSafe(OwnerActor),
			OwnerActor ? OwnerActor->HasAuthority() : false,
			*GetNameSafe(AbilitySystemComponent),
			*GetNameSafe(AbilityClass));
		return;
	}

	const ULMSGameplayAbility* AbilityCDO = Cast<ULMSGameplayAbility>(AbilityClass->GetDefaultObject());
	const int32 InputID = AbilityCDO ? static_cast<int32>(AbilityCDO->AbilityInputID) : INDEX_NONE;
	int32 ResolvedInputID = InputID;

	if (Cast<ULMSWeaponPrimaryAbility>(AbilityCDO))
	{
		ResolvedInputID = static_cast<int32>(ELMSAbilityInputID::PrimaryAttack);
	}
	else if (Cast<ULMSWeaponSecondaryAbility>(AbilityCDO))
	{
		ResolvedInputID = static_cast<int32>(ELMSAbilityInputID::SecondaryAttack);
	}

	FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, ResolvedInputID, CurrentWeapon);
	const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	if (AbilityHandle.IsValid())
	{
		GrantedAbilityHandles.Add(AbilityHandle);
		UE_LOG(LogTemp, Log, TEXT("Granted weapon ability: %s InputID=%d"), *GetNameSafe(AbilityClass), ResolvedInputID);
	}
}

void ULMSWeaponComponent::StartMeleeAttack()
{
	if (bIsReloading || bIsBlocking)
	{
		return;
	}

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (!OwnerCharacter || !World)
	{
		return;
	}

	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const FVector Start = OwnerCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f) + Forward * 50.f;
	const FVector End = Start + Forward * CurrentWeaponData.Range;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LMSMeleeAttack), false);
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.AddIgnoredActor(CurrentWeapon);

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(MeleeTraceRadius),
		QueryParams);

	TSet<AActor*> DamagedActors;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerCharacter || HitActor == CurrentWeapon || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		DamagedActors.Add(HitActor);

		if (OwnerCharacter->HasAuthority())
		{
			UGameplayStatics::ApplyDamage(
				HitActor,
				CurrentWeaponData.Damage,
				OwnerCharacter->GetController(),
				CurrentWeapon ? Cast<AActor>(CurrentWeapon) : Cast<AActor>(OwnerCharacter),
				UDamageType::StaticClass());
		}
	}

	if (bDrawDebugMeleeTrace)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, Start, End, DebugColor, false, 1.5f, 0, 2.f);
		DrawDebugSphere(World, End, MeleeTraceRadius, 16, DebugColor, false, 1.5f);

		for (const FHitResult& Hit : Hits)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 16.f, 8, FColor::Yellow, false, 1.5f);
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Melee attack: %s Hits=%d Damage=%.1f"),
		*CurrentWeaponData.WeaponID.ToString(),
		DamagedActors.Num(),
		CurrentWeaponData.Damage);
}

void ULMSWeaponComponent::StartRangedAttack()
{
	if (bIsReloading)
	{
		return;
	}

	if (AmmoInMagazine <= 0)
	{
		Reload();
		return;
	}

	--AmmoInMagazine;
	const FTransform AttackOrigin = CurrentWeapon ? CurrentWeapon->GetAttackOriginTransform() : FTransform::Identity;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Ranged attack: %s Ammo=%d/%d Origin=%s"),
		*CurrentWeaponData.WeaponID.ToString(),
		AmmoInMagazine,
		ReserveAmmo,
		*AttackOrigin.GetLocation().ToString());
}

void ULMSWeaponComponent::StartBlock()
{
	if (CurrentWeaponData.WeaponType != ELMSWeaponType::Melee || !CurrentWeaponData.bCanBlock || bIsReloading)
	{
		return;
	}

	bIsBlocking = true;
	UE_LOG(LogTemp, Log, TEXT("Block started: %s"), *CurrentWeaponData.WeaponID.ToString());
}

void ULMSWeaponComponent::StopBlock()
{
	if (!bIsBlocking)
	{
		return;
	}

	bIsBlocking = false;
	UE_LOG(LogTemp, Log, TEXT("Block stopped: %s"), *CurrentWeaponData.WeaponID.ToString());
}

void ULMSWeaponComponent::StartAim()
{
	if (CurrentWeaponData.WeaponType != ELMSWeaponType::Ranged || !CurrentWeaponData.bCanAim || bIsReloading)
	{
		return;
	}

	bIsAiming = true;
	UE_LOG(LogTemp, Log, TEXT("Aim started: %s"), *CurrentWeaponData.WeaponID.ToString());
}

void ULMSWeaponComponent::StopAim()
{
	if (!bIsAiming)
	{
		return;
	}

	bIsAiming = false;
	UE_LOG(LogTemp, Log, TEXT("Aim stopped: %s"), *CurrentWeaponData.WeaponID.ToString());
}

bool ULMSWeaponComponent::CanReload() const
{
	return CurrentWeapon
		&& CurrentWeaponData.WeaponType == ELMSWeaponType::Ranged
		&& !bIsReloading
		&& !bIsAiming
		&& CurrentWeaponData.MagazineSize > 0
		&& AmmoInMagazine < CurrentWeaponData.MagazineSize
		&& ReserveAmmo > 0;
}

void ULMSWeaponComponent::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}

	const int32 NeededAmmo = CurrentWeaponData.MagazineSize - AmmoInMagazine;
	const int32 AmmoToLoad = FMath::Min(NeededAmmo, ReserveAmmo);

	AmmoInMagazine += AmmoToLoad;
	ReserveAmmo -= AmmoToLoad;
	bIsReloading = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Reload finished: %s Ammo=%d/%d"),
		*CurrentWeaponData.WeaponID.ToString(),
		AmmoInMagazine,
		ReserveAmmo);
}
