/*******************************************************************************
 * File Name          : DCMotor.c
 * Description        : The code created to control DC motor with time and encoder values.
 * 			It has 2 modes, 0 mode is for controlling with time and 1 mode is
 *		        for controlling with degree. 
 * Author:              Group 1
 *			 Sinan KARACA
 *			 Mohammed Al Bunde
 * Date:                25.11.2021				 
 ******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "common.h"
#include "main.h"
#include "math.h"

//Define PI value for sin function
#define PI 3.14159256

//Necessary function for PWM running
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void speedTime(void);

//Global variable declerations
int32_t channel = 0;
int32_t pwmValue = 0;
int32_t ispwmValueEntered = 0;
uint32_t turning, turning1 = 0;
double secCounter2 = 0; 
double secCounter50 = 0;
uint32_t stepCount = 0;
uint32_t totalTime = 0;
uint32_t stepTemp = 0;
uint32_t timerCounterValue;
uint32_t continousCheck = 0;
uint32_t mode;
uint32_t dir;
uint32_t speedPwm;
int32_t timeAngle;
uint32_t timeAngleUnsigned;
uint32_t x;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef tim11;
TIM_HandleTypeDef tim3;
TIM_Encoder_InitTypeDef encoderConfig;




// FUNCTION      : changeTime()
// DESCRIPTION   : Changing running time of motor for mode 0
// PARAMETERS    : Int, take it from terminal
// RETURNS       : ParserReturnVal_t, terminal return parameter

ParserReturnVal_t changeTime(int action) {

	fetch_int32_arg(&timeAngle);
	totalTime = timeAngle * 100;
	
	return CmdReturnOk;
	
}

ADD_CMD("cTime",changeTime,"Time<Seconds>");



// Initialise the two timers
// Initialise the GPIO for pwm output of 3 channel
ParserReturnVal_t glow(int action) {
	
	HAL_StatusTypeDef rc;
	
	
	//uint32_t scalerTimer;
	fetch_uint32_arg(&mode);
	fetch_int32_arg(&timeAngle);
	fetch_uint32_arg(&speedPwm);
	fetch_uint32_arg(&dir);


	if(mode < 0 || mode > 1 ){

		printf("Check the parameters.. \n");
		return CmdReturnBadParameter1;
	
	}
	
	
	//If mode is 0, check the direction and speed.
	//When mode is 1, do not need to check direction and speed.
	if(mode == 0){
		if(dir < 0 || dir > 1){ 
			printf("Check direction <0-1> \n");
		
			return CmdReturnBadParameter1;
		
		}
		
		if( speedPwm < 1|| speedPwm > 100 ) {
			
			printf("Check speed <0-100> \n");
		
			return CmdReturnBadParameter1;
		
		
		}
		
		
	
	}
	
	//Check if typed timeAngle parameter is above or below 0
	//If it is change it to unsigned value
	if(timeAngle > 0){
		timeAngleUnsigned = timeAngle;
	}else{
		timeAngleUnsigned = -1 * timeAngle;
	}
	
	
	totalTime = timeAngle * 100;
	
	
	//initialisation of all variable for next step
	
	
	if (continousCheck == 0){
	
		stepCount = 0; 	
		
	}

	//Change the speed by manipulating the TIM11 period
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* initialize GPIO pins PA6 and PA7 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* Configure these timer pins mode using
	HAL_GPIO_Init() */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = 2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	__HAL_RCC_TIM3_CLK_ENABLE();
	tim3.Instance = TIM3;
	tim3.Init.Prescaler = 0;
	tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim3.Init.Period = 0xffff;
	tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim3.Init.RepetitionCounter = 0;
	rc = HAL_TIM_Base_Init(&tim3);
	if(rc != HAL_OK) {
		printf("Failed to initialize Timer 3 Base, “ ”rc=%u\n",rc);
	return CmdReturnBadParameter1;
	}
	
	encoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	encoderConfig.IC1Polarity = 0;
	encoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	encoderConfig.IC1Prescaler = 0;
	encoderConfig.IC1Filter = 3;
	encoderConfig.IC2Polarity = 0;
	encoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	encoderConfig.IC2Prescaler = 0;
	encoderConfig.IC2Filter = 3;
	
	rc = HAL_TIM_Encoder_Init(&tim3, &encoderConfig);
	if(rc != HAL_OK) {
	printf("Failed to init Timer 3 Encoder," "rc=%u\n",rc);
	return CmdReturnBadParameter1;
	}
	
	rc = HAL_TIM_Encoder_Start_IT(&tim3,TIM_CHANNEL_1);
	if(rc != HAL_OK) {
	printf("Failed to start Timer 3 Encoder," "rc=%u\n",rc);
	return CmdReturnBadParameter1;
	}
	rc = HAL_TIM_Encoder_Start_IT(&tim3,TIM_CHANNEL_2);
	if(rc != HAL_OK) {
	printf("Failed to start Timer 3 Encoder," "rc=%u\n",rc);
	return CmdReturnBadParameter1;
	}
	
	__GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = (GPIO_PIN_8|GPIO_PIN_9);
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// 1000 is maximum value
	// user types 0-100, we multiply with 10
	pwmValue = speedPwm * 10;


	__HAL_RCC_TIM1_CLK_ENABLE();
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	__HAL_RCC_TIM11_CLK_ENABLE();
	tim11.Instance = TIM11;
	tim11.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 100000 - 1;
	tim11.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim11.Init.Period = 999;
	tim11.Init.ClockDivision =
	TIM_CLOCKDIVISION_DIV1;
	tim11.Init.RepetitionCounter = 0;

	HAL_TIM_Base_Init(&tim11);

	HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 10, 0U);
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

	HAL_TIM_Base_Start_IT(&tim11);

	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim1);
	
	//Enable 3 channels
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	return CmdReturnOk;
}


