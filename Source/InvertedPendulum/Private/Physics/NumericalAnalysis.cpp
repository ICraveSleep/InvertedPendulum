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
	theta = 3.12;
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
	float dt = 0.00015; // [s]  Have worked with 0.0001
	uint16 step_count = 0;
	double loop_period = 0;     // [s]
	double step_period = 0;     // [s]
	std::chrono::steady_clock::time_point step_start_time; // = loop_start_time
	std::chrono::steady_clock::time_point step_end_time;
	std::chrono::steady_clock::time_point loop_end_time;

	float u = 0;
	float d = 0;
	while (!stop_thread) {		
		step_start_time = std::chrono::steady_clock::now();

		// theta_dd = (- g * sin(theta) - pos_dd * cos(theta))/l;
		// pos_dd = (-m_pole * l * theta_dd * cos(theta) + m_pole * l * theta_d * theta_d * sin(theta)) / (m_cart + m_pole);

		// Advance model
		pos_dd = (m_pole*m_pole*l*l*g*cos(theta)*sin(theta) + m_pole*l*l*(m_pole*l*theta_d*theta_d*sin(theta) - d*pos_d) + m_pole*l*l*u) / (m_pole*l*l* (m_cart + m_pole*(1-cos(theta)*cos(theta))));
		theta_dd = (-(m_pole+m_cart)*m_pole*g*l*sin(theta) - m_pole*l*cos(theta)*(m_pole*l*theta_d*theta_d*sin(theta)- d*pos_d) + m_pole*l*cos(theta)*u) / (m_pole*l*l*(m_cart+m_pole*(1-cos(theta)*cos(theta))));

		theta_d = theta_dd * dt + theta_d;
		pos_d = pos_dd * dt + pos_d;

		theta = 0.5*dt*dt*theta_dd + theta_d * dt + theta;
		pos = 0.5*dt*dt*pos_dd + pos_d * dt + pos;

		UE4_tmux.Lock();
		// Update get_values
		current_theta = theta;
		current_pos = pos;
		UE4_tmux.Unlock();

		step_count++;
		
		step_end_time = std::chrono::steady_clock::now();
		step_period = std::chrono::duration<double>(step_end_time - step_start_time).count();
		
		if (step_period < dt){
			FPlatformProcess::Sleep(dt - step_period);
		}
		else{
			UE_LOG(LogTemp, Warning, TEXT("Overflow: Elapsed Time: %f[s], Step Time: %f[s]"),loop_period, step_period)
			UE_LOG(LogTemp, Warning, TEXT("Step period was larger than dt. Time overflow: %f[s]"), (step_period-dt));
		}
		
		loop_end_time = std::chrono::steady_clock::now();
		loop_period = std::chrono::duration<double>(loop_end_time - step_start_time).count();

		if(step_count >= 10000 && step_count != 0){
			UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %f[s], Step Time: %f[s]"),loop_period, step_period);
			step_count = 0;
		}
	// TODO fool loop is dt + 0.0005 might be better to only check the total time and end.
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