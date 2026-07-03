#include "LMSWeaponComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "LMSWeaponBase.h"
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
		return;
	}

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
	AActor* OwnerActor = GetOwner();
	UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	if (!OwnerActor || !OwnerActor->HasAuthority() || !AbilitySystemComponent || !AbilityClass)
	{
		return;
	}

	const ULMSGameplayAbility* AbilityCDO = Cast<ULMSGameplayAbility>(AbilityClass->GetDefaultObject());
	const int32 InputID = AbilityCDO ? static_cast<int32>(AbilityCDO->AbilityInputID) : INDEX_NONE;

	FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, InputID, CurrentWeapon);
	const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	if (AbilityHandle.IsValid())
	{
		GrantedAbilityHandles.Add(AbilityHandle);
	}
}

void ULMSWeaponComponent::StartMeleeAttack()
{
	if (bIsReloading || bIsBlocking)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Melee attack: %s"), *CurrentWeaponData.WeaponID.ToString());
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