// FUNCTION      : HAL_TIM_MspPostInit()
// DESCRIPTION   : Initialise the 3 output channel pins
//		   In this lab, only one PWM output has been used.
// PARAMETERS    : TIM_HandleTypeDef -- Take the TIM1 as input
// RETURNS       : None

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (htim->Instance == TIM1) {

		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}

}

// FUNCTION      : resetPosition()
// DESCRIPTION   : reset the position of DC motor
//		   delete the memory of encoder
// PARAMETERS    : int, takes it from terminal
// USAGE	 : resetPos
// RETURNS       : ParserReturnVal_t, return terminal return parameter

ADD_CMD("pwmGlow",glow,"Mode<0-1> Time<Seconds> SpeedPwm<Percentage> Dir<0-1>");

ParserReturnVal_t resetPosition(int action) {

	TIM3->CNT = 0;
	mode = 2;
	timerCounterValue = 0;
	
	printf("Position zeroing is completed.");
	printf("Current position : %ld\n", TIM3->CNT);

	return CmdReturnOk;


}



ADD_CMD("resetPos",resetPosition,"Resetting position!");

// FUNCTION      : monitorPosition()
// DESCRIPTION   : monitor the position of motor in Degree
// PARAMETERS    : int, takes it from terminal
// USAGE	 : monitorPos
// RETURNS       : ParserReturnVal_t, return terminal return parameter

ParserReturnVal_t monitorPosition(int action) {

	uint32_t tempPos;
	
	tempPos = TIM3->CNT % 120 ;
	tempPos = tempPos * 3;
	
	

	printf("Current position : %ld\n", tempPos);
	
	return CmdReturnOk;


}

ADD_CMD("monitorPos",monitorPosition,"Resetting position!");



// FUNCTION      : TIM1_TRG_COM_TIM11_IRQHandler()
// DESCRIPTION   : Interrupt handler function for TIM11
//		   Used to measure time
// PARAMETERS    : None
// RETURNS       : None

void TIM1_TRG_COM_TIM11_IRQHandler (void) {

	HAL_TIM_IRQHandler(&tim11);

}

// FUNCTION      : TIM3_IRQHandler()
// DESCRIPTION   : Interrupt handler function for TIM3
//		   PWM interrupt function
// PARAMETERS    : None
// RETURNS       : None

void TIM3_IRQHandler (void) {

	HAL_TIM_IRQHandler(&tim3);

}


