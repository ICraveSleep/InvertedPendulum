// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// ADDED START //
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
// ADDED END //
#include "CartPole.generated.h"

UCLASS()
class INVERTEDPENDULUM_API ACartPole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACartPole();

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "CartPole")
	UStaticMeshComponent* base_mesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "CartPole")
	UStaticMeshComponent* pole_mesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "CartPole")
		UStaticMeshComponent* cart_mesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "CartPole")
		USceneComponent* revolute_joint;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	float pole_angle;
	float cart_position;
	
	
};
