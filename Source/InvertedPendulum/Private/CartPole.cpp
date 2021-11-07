// Fill out your copyright notice in the Description page of Project Settings.

#include "CartPole.h"


// Sets default values
ACartPole::ACartPole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//SetVariables
	_address = {127, 0, 0, 1};
	_sender_port = 22002;
	_receiver_port = 22001;
	_buffer_size = 64;

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

	// Initialize writer data
	if (Writer.Num() <= 0){
		// float data = 123.123;
		_save_data = 123.123;
		unsigned char float_bytes[sizeof(_save_data)];
		UE_LOG(LogTemp, Warning, TEXT("Array size is set to: %i"), sizeof(_save_data));
		memcpy(float_bytes, &_save_data, sizeof(_save_data));
		// Writer << data;
		Writer.Add(float_bytes[0]);
		Writer.Add(float_bytes[1]);
		Writer.Add(float_bytes[2]);
		Writer.Add(float_bytes[3]);
	}

	activate_sockets();
}

// Called every frame
void ACartPole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (numerical_thread) {
		numerical_thread->get_values(pole_angle, cart_position);
	}

	// PrintThreadData();
	float time_pass = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	FRotator update = { 0, 0, (pole_angle*180.0f/3.14f) };
	FVector pos_update = { 7.2269f, cart_position * 10.0f , 91.609f};
	revolute_joint->SetRelativeRotation(update);	
	cart_mesh->SetRelativeLocation(pos_update);

	// Convert to send data function -- START

	if (Writer.Num() > 0){
		_save_data = pole_angle;
		unsigned char float_bytes[sizeof(_save_data)];
		// UE_LOG(LogTemp, Warning, TEXT("Array size is set to: %i, at time %f"), sizeof(_save_data), time_pass);
		UE_LOG(LogTemp, Warning, TEXT("Array size is set to: %i"), sizeof(_save_data));
		memcpy(float_bytes, &_save_data, sizeof(_save_data));
		// Writer << data;
		// Writer.Empty();
		Writer[0] = float_bytes[0];
		Writer[1] = float_bytes[1];
		Writer[2] = float_bytes[2];
		Writer[3] = float_bytes[3];
	}
	int32 BytesSent = 0;
	_send_socket->Send(Writer.GetData(), Writer.Num(), BytesSent);
	UE_LOG(LogTemp, Warning, TEXT("Sent bytes: %i, Writer.Num(): %i"), BytesSent, Writer.Num());
	// Convert to send data function -- END

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
	UE_LOG(LogTemp, Warning, TEXT("Processed Calculation: Angle= %f, Pos = %f"), pole_angle, cart_position);	
}

void ACartPole::activate_sockets(){
	
	FIPv4Endpoint _receiver_endpoint(_address, _receiver_port);
	FIPv4Endpoint _sender_endpoint(_address, _sender_port);
	
	socket_addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	socket_addr->SetPort(_sender_endpoint.Port);
	socket_addr->SetIp(_sender_endpoint.Address.Value);
	_send_socket = FUdpSocketBuilder(TEXT("UDPSocket")).AsReusable().AsNonBlocking();
	_send_socket->Bind(*socket_addr);
	bool connected;
	connected = _send_socket->Connect(*socket_addr);
	UE_LOG(LogTemp, Warning, TEXT("Connected: %u"), connected);

	// Receiver
	
		_listen_socket = FUdpSocketBuilder(TEXT("UPD Listener"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(_receiver_endpoint)
		.WithReceiveBufferSize(_buffer_size);
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

	for(int i = 0; i < data_size; i++){
	*ArrayReaderPtr << rec_data[i];  //TODO figure out why this works
	}
	
	for(int i = 0; i < data_size; i++){
		UE_LOG(LogTemp, Warning, TEXT("Received data bytes: %i"), rec_data[i]);
	}
}

void ACartPole::close_sockets(){

	if(_send_socket){
		_send_socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_send_socket);
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
