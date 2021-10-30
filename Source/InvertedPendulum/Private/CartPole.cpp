// Fill out your copyright notice in the Description page of Project Settings.

#include "CartPole.h"


// Sets default values
ACartPole::ACartPole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Get the Meshes 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMesh(TEXT("/Game/Models/Base/inverted_pendulum_base.inverted_pendulum_base"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CartMesh(TEXT("/Game/Models/Cart/inverted_pendulum_cart.inverted_pendulum_cart"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PendulumMesh(TEXT("/Game/Models/Pole/inverted_pendulum_pendulum.inverted_pendulum_pendulum"));

	// Add the Meshes when the Actor (CartPole) is spawned to the StaticMeshComponents
	// Base 
	base_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CartPoleMesh")); // Create Object
	base_mesh->SetStaticMesh(BaseMesh.Object); // Add the static mesh BaseMesh
	base_mesh->SetupAttachment(RootComponent); // Attach it to the RootComponent (Make it the "base link")
	// TODO add prismatic joint
	// Cart
	cart_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cart Mesh"));
	cart_mesh->SetStaticMesh(CartMesh.Object); // Add the static mesh CartMesh
	cart_mesh->SetupAttachment(base_mesh); // Attach it to the base component
	cart_mesh->SetRelativeLocation({ 7.2269, 0, 91.609 });  // Set position in relation to base_mesh origin [cm].
	// Create a dummy link to act as a revolute joint between the cart_mesh and the pole_mesh
	revolute_joint = CreateDefaultSubobject<USceneComponent>(TEXT("This is a joint"));
	revolute_joint->SetupAttachment(cart_mesh); // Make the cart the "parent link"
	// Pole
	pole_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ARM"));
	pole_mesh->SetStaticMesh(PendulumMesh.Object); // Add the statich mesh PendulumMesh
	pole_mesh->SetupAttachment(revolute_joint);  // Attach it to the revolute joint as "child link"
	pole_mesh->SetRelativeLocation({ 0, 0, 0 }); // Set position in relation to revolute_joint origin [cm].

}

// Called when the game starts or when spawned
void ACartPole::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACartPole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

