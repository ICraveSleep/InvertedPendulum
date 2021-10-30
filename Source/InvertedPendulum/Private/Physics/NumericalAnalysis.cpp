// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/NumericalAnalysis.h"

// Sets default values for this component's properties
FNumericalAnalysis::FNumericalAnalysis(ACartPole *ThreadActor)
{
}

FNumericalAnalysis::~FNumericalAnalysis()
{
}

bool FNumericalAnalysis::Init()
{
	stop_thread = false;
	m_pole = 0.100;  // 0.071
	m_cart = 0.200;  // 0.288
	l = (0.685 - 0.246);
	step_count = 0;
	calculations = 50000;
	theta = 1.57;
	current_theta = theta;
	theta_d = 0;
	theta_dd = 0;

	pos = 0;
	current_pos = pos;
	pos_d = 0;
	pos_dd = 0;

	return true;
}

uint32 FNumericalAnalysis::Run()
{
	float dt = 0.001;
	while (!stop_thread) {
		UE4_tmux.Lock();
		if (step_count <= calculations) {
			theta_dd = -1*((9.81 * sin(theta) + pos_dd * cos(theta)))/l;
			pos_dd = ((-m_pole * l * theta_dd * cos(theta) + m_pole * l * theta_d * theta_d * sin(theta))) / (m_cart + m_pole);

			theta_d = theta_dd * dt + theta_d;
			pos_d = pos_dd * dt + pos_d;

			theta = 0.5*dt*dt*theta_dd + theta_d * dt + theta;
			pos = 0.5*dt*dt*pos_dd + pos_d * dt + pos;

			//current_step = theta;
			current_theta = theta;
			current_pos = pos;
			step_count++;
		}
		else {
			stop_thread = true;
		}
		UE4_tmux.Unlock();
		FPlatformProcess::Sleep(dt);

	}
	return 0;
}

void FNumericalAnalysis::Stop()
{
	stop_thread = true;
}

bool FNumericalAnalysis::get_values(float& current_angle, float& current_position)
{
	UE4_tmux.Lock();
	current_angle = MoveTemp(current_theta);
	current_position = MoveTemp(current_pos);
	UE4_tmux.Unlock();
	return true;
}