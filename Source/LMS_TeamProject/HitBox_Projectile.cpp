// Fill out your copyright notice in the Description page of Project Settings.

#include "HitBox_Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AHitBox_Projectile::AHitBox_Projectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Damage = 10.f;
	InitialSpeed = 3000.f;
	MaxSpeed = 3000.f;
	ArcParam = 0.5f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComponent->InitSphereRadius(15.f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &AHitBox_Projectile::OnHit);
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 0.f;  // LaunchToTarget 호출 전까지 정지
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.f;
}

void AHitBox_Projectile::BeginPlay()
{
	Super::BeginPlay();
}

void AHitBox_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AHitBox_Projectile::LaunchToTarget(FVector TargetLocation)
{
	FVector LaunchVelocity;
	bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		this,
		LaunchVelocity,
		GetActorLocation(),
		TargetLocation,
		0.f,       // 기본 중력 사용
		ArcParam
	);

	if (bSuccess)
	{
		ProjectileMovement->MaxSpeed = LaunchVelocity.Size();
		ProjectileMovement->Velocity = LaunchVelocity;
	}

	return bSuccess;
}

void AHitBox_Projectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}
