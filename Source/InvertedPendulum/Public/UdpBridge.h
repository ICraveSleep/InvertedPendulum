// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Serialization/ArrayWriter.h"
#include "UdpBridge.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVERTEDPENDULUM_API UUdpBridge : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUdpBridge();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FIPv4Address _address = {127, 0, 0, 1};
	// FIPv4Address _address;
	uint16 _port = 22001;
	FSocket* _socket;
	
	int32 _buffer_size = 64;
	TSharedPtr<FInternetAddr> socket_addr;  // TODO should be remote_addres
	FArrayWriter Writer;

	// Receiver
	FSocket* _listen_socket;
	FUdpSocketReceiver* _udp_receiver = nullptr;
	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	
};
