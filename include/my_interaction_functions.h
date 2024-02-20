#pragma once

extern "C" {
#include <interface.h>
}

////////////////
// Ports related functions
///////////////////////////

// given a byte value, returns the value of its bit n
int GetBitValue(int portNumber, int portIndex);

// given a byte value, set the n bit to value
void SetBitValue(int portNumber, int portIndex, int value);

//////////////////////////////////
// Conveyor Feed related functions
//////////////////////////////////

//Stops ConveyorFeed 
void stopConveyorFeed();

//Moves ConveyorFeed forward
void moveConveyorFeed();

//////////////////////////////////
// Conveyor Swap related functions
//////////////////////////////////

//Stops ConveyorSwap 
void stopConveyorSwap();

//Moves ConveyorSwap forward
void moveConveyorSwap();

//////////////////////////////////
// Conveyor Drilling related functions
//////////////////////////////////

//Stops ConveyorDrilling
void stopConveyorDrilling();

//Moves ConveyorDrilling forward
void moveConveyorDrilling();

/////////////////////////////
// Conveyor Milling related functions
/////////////////////////////

//Stops conveyorMilling
void stopConveyorMilling();

//Moves conveyorMilling forward
void moveConveyorMilling();

/////////////////////////////
// Slider 1 related functions
/////////////////////////////

void stopSlider1();

void moveSlider1Front();

void moveSlider1Back();

int getSlider1Pos();

void gotoSlider1(int pos);

/////////////////////////////
// Slider 2 related functions
/////////////////////////////

void stopSlider2();

void moveSlider2Front();

void moveSlider2Back();

int getSlider2Pos();

void gotoSlider2(int pos);

////////////////////////
// Milling Machine related functions
////////////////////////

void stopMillingMachine();

void executeMillingMachine();

////////////////////////
// Milling Machine related functions
////////////////////////

void stopDrillingMachine();

void executeDrillingMachine();

////////////////////////
// Block Sensores related functions
////////////////////////

//Retuns True if the Loading Station sensor is detecting a block, otherwise returns False
bool SenseLoadingStation();

//Retuns True if the Drilling Machine sensor is detecting a block, otherwise returns False
bool SenseDrillingMachine();

//Retuns True if the MillingMachine sensor is detecting a block, otherwise returns False
bool SenseMillingMachine();

//Retuns True if the Slinder 1 sensor is detecting a block, otherwise returns False
bool SenseSlider1();

//Retuns True if the ConveyorSwap sensor is detecting a block, otherwise returns False
bool SenseConveyorSwap();