// FUNCTION      : HAL_TIM_PeriodElapsedCallback()
// DESCRIPTION   : It changes the duty cycle percentage for each interrupt cycle
// PARAMETERS    : TIM_HandleTypeDef ---- TIM11 
// RETURNS       : None

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {


	if (htim == &tim11) {
		
		//mode 0 is for speed time control of DC motor
		//run motor for specified time
		//control CCW and CW turns of motor
		if(mode == 0){
		
			if(dir == 0) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);     
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 1);     
	
			} else if(dir == 1){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);     
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 0);     
			}
			speedTime();
			
		
		//mode 1 is for encoder controll of DC motor
		//run motor for specified degree of turn
		//CCW for minus degrees and CW for positive degrees	
		}else if(mode == 1){
			
			if (timeAngle > 0){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);     
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 0); 
				
				if(TIM3->CNT > 64536){
			   		timerCounterValue = 5;
				}else{
			   		timerCounterValue = TIM3->CNT + 5 ;
				}
				
			}else{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);     
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 1); 
				

				if(TIM3->CNT < 1000){
			   		timerCounterValue = 5;
				}else{
			   		timerCounterValue = 65536 - TIM3->CNT + 5 ;
			   		
				}
			
			}


			//Initial speed of 7.5 percent of PWM signal
			//Speed is increasing proportionally.
			//120 pulse for each turn
			//Whenever it reached to the specified degree, it should stop the motor.
			if(timeAngleUnsigned  <= timerCounterValue * 3  ){
				
				htim1.Instance->CCR1 = 0;
				x = 1;
			
			} else if ((timeAngleUnsigned - timerCounterValue * 3 ) < 500 || timerCounterValue < 500){
				htim1.Instance->CCR1 = 75;
				x = 2;
			
			} else if ((timeAngleUnsigned - timerCounterValue * 3  ) < 1000 || timerCounterValue < 1000){
				htim1.Instance->CCR1 = 100;
				x = 3;
			
			} else if ((timeAngleUnsigned - timerCounterValue * 3  ) < 5000 || timerCounterValue < 5000){
				htim1.Instance->CCR1 = 110;
				x = 4;
			
			} else if((timeAngleUnsigned - timerCounterValue * 3  ) < 10000 || timerCounterValue < 10000){
				htim1.Instance->CCR1 = 120;
				x = 5;
			} else if((timeAngleUnsigned - timerCounterValue * 3  ) < 20000 || timerCounterValue < 20000){
				htim1.Instance->CCR1 = 130;
				x = 6;
				
			}
		}
		

		
	}

}

// FUNCTION      : speedTime()
// DESCRIPTION   : The function for sinosodial acceleration 
//		   Only used for Dc motor time&speed control for DC Motor Lab
// PARAMETERS    : None 
// RETURNS       : None

void speedTime(void) {

	double resultChn;
	double resultTime;
	
	// 100 = 1 sec
	// 2 = 20 ms
	
	
	//Initial acceleration 	
	if(stepCount < totalTime  / 2){
		continousCheck = 1;

		resultTime = pwmValue / 2;
		
		if(secCounter50 < resultTime){
		
			if(secCounter2 >= 5){

			    secCounter2 = 0;
			    
			    secCounter50 = secCounter50 + 1;
			    
			}else{
			    secCounter2 = secCounter2 + 1 ;
			}
			// 3.14159265
			
			resultChn = (double)pwmValue / resultTime; 

			resultChn = sin(resultChn * secCounter50 / (double)pwmValue * 1.570796325) * (double)pwmValue;

			htim1.Instance->CCR1 = resultChn;
			
			stepTemp = stepCount;
			

		
		}
		
		stepCount = stepCount + 1; 

	
        // Slowing down and stopping
	} else if(stepCount > totalTime - stepTemp){
	
			resultTime = pwmValue / 2;
			
			if(secCounter50 > 0){
			
				if(secCounter2 >= 5){

				    secCounter2 = 0;
				    
				    secCounter50 = secCounter50 - 1;
				    
				}else{
				    secCounter2 = secCounter2 + 1 ;
				}
				// 3.14159265
				
				resultChn = (double)pwmValue / resultTime; 

				resultChn = sin(resultChn * secCounter50 / (double)pwmValue * 1.570796325) * (double)pwmValue;

				htim1.Instance->CCR1 = resultChn;
				
				stepCount = stepCount + 1;
				
				
				
			} else {
			
				totalTime = 0;
				secCounter50 = 0;
				continousCheck = 0;
				stepCount = 0;
			
			}		
	// If it is not slowing or accelerating, it will keep the speed constant
	} else {
	
                        resultTime = pwmValue / 2;
                        
                        resultChn = (double)pwmValue / resultTime; 
		
			resultChn = sin(resultChn * secCounter50 / (double)pwmValue * 1.570796325) * (double)pwmValue;

			htim1.Instance->CCR1 = resultChn;
			
			stepCount = stepCount + 1;
			
	
	}
	
    }

