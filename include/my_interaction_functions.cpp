#include "my_interaction_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
/*
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#define SERVER_IP "192.168.2.69"
#define SERVER_PORT 80

SOCKET clientSocket;

extern "C" {
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include <interrupts.h>
#include <conio.h>
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Unity //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int ConnectUnity()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed.\n");
        return EXIT_FAILURE;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    //struct in_addr ipAddress;
    //serverAddr.sin_addr.s_addr = inet_pton(AF_INET, SERVER_IP, &ipAddress);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Connection failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }
}

int DisconnectUnity()
{
    // Close the socket
    closesocket(clientSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// BIT //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int GetBitValue(int portNumber, int portIndex)
{
    printf("Get\n");
    char message[5];

    message[0] = 'G';
    message[1] = portNumber + '0';
    message[2] = portIndex + '0';
    message[3] = 'a';
    message[4] = 'b';

    send(clientSocket, message, 5, 0);
    //printf("Message sent to Unity: %s\n", message);
    printf("SentG\n");
    char buffer;
    recv(clientSocket, &buffer, sizeof(char), 0);
    //printf("Response from Unity: %d\n", buffer[0]);

    printf("ReceivedG");
    printf("\n\n\n\n%c\n\n\n\n\n", buffer);
    printf("\n\n\n\n%d\n\n\n\n\n", buffer);

    return buffer - '0';
    timer
    delay();
}

void SetBitValue(int portNumber, int portIndex, int value)
{
    if (value != 0 && value != 1)
        return;

    printf("Set\n");
    char message[5];
    message[0] = 'S';
    message[1] = portNumber + '0';
    message[2] = portIndex + '0';
    message[3] = ' ';
    message[4] = value + '0';

    send(clientSocket, message, 5, 0);
    //printf("Message sent to Unity: %s\n", message);
    printf("SentSet\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Conveyor Feed //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopConveyorFeed()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 5, 0);
    taskEXIT_CRITICAL();
}

void moveConveyorFeed()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 5, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Conveyor Swap //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopConveyorSwap()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 10, 0);
    taskEXIT_CRITICAL();
}

void moveConveyorSwap()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 10, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Conveyor Drilling //////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopConveyorDrilling()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 8, 0);
    taskEXIT_CRITICAL();
}

void moveConveyorDrilling()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 8, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Conveyor Milling ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopConveyorMilling()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 6, 0);
    taskEXIT_CRITICAL();
}

void moveConveyorMilling()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 6, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Slider 1 ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopSlider1()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 1, 0); 
    SetBitValue(0, 2, 0); 
    taskEXIT_CRITICAL();
}

void moveSlider1Front()
{
    taskENTER_CRITICAL(); 
    SetBitValue(0, 2, 0);
    SetBitValue(0, 1, 1);
    taskEXIT_CRITICAL();
}

void moveSlider1Back()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 1, 0);
    SetBitValue(0, 2, 1);
    taskEXIT_CRITICAL();
}

int getSlider1Pos()
{
    taskENTER_CRITICAL();
    int I1 = GetBitValue(1, 1);
    int I2 = GetBitValue(1, 2);
    taskEXIT_CRITICAL();
    if (I2 == 1)
        return 0;           //if high level it's retracted
    else if (I1 == 1)
        return 1;           //if high level it's extended
    else return -1;         //if none are high level it's not on either ends
}

void gotoSlider1(int pos)
{
    if (pos == 1) {
        moveSlider1Front(); //if pos == 1 then send the Slider 1 forward
        
        while (getSlider1Pos() != 1) {
            continue;
        }
    }
    else {
        moveSlider1Back(); //if pos == 0 then send the Slider 1 back
        
        while (getSlider1Pos() != 0) {
            continue;
        }
    }

    stopSlider1(); //when pos reached stop
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Slider 2 ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopSlider2()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 3, 0);
    SetBitValue(0, 4, 0);
    taskEXIT_CRITICAL();
}

void moveSlider2Front()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 4, 0);
    SetBitValue(0, 3, 1);
    taskEXIT_CRITICAL();
}

void moveSlider2Back()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 3, 0);
    SetBitValue(0, 4, 1);
    taskEXIT_CRITICAL();
}

int getSlider2Pos()
{
    taskENTER_CRITICAL();
    int I3 = GetBitValue(1, 3);
    int I4 = GetBitValue(1, 4);
    taskEXIT_CRITICAL();
    if (I4 == 1)
        return 0;           //if high level it's retracted
    else if (I3 == 1)
        return 1;           //if high level it's extended
    else return -1;         //if none are high level it's not on either ends
}

void gotoSlider2(int pos)
{
    if (pos == 1) {
        moveSlider2Front(); //if pos == 1 then send the Slider 1 forward

        while (getSlider2Pos() != 1) {
            continue;
        }
    }
    else {
        moveSlider2Back(); //if pos == 0 then send the Slider 1 back

        while (getSlider2Pos() != 0) {
            continue;
        }
    }

    stopSlider2(); //when pos reached stop
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Milling Machine /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopMillingMachine()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 7, 0);
    taskEXIT_CRITICAL();
}

void executeMillingMachine()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 7, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Drilling Machine ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void stopDrillingMachine()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 9, 0);
    taskEXIT_CRITICAL();
}

void executeDrillingMachine()
{
    taskENTER_CRITICAL();
    SetBitValue(0, 9, 1);
    taskEXIT_CRITICAL();
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Block Sensors //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SenseLoadingStation()
{
    taskENTER_CRITICAL();
    int bit = GetBitValue(1, 7);
    taskEXIT_CRITICAL();
    if (bit == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool SenseDrillingMachine()
{
    taskENTER_CRITICAL();
    int bit = GetBitValue(1, 8);
    taskEXIT_CRITICAL();
    if (bit == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool SenseMillingMachine()
{
    taskENTER_CRITICAL();
    int bit = GetBitValue(1, 6);
    taskEXIT_CRITICAL();
    if (bit == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool SenseSlider1()
{
    taskENTER_CRITICAL();
    int bit = GetBitValue(1, 5);
    taskEXIT_CRITICAL();
    if (bit == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool SenseConveyorSwap()
{
    taskENTER_CRITICAL();
    int bit = GetBitValue(1, 9);
    taskEXIT_CRITICAL();
    if (bit == 1) {
        return true;
    }
    else {
        return false;
    }
}