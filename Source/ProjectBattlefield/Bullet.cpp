// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "ActorInteractionInterface.h"
#include "Kismet/GameplayStatics.h"

// to do: add ability bar (points) to character or something
// to do: add validations to special ability action
// to do: add validations to fire gun action
// to do: think the things better.

ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;
	
	capsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	staticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	projectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	capsuleComponent->SetupAttachment(GetRootComponent());
	staticMeshComponent->SetupAttachment(capsuleComponent);

	capsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnCapsuleBeginOverlap);
	capsuleComponent->OnComponentHit.AddDynamic(this, &ABullet::OnCapsuleHit);


	SetCanBeDamaged(false);

	capsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	capsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	capsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	capsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	capsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	capsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	damage = 50.f;
}

UProjectileMovementComponent* ABullet::GetProjetileMovementComponent()
{
	return projectileMovementComponent;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(bulletDestructionTimerHandle, this, &ABullet::destroyBullet, 5.f, false); // to do: Add a custom destruction time (parameter) to destroy it in the ebst moment
}

void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABullet::StartDestroyProcess(AActor* OtherActor)
{
	// to do: focus this method's functionality and create another method to do somethings from here.
	if (OtherActor != GetInstigator())
	{
		if (OtherActor->CanBeDamaged()) UGameplayStatics::ApplyDamage(OtherActor, damage, GetInstigator()->GetController(), this, damageType);
		DrawDebugSphere(GetWorld(), GetActorLocation(), 10, 25, FColor::Blue, false, 5.f); // debug
		capsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorHiddenInGame(true);
		GetWorldTimerManager().ClearTimer(bulletDestructionTimerHandle);
		GetWorldTimerManager().SetTimer(bulletDestructionTimerHandle, this, &ABullet::destroyBullet, 1.f, false);
		// to do: set a parameter to change the destruction time
	}
}

void ABullet::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	StartDestroyProcess(OtherActor);
}

void ABullet::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	StartDestroyProcess(OtherActor);
}

void ABullet::destroyBullet()
{
	Destroy();
}
// to do: display a DECAL when hit something (this is for the bullet mark)	
