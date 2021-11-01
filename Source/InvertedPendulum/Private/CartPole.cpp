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
	base_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh")); // Create Object
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
	pole_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole Mesh"));
	pole_mesh->SetStaticMesh(PendulumMesh.Object); // Add the statich mesh PendulumMesh
	pole_mesh->SetupAttachment(revolute_joint);  // Attach it to the revolute joint as "child link"
	pole_mesh->SetRelativeLocation({ 0, 0, 0 }); // Set position in relation to revolute_joint origin [cm].

}

// Called when the game starts or when spawned
void ACartPole::BeginPlay()
{
	Super::BeginPlay();
	cart_mesh->SetRelativeLocation({ 7.2269, 0, 91.609 });  // Set back postikon on begin play
	pole_mesh->SetRelativeLocation({ 0, 0, 0 }); // Set back postikon on begin play

	numerical_thread = new FNumericalAnalysis(this);
	current_running_thread = FRunnableThread::Create(numerical_thread, TEXT("Calculation thread"));

	activate_sockets();
}

// Called every frame
void ACartPole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (numerical_thread) {
		numerical_thread->get_values(pole_angle, cart_position);
	}

	PrintThreadData();
	float time_pass = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	FRotator update = { 0, 0, (pole_angle*180.0f/3.14f) };
	FVector pos_update = { 7.2269f, cart_position * 10.0f , 91.609f};
	revolute_joint->SetRelativeRotation(update);	
	cart_mesh->SetRelativeLocation(pos_update);

}

void ACartPole::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (numerical_thread && current_running_thread) {
		numerical_thread->Stop();
		current_running_thread->WaitForCompletion();
		delete numerical_thread;
	}

	close_sockets();
}

void ACartPole::PrintThreadData()
{
	//numerical_thread->get_value(got_angle, got_pos);
	UE_LOG(LogTemp, Warning, TEXT("Processed Calculation: Angle= %f, Pos = %f"), pole_angle, cart_position);	
}

void ACartPole::activate_sockets(){
		// ...
	// FIPv4Address::Parse(FString("127.0.0.1"), _address);
	uint16 _port2 = 22002;
	FIPv4Endpoint Endpoint(_address, _port);
	FIPv4Endpoint Endpoint2(_address, _port2);
	
	socket_addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	socket_addr->SetPort(Endpoint2.Port);
	socket_addr->SetIp(Endpoint2.Address.Value);
	
	
	_socket = FUdpSocketBuilder(TEXT("UDPSocket")).AsReusable()
													.AsNonBlocking();
													// .BoundToEndpoint(Endpoint)
													// .WithSendBufferSize(_buffer_size).WithBroadcast();

	int32 SendSize = 64;
	// _socket->SetReceiveBufferSize(SendSize,SendSize);

	_socket->Bind(*socket_addr);

	FTimespan thread_wait_time = FTimespan::FromMilliseconds(500);


	// TArray<uint8_t> data;
	// for(int i = 1; i < 60; i++){
	// 	data.Add(i);
	// }
	
	// uint8 data = 8;
	float data = 123.123;
	
	Writer << data;
	int32 BytesSent = 0;
	bool connected;

	connected = _socket->Connect(*socket_addr);
	UE_LOG(LogTemp, Warning, TEXT("Connected: %u"), connected);


	FString serialized = TEXT("Hello from from UE4 UDP client");
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	// int32 size_len = FCString::Strlen(serializedChar);
	const char* send_string = TCHAR_TO_UTF8(serializedChar);
	int32 size_len = strlen(send_string);
	_socket->Send(Writer.GetData(), Writer.Num(), BytesSent);
	// _socket->Send((uint8*)send_string, size_len, BytesSent);

	UE_LOG(LogTemp, Warning, TEXT("Sent data: %i"), BytesSent);

	// Receiver
	
		_listen_socket = FUdpSocketBuilder(TEXT("UPD Listener"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(SendSize);
	;
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(10);
	_udp_receiver = new FUdpSocketReceiver(_listen_socket, ThreadWaitTime, TEXT("UDP RECEIVER"));
	_udp_receiver->OnDataReceived().BindUObject(this, &ACartPole::Recv);
	
	_udp_receiver->Start();
}

void ACartPole::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	UE_LOG(LogTemp, Warning, TEXT("Received data bytes: %i"), ArrayReaderPtr->Num());
	
	uint8 rec_data[64] = {};
	int32 data_size = ArrayReaderPtr->Num();
	int32 offset = 0;
	// *ArrayReaderPtr << rec_data[0];

	for(int i = 0; i < data_size; i++){
	*ArrayReaderPtr << rec_data[i];  //TODO figure out this
	}
	
	for(int i = 0; i < data_size; i++){
		UE_LOG(LogTemp, Warning, TEXT("Received data bytes: %i"), rec_data[i]);
	}
}

void ACartPole::close_sockets(){
		if(_socket){
		_socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_socket);
		UE_LOG(LogTemp, Warning, TEXT("EndPlay called, socket closed"));
	}

	

	_udp_receiver->Stop();
	delete _udp_receiver;
	_udp_receiver = nullptr;
	
	//Clear all sockets!
	//		makes sure repeat plays in Editor dont hold on to old sockets!
	if(_listen_socket)
	{
		_listen_socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_listen_socket);
		UE_LOG(LogTemp, Warning, TEXT("EndPlay called, _listen_socket closed"));
	}
}
