// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Public/HAL/Runnable.h"
// #include "CartPole.h"

//Forward Declaration  // TODO Figure out how to not have forward declarations
class FRunnableThread;
class ACartPole;
/**
 * 
 */

class FNumericalAnalysis : public FRunnable
{

public:
	// Sets default values for this component's properties
	FNumericalAnalysis(ACartPole *ThreadActor);

	~FNumericalAnalysis();

	bool stop_thread;
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual bool get_values(float &current_angle, float &current_position);


protected:
	FCriticalSection UE4_tmux;


private:
	ACartPole *CurrentThreadActor;

	uint32 current_step;
	uint32 calculations;
	uint32 step_count;

	float theta;
	float theta_d;
	float theta_dd;

	float pos;
	float pos_d;
	float pos_dd;

	float current_pos;
	float current_theta;

	float g = 9.81;
	float m_cart;
	float m_pole;
	float l;

};
