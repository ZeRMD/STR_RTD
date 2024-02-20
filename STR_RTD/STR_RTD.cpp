// STR_RTD.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <stdio.h>
#include<conio.h>
#include<stdlib.h>
#include <windows.h> //for Sleep function
#include "my_interaction_functions.h"

extern "C" {
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include <interrupts.h>
#include <ctime>
#include <interface.h>
}

#define mainREGION_1_SIZE   8201
#define mainREGION_2_SIZE   29905
#define mainREGION_3_SIZE   7607

////////////////////////////////
///////////////////////Handlers
////////////////////////////////

xSemaphoreHandle sem_Feeding;
xSemaphoreHandle sem_Mill;
xSemaphoreHandle sem_Drill;
xSemaphoreHandle sem_Swap;

xSemaphoreHandle sem_Conveyor_Milling_move;
xSemaphoreHandle sem_Conveyor_Milling_stop;

xSemaphoreHandle sem_Milling_Is_Free;
xSemaphoreHandle sem_Drilling_Is_Free;
xSemaphoreHandle sem_Swap_Is_Free;

xSemaphoreHandle sem_Conveyor_Millig_Machine_move;
xSemaphoreHandle sem_Conveyor_Millig_Machine_stop;
xSemaphoreHandle sem_Conveyor_Drillig_Machine_move;
xSemaphoreHandle sem_Conveyor_Drillig_Machine_stop;

xSemaphoreHandle sem_Slider_1;
xSemaphoreHandle sem_Slider_2;

xQueueHandle mbx_Milling_Machine;
xQueueHandle mbx_Drilling_Machine;

///////////////////
//////Base Funtions
///////////////////

void vAssertCalled(unsigned long ulLine, const char* const pcFileName)
{
	static BaseType_t xPrinted = pdFALSE;
	volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;
	/* Called if an assertion passed to configASSERT() fails.  See
	http://www.freertos.org/a00110.html#configASSERT for more information. */
	/* Parameters are not used. */
	(void)ulLine;
	(void)pcFileName;
	printf("ASSERT! Line %ld, file %s, GetLastError() %ld\r\n", ulLine, pcFileName, GetLastError());

	taskENTER_CRITICAL();
	{
		/* Cause debugger break point if being debugged. */
		__debugbreak();
		/* You can step out of this function to debug the assertion by using
		   the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		   value. */
		while (ulSetToNonZeroInDebuggerToContinue == 0)
		{
			__asm { NOP };
			__asm { NOP };
		}
	}
	taskEXIT_CRITICAL();
}

static void  initialiseHeap(void)
{
	static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
	/* Just to prevent 'condition is always true' warnings in configASSERT(). */
	volatile uint32_t ulAdditionalOffset = 19;
	const HeapRegion_t xHeapRegions[] =
	{
		/* Start address with dummy offsetsSize */
		{ ucHeap + 1,mainREGION_1_SIZE },
		{ ucHeap + 15 + mainREGION_1_SIZE,mainREGION_2_SIZE },
		{ ucHeap + 19 + mainREGION_1_SIZE +
				mainREGION_2_SIZE,mainREGION_3_SIZE },
		{ NULL, 0 }
	};


	configASSERT((ulAdditionalOffset +
		mainREGION_1_SIZE +
		mainREGION_2_SIZE +
		mainREGION_3_SIZE) < configTOTAL_HEAP_SIZE);
	/* Prevent compiler warnings when configASSERT() is not defined. */
	(void)ulAdditionalOffset;
	vPortDefineHeapRegions(xHeapRegions);
}

//////////////////////
/////////////Main Menu
//////////////////////

