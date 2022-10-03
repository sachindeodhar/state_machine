/*
* MIT License
* 
* Copyright (c) [2018] [Sachin Deodhar]
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "state_machine.h"

#include <stdio.h>
#include <unistd.h>

#if defined (__GNUC__)
#define LOGF(x, ...) printf(x, ##__VA_ARGS__)
#else
#define LOGF(x, ...) do {} while(0)
#endif

static uint8_t App_Startup(void *ptr, EVENT_T e, void *data);
static uint8_t App_ConnectWifi(void *ptr, EVENT_T e, void *data);
static uint8_t App_AuthenticatingWifi(void *ptr, EVENT_T e, void *data);
static uint8_t App_GettingWifiIP(void *ptr, EVENT_T e, void *data);
static uint8_t App_WifiBase(void *ptr, EVENT_T e, void *data);
static uint8_t App_Ready(void *ptr, EVENT_T e, void *data);

int main(int argc, char **argv)
{
    STATE_T state_data;

    StateMachine_Init(&state_data, App_Startup, NULL);

    while(1)
    {
        StateMachine_Run(&state_data);
        sleep(1);
    }
    return 0;
}

typedef enum { EV_STARTUP_WAIT_TIMEOUT = EV_MAX } EVENT_APP_STARTUP_T;
static uint8_t App_Startup(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_ENTRY:
            LOGF("App_Startup - EV_ENTRY\n");
            StateMachine_StartTimer(sptr, TMR_SINGLE, 5, EV_STARTUP_WAIT_TIMEOUT); //wait 5 seconds
            break;
        case EV_EXIT:
            LOGF("App_Startup - EV_EXIT\n");
            break;
        case EV_TIMER_RUNNING:
            LOGF("App_Startup - EV_TIMER_RUNNING - Waiting for timeout\n");
            break;
        case EV_STARTUP_WAIT_TIMEOUT:
            LOGF("App_Startup - EV_STARTUP_WAIT_TIMEOUT\n");
            StateMachine_Transition(sptr, App_ConnectWifi);
            break;
        default:
            LOGF("App_Startup - DEFAULTS\n");
            break;
    }
    return 0;
}

typedef enum { EV_WIFI_DISCONNECTED = EV_MAX, EV_IP_CONFIGURED, EV_APP_WIFI_BASE_MAX } EVENT_APP_WIFI_BASE;
static uint8_t App_WifiBase(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_WIFI_DISCONNECTED:
            LOGF("App_WifiBase - EV_WIFI_DISCONNECTED\n");    
            break;
        case EV_IP_CONFIGURED:
            LOGF("App_WifiBase - EV_IP_CONFIGURED\n");
            StateMachine_Transition(sptr, App_Ready);
            break;
        case EV_ENTRY:
            LOGF("App_WifiBase - EV_ENTRY\n");
            break;
        case EV_EXIT:
            LOGF("App_WifiBase - EV_EXIT\n");
            break;
        default:
            LOGF("App_WifiBase - DEFAULTS\n");
            break;
    }
    return 0;
}

typedef enum { EV_CHECKING_NETWORK = EV_APP_WIFI_BASE_MAX } EVENT_APP_CONNECT_WIFI;
static uint8_t App_ConnectWifi(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_CHECKING_NETWORK:
            LOGF("App_ConnectWifi - EV_CHECKING_NETWORK\n");
            StateMachine_Transition(sptr, App_AuthenticatingWifi);
            return 0;
        case EV_ENTRY:
            LOGF("App_ConnectWifi - EV_ENTRY\n");
            StateMachine_EmitEvent(sptr, EV_CHECKING_NETWORK);
            break;
        case EV_EXIT:
            LOGF("App_ConnectWifi - EV_EXIT\n");
            break;
        default:
            LOGF("App_ConnectWifi - DEFAULTS\n");
            break;
    }
    return App_WifiBase(ptr, e, data);
}

typedef enum { EV_AUTHENTICATING = EV_APP_WIFI_BASE_MAX } EVENT_APP_AUTHENTICATING_WIFI;
static uint8_t App_AuthenticatingWifi(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_AUTHENTICATING:
            LOGF("App_AuthenticatingWifi - EV_AUTHENTICATING\n");
            StateMachine_Transition(sptr, App_GettingWifiIP);
            break;
        case EV_ENTRY:
            LOGF("App_AuthenticatingWifi - EV_ENTRY\n");
            StateMachine_EmitEvent(sptr, EV_AUTHENTICATING);
            break;
        case EV_EXIT:
            LOGF("App_AuthenticatingWifi - EV_EXIT\n");
            break;
        default:
            LOGF("App_AuthenticatingWifi - DEFAULTS\n");
            break;
    }
    return App_WifiBase(ptr, e, data);
}

typedef enum { EV_WAITING_FOR_IP_TIMER = EV_APP_WIFI_BASE_MAX } EVENT_APP_GETTING_IP;
typedef struct {
    uint8_t count;
} GETTING_IP_STATE_DATA_T;
GETTING_IP_STATE_DATA_T *get_ip_state_data;
static uint8_t App_GettingWifiIP(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_IP_CONFIGURED:
            LOGF("App_GettingWifiIP - EV_IP_CONFIGURED\n");
            break;    
        case EV_ENTRY:
            LOGF("App_GettingWifiIP - EV_ENTRY\n");
            StateMachine_StartTimer(sptr, TMR_REPEAT, 1, EV_WAITING_FOR_IP_TIMER);
            get_ip_state_data = (GETTING_IP_STATE_DATA_T *) malloc(sizeof(GETTING_IP_STATE_DATA_T));
            get_ip_state_data->count = 0;
            break;
        case EV_EXIT:
            LOGF("App_GettingWifiIP - EV_EXIT\n");
            free(get_ip_state_data);
            break;
        case EV_WAITING_FOR_IP_TIMER:
            LOGF("App_GettingWifiIP - EV_WAITING_FOR_IP_TIMER\n");
            get_ip_state_data->count++;
            if(get_ip_state_data->count > 5) {
                StateMachine_CancelTimer(sptr);
                StateMachine_EmitEvent(sptr, EV_IP_CONFIGURED);
            }
            break;
        default:
            LOGF("App_GettingWifiIP - DEFAULTS\n");
            break;
    }
    return App_WifiBase(ptr, e, data);
}

typedef enum { EV_WAITING_FOR_CARD = EV_MAX } EVENT_APP_READY_T;
static uint8_t App_Ready(void *ptr, EVENT_T e, void *data) {
    STATE_T *sptr = (STATE_T *)ptr;
    switch(e) {
        case EV_WAITING_FOR_CARD:
            LOGF("App_Ready - EV_WAITING_FOR_CARD\n");
            break;
        case EV_ENTRY:
            LOGF("App_Ready - EV_ENTRY\n");
            StateMachine_EmitEvent(sptr, EV_WAITING_FOR_CARD);
            break;
        case EV_EXIT:
            LOGF("App_Ready - EV_EXIT\n");
            break;
        default:
            LOGF("App_Ready - DEFAULTS\n");
            break;
    }
    return 0;
}



