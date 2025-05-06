
/* Standard includes. */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"

/* Demo application includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

// flag represented in binary tree
//char flag[] = "CGGC{If yOU c@n 5O1VE 7h!5 THeN y0u tRU3LY Pr0VE you @rE Cy5T!cK now}";
//int flag_len = sizeof(flag) - 1;

char *layers[]={
    "C",
    "G ",
    "G0n",
    "C1uyo",
    "{ V   oCw",
    "Iyc EhTNt3PVury!}",
    "f OU@n5O 7!5He yRULYr0E  @E 5TcK"
};

#define LAYER_CNT sizeof(layers) / sizeof(char*)

int layer_idx[LAYER_CNT] = {0};
char fail_str[] = "you are not cystick enough\n";
char success_str[] = "you are truelly a cystick now :D\n";

// create task by user input, and verify according to tsk priority
void ValidateUserInput(Socket_t xSocket)
{
    int layer = uxTaskPriorityGet(xTaskGetCurrentTaskHandle()) - 1;

    if (xSocket->u.xTCP.eTCPState == eCLOSED || xSocketValid(xSocket) == pdFALSE)
    {
        memset(layer_idx, 0, sizeof(layer_idx));
        vTaskDelete(NULL);
        return;
    }
    if (layer >= LAYER_CNT || layer_idx[layer] >= strlen(layers[layer]))
    {
        vTaskDelete(NULL);
        return;
    }
    
    char cur_char;
    if (FreeRTOS_recv(xSocket, &cur_char, 1, 0) == 0)
    {
        FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
        FreeRTOS_closesocket( xSocket );
        memset(layer_idx, 0, sizeof(layer_idx));
        vTaskDelete(NULL);
        return;
    }

    if (cur_char == 0) // EOF
    {
        // do not close socket again
        memset(layer_idx, 0, sizeof(layer_idx));
        vTaskDelete(NULL);
        return;
    }

    if (cur_char != layers[layer][layer_idx[layer]])
    {

        FreeRTOS_send(xSocket, fail_str, sizeof(fail_str), 0);
        FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
        FreeRTOS_closesocket( xSocket );
        memset(layer_idx, 0, sizeof(layer_idx));
        vTaskDelete(NULL);
        return;
    }
    
    layer_idx[layer] ++;
    layer ++;
    
    bool check = true;
    for (int i = 0; i != LAYER_CNT; ++i)
        if (layer_idx[i] != strlen(layers[i]))
        {
            check = false;
            break;
        }

    if (check)
    {
        FreeRTOS_send(xSocket, success_str, sizeof(success_str), 0);
        FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
        FreeRTOS_closesocket( xSocket );
        memset(layer_idx, 0, sizeof(layer_idx));
        vTaskDelete(NULL);
        return;
    }
    for (int i = 0; i != 2; ++ i)
        xTaskCreate(ValidateUserInput, "씨팔", 0x100, xSocket, layer + 1, 0);
    vTaskDelete(NULL);
}