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
	theta = 2.14;
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
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		if (step_count <= calculations) {

			theta_dd = (- g * sin(theta) - pos_dd * cos(theta))/l;
			pos_dd = (-m_pole * l * theta_dd * cos(theta) + m_pole * l * theta_d * theta_d * sin(theta)) / (m_cart + m_pole);

			theta_d = theta_dd * dt + theta_d;
			pos_d = pos_dd * dt + pos_d;

			theta = 0.5*dt*dt*theta_dd + theta_d * dt + theta;
			pos = 0.5*dt*dt*pos_dd + pos_d * dt + pos;

			current_theta = theta;
			current_pos = pos;

			step_count++;
		}
		else {
			stop_thread = true;
		}

		// Update get_values
		
		UE4_tmux.Unlock();
		FPlatformProcess::Sleep(0.0005);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		// std::chrono::microseconds ms_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
		// long long ms_elapsed = ms_time.count();
		// double time_seconds = static_cast<double>(ms_elapsed)/1000000;
		double time_seconds = std::chrono::duration<double>(end - begin).count();
		// UE_LOG(LogTemp, Warning, TEXT("Elapsed time: %li, Time sconds: %f"), ms_elapsed, time_seconds);
		// UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %f[s]"), time_seconds);
		
		if (time_seconds < dt){
			FPlatformProcess::Sleep(dt - time_seconds);
		}
		else{
			// UE_LOG(LogTemp, Warning, TEXT("Time overflow: %li"), ms_elapsed);
			UE_LOG(LogTemp, Warning, TEXT("Time overflow: %f[s]"), time_seconds);
		}
		std::chrono::steady_clock::time_point step_end = std::chrono::steady_clock::now();
		double step_seconds = std::chrono::duration<double>(step_end - begin).count();
		UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %f[s], Step Time: %f[s]"),time_seconds, step_seconds);

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