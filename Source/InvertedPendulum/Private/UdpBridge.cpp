// Fill out your copyright notice in the Description page of Project Settings.


#include "UdpBridge.h"


// Sets default values for this component's properties
UUdpBridge::UUdpBridge()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUdpBridge::BeginPlay()
{
	Super::BeginPlay();

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
	_udp_receiver->OnDataReceived().BindUObject(this, &UUdpBridge::Recv);
	
	_udp_receiver->Start();
	
}


// Called every frame
void UUdpBridge::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUdpBridge::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
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

void UUdpBridge::EndPlay(EEndPlayReason::Type Reason){

	Super::EndPlay(Reason);
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