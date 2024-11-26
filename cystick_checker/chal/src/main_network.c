/* Standard includes. */
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"

/* Demo application includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "CMSIS/CMSDK_CM3.h"


#define usUsedStackSize 0x1000


static const uint8_t ucIPAddress[ 4 ] =
{
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};
static const uint8_t ucNetMask[ 4 ] =
{
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};
static const uint8_t ucGatewayAddress[ 4 ] =
{
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};
static const uint8_t ucDNSServerAddress[ 4 ] =
{
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};
const uint8_t ucMACAddress[ 6 ] =
{
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

void prvServerConnectionInstance( Socket_t server_sock )
{
    FreeRTOS_debug_printf( ("server connected\n") );
    
    /* Initiate a shutdown in case it has not already been initiated. */
    FreeRTOS_shutdown( server_sock, FREERTOS_SHUT_RDWR );
    FreeRTOS_closesocket( server_sock );
    vTaskDelete(NULL);
}

void vCreateTCPServerSocket( void )
{
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 20;

    /* Attempt to open the socket. */
    xListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET4, /* Or FREERTOS_AF_INET6 for IPv6. */
                                        FREERTOS_SOCK_STREAM,  /* SOCK_STREAM for TCP. */
                                        FREERTOS_IPPROTO_TCP);

    /* Check the socket was created. */
    configASSERT( xListeningSocket != FREERTOS_INVALID_SOCKET );

    /* If FREERTOS_SO_RCVBUF or FREERTOS_SO_SNDBUF are to be used with
       FreeRTOS_setsockopt() to change the buffer sizes from their default then do
       it here!. (see the FreeRTOS_setsockopt() documentation. */

    /* If ipconfigUSE_TCP_WIN is set to 1 and FREERTOS_SO_WIN_PROPERTIES is to
       be used with FreeRTOS_setsockopt() to change the sliding window size from
       its default then do it here! (see the FreeRTOS_setsockopt()
       documentation. */

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt( xListeningSocket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeOut,
                         sizeof( xReceiveTimeOut ) );

    /* Set the listening port to 10000. */
    memset( &xBindAddress, 0, sizeof(xBindAddress) );
    xBindAddress.sin_port = ( uint16_t ) 48763;
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
    xBindAddress.sin_family = FREERTOS_AF_INET4; /* FREERTOS_AF_INET6 to be used for IPv6 */

    /* Bind the socket to the port that the client RTOS task will send to. */
    FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

    /* Set the socket into a listening state so it can accept connections.
       The maximum number of simultaneous connections is limited to 20. */
    FreeRTOS_listen( xListeningSocket, xBacklog );

    for( ;; )
    {
        /* Wait for incoming connections. */
        xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );
        configASSERT( xConnectedSocket != FREERTOS_INVALID_SOCKET );

        /* Spawn a RTOS task to handle the connection. */
        xTaskCreate( (TaskFunction_t) prvServerConnectionInstance,
                     "CystickChecker",
                     usUsedStackSize,
                     ( void * ) xConnectedSocket,
                     tskIDLE_PRIORITY,
                     NULL );
    }
}

static NetworkInterface_t xInterfaces[ 1 ];
static NetworkEndPoint_t xEndPoints[ 4 ];

void init_networking()
{ 
    BaseType_t xReturn;
    const uint32_t ulLongTime_ms = pdMS_TO_TICKS( 1000UL );

    FreeRTOS_debug_printf( ("start init networking\n") );
    NVIC_SetPriority( ETHERNET_IRQn, configMAC_INTERRUPT_PRIORITY );
    extern NetworkInterface_t * pxMPS2_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                                    NetworkInterface_t * pxInterface );
    pxMPS2_FillInterfaceDescriptor( 0, &( xInterfaces[ 0 ] ) );
    FreeRTOS_FillEndPoint( &( xInterfaces[ 0 ] ), &( xEndPoints[ 0 ] ), ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
    FreeRTOS_debug_printf( ("init IP stack\n") );
    xReturn = FreeRTOS_IPInit_Multi();
    FreeRTOS_debug_printf( ("done init IP stack\n") );
    
    configASSERT( xReturn == pdTRUE );

    /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
       are created in the vApplicationIPNetworkEventHook() hook function
       below.  The hook function is called when the network connects. */                    

    /*
     * Other RTOS tasks can be created here.
     */

    FreeRTOS_debug_printf( ("start sched\n") );
    
    /* Start the RTOS scheduler. */
    vTaskStartScheduler();
    
    /* If all is well, the scheduler will now be running, and the following
       line will never be reached.  If the following line does execute, then
       there was insufficient FreeRTOS heap memory available for the idle and/or
       timer tasks to be created. */
    FreeRTOS_debug_printf( ("enter sleeping loop\n") );
    
    for( ;; )
        usleep( ulLongTime_ms * 1000 );;
}

void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                           struct xNetworkEndPoint * pxEndPoint )
{
    FreeRTOS_debug_printf( ("vApplicationIPNetworkEventHook_Multi\n") );
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* Both eNetworkUp and eNetworkDown events can be processed here. */
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the TCP/IP stack if they have not already
        been created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /*
             * For convenience, tasks that use FreeRTOS-Plus-TCP can be created here
             * to ensure they are not created before the network is usable.
             */
            FreeRTOS_debug_printf( ("create server task\n") );
            xTaskCreate( (TaskFunction_t) vCreateTCPServerSocket,
                     "server",
                     usUsedStackSize,
                     NULL,
                     tskIDLE_PRIORITY,
                     NULL );
            xTasksAlreadyCreated = pdTRUE;
        }
    }
}

UBaseType_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
    static UBaseType_t ulNextRand = 0x48763;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *( pulNumber ) = uxRand();
    return pdTRUE;
}

const char * pcApplicationHostnameHook( void )
{
    /* Assign the name "FreeRTOS" to this network node.  This function will
        * be called during the DHCP: the machine will be registered with an IP
        * address plus this name. */
    return "cystick";
}

BaseType_t xApplicationDNSQueryHook_Multi(struct xNetworkEndPoint * pxEndPoint, const char * pcName )
{
    BaseType_t xReturn;
    /* Determine if a name lookup is for this node.  Two names are given
        * to this node: that returned by pcApplicationHostnameHook() and that set
        * by mainDEVICE_NICK_NAME. */
    if( strcasecmp( pcName, pcApplicationHostnameHook() ) == 0 )
    {
        xReturn = pdPASS;
    }
    else if( strcasecmp( pcName, "cystick" ) == 0 )
    {
        xReturn = pdPASS;
    }
    else
    {
        xReturn = pdFAIL;
    }

    return xReturn;
}
extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    return uxRand();
}