void Task_Main_menu(void* pvParameters) {
	printf("\n///// BRICK INPUT MENU /////\nType \"1\" for Introduction of a block type 1\nType \"2\" for Introduction of a block type 2\nType \"3\" for Introduction of a block type 3\n///// BRICK INPUT MENU /////\n\nInput: ");
	int tecla = 0;
	bool packageType;
	while (TRUE)
	{
		//27=ESC
		while (tecla != 27) {
			tecla = getchar();
			switch (tecla) {
			case 'M': //Re-prints the Menu

				printf("\n///// Product INPUT MENU /////\nType \"1\" for Introduction of a block type 1\nType \"2\" for Introduction of a block type 2\nType \"3\" for Introduction of a block type 3\n///// BRICK INPUT MENU /////\n\nInput: ");

				break;
			case '1': //Block insertion with confirmation
				packageType = false;
				xQueueSend(mbx_Milling_Machine, &packageType, portMAX_DELAY);
				packageType = false;
				xQueueSend(mbx_Drilling_Machine, &packageType, portMAX_DELAY);
				xSemaphoreGive(sem_Feeding);
				break;
			case '2': //Block insertion without confirmation
				packageType = true;
				xQueueSend(mbx_Milling_Machine, &packageType, portMAX_DELAY);
				packageType = false;
				xQueueSend(mbx_Drilling_Machine, &packageType, portMAX_DELAY);
				xSemaphoreGive(sem_Feeding);
				break;
			case '3':
				packageType = false;
				xQueueSend(mbx_Milling_Machine, &packageType, portMAX_DELAY);
				packageType = true;
				xQueueSend(mbx_Drilling_Machine, &packageType, portMAX_DELAY);
				xSemaphoreGive(sem_Feeding);
				break;
			case '4':
				packageType = true;
				xQueueSend(mbx_Milling_Machine, &packageType, portMAX_DELAY);
				packageType = true;
				xQueueSend(mbx_Drilling_Machine, &packageType, portMAX_DELAY);
				xSemaphoreGive(sem_Feeding);
				break;
			}
		}
	}
}

/////////////////////////
///////Conveyors and Sliders
/////////////////////////

void Task_Feeding(void* pvParameters) {
	while (true) {
		xSemaphoreTake(sem_Feeding, portMAX_DELAY);
		
		while (!SenseLoadingStation()) {
			continue;
		}
		
		moveConveyorFeed();

		while (!SenseSlider1()) {
			continue;
		}
		
		if (xSemaphoreTake(sem_Milling_Is_Free, 0) != pdTRUE) {
			stopConveyorFeed();
			xSemaphoreTake(sem_Milling_Is_Free, portMAX_DELAY);
			moveConveyorFeed();
		}
		
		vTaskDelay(1000);

		xSemaphoreGive(sem_Slider_1);
		xSemaphoreGive(sem_Mill);
		stopConveyorFeed();
	}
}

void Task_Swaping(void* pvParameters) {
	while (true) {
		xSemaphoreTake(sem_Swap, portMAX_DELAY);
		moveConveyorSwap();
		
		while (!SenseConveyorSwap()) {
			continue;
		}
		
		vTaskDelay(1000);

		stopConveyorSwap();
		if (!SenseConveyorSwap()) {
			xSemaphoreGive(sem_Swap_Is_Free);
		}
	}
}

void Task_Slider_1(void* pvParameters) {
	//gotoSlider1(0);
	while (true) {
		xSemaphoreTake(sem_Slider_1, portMAX_DELAY);
		gotoSlider1(1);
		gotoSlider1(0);
	}
}

void Task_Conveyor_Milling(void* pvParameters) {
	//gotoSlider1(0);
	while (true) {
		xSemaphoreTake(sem_Conveyor_Milling_move, portMAX_DELAY);
		moveConveyorMilling();
		xSemaphoreTake(sem_Conveyor_Milling_stop, portMAX_DELAY);
		stopConveyorMilling();
	}
}

void Task_Slider_2(void* pvParameters) {
	//gotoSlider2(0);
	while (true) {
		xSemaphoreTake(sem_Slider_2, portMAX_DELAY);
		gotoSlider2(1);
		gotoSlider2(0);
	}
}

///////////////////////////
////////////Machines
///////////////////////////

void Task_Milling_Machine(void* pvParameters) {
	while (true) {
		bool next_to_pass;
		
		xQueueReceive(mbx_Milling_Machine, &next_to_pass, portMAX_DELAY);

		xSemaphoreTake(sem_Mill, portMAX_DELAY);
		xSemaphoreGive(sem_Conveyor_Milling_move);

		while (!SenseMillingMachine()) //Waits for a product to be in front of the Machine
			continue;

		if (next_to_pass) //if the product is meant to be Milled we mill it
		{
			xSemaphoreGive(sem_Conveyor_Milling_stop);
			executeMillingMachine();
			vTaskDelay(1000);
			stopMillingMachine();
			xSemaphoreGive(sem_Conveyor_Milling_move);
		}
		else {
			while (SenseMillingMachine()) //if the product is meant to be let trough we wait for it to pass
				continue;
		}
			
		if(xSemaphoreTake(sem_Drilling_Is_Free, 0) != pdTRUE) {
			xSemaphoreGive(sem_Conveyor_Milling_stop);
			xSemaphoreTake(sem_Drilling_Is_Free, portMAX_DELAY);
			xSemaphoreGive(sem_Conveyor_Milling_move);
		}
		xSemaphoreGive(sem_Milling_Is_Free);
		xSemaphoreGive(sem_Drill);

	}
}

void Task_Drilling_Machine(void* pvParameters) {
	while (true) {
		bool next_to_pass;

		xQueueReceive(mbx_Drilling_Machine, &next_to_pass, portMAX_DELAY); 

		xSemaphoreTake(sem_Drill, portMAX_DELAY);
		moveConveyorDrilling();

		while (!SenseDrillingMachine()) //Waits for a product to be in front of the Machine
			continue;
		xSemaphoreGive(sem_Conveyor_Milling_stop);

		if (next_to_pass) //if the product is meant to be Milled we mill it
		{
			stopConveyorDrilling();
			executeDrillingMachine();
			vTaskDelay(1000);
			stopDrillingMachine();
			moveConveyorDrilling();
		}
		else {
			while (SenseDrillingMachine()) //if the product is meant to be let trough we wait for it to pass
				continue;
		}

		if (xSemaphoreTake(sem_Swap_Is_Free, 0) != pdTRUE) {
			stopConveyorDrilling();
			xSemaphoreTake(sem_Swap_Is_Free, portMAX_DELAY);
			moveConveyorDrilling();
		}
		
		xSemaphoreGive(sem_Drilling_Is_Free);

		vTaskDelay(1000);
		xSemaphoreGive(sem_Slider_2);
		xSemaphoreGive(sem_Swap);
		stopConveyorDrilling();
	}
}

//////////////////////////
////////////////Interrupts
//////////////////////////

void myDaemonTaskStartupHook(void) {
	
	sem_Feeding = xSemaphoreCreateCounting(100, 0);
	sem_Mill = xSemaphoreCreateCounting(100, 0);
	sem_Drill = xSemaphoreCreateCounting(100, 0);
	sem_Swap = xSemaphoreCreateCounting(100, 0);

	sem_Conveyor_Milling_move = xSemaphoreCreateCounting(100, 0);
	sem_Conveyor_Milling_stop = xSemaphoreCreateCounting(100, 0);

	sem_Milling_Is_Free = xSemaphoreCreateCounting(100, 1);
	sem_Drilling_Is_Free = xSemaphoreCreateCounting(100, 1);
	sem_Swap_Is_Free = xSemaphoreCreateCounting(100, 1);
	
	sem_Slider_1 = xSemaphoreCreateCounting(100, 0);
	sem_Slider_2 = xSemaphoreCreateCounting(100, 0);

	mbx_Milling_Machine = xQueueCreate(100, sizeof(bool));
	mbx_Drilling_Machine = xQueueCreate(100, sizeof(bool));

	xTaskCreate(Task_Feeding, "Task_Feeding", 100, NULL, 0, NULL);
	xTaskCreate(Task_Swaping, "Task_Swaping", 100, NULL, 0, NULL);

	xTaskCreate(Task_Conveyor_Milling, "Task_Conveyor_Milling", 100, NULL, 0, NULL);
	
	xTaskCreate(Task_Slider_1, "Task_Slider_1", 100, NULL, 0, NULL);
	xTaskCreate(Task_Slider_2, "Task_Slider_2", 100, NULL, 0, NULL);

	xTaskCreate(Task_Milling_Machine, "Task_Milling_Machine", 100, NULL, 0, NULL);
	xTaskCreate(Task_Drilling_Machine, "Task_Drilling_Machine", 100, NULL, 0, NULL);
	
	xTaskCreate(Task_Main_menu, "Task_Main_menu", 100, NULL, 0, NULL);
}

int main(int argc, char** argv) {
	initialiseHeap();
	vApplicationDaemonTaskStartupHook = &myDaemonTaskStartupHook;
	//vApplicationTickHook = &myTickHook;
	//vApplicationIdleHook = &myIdleHook;
	vTaskStartScheduler();
	Sleep(5000);
}