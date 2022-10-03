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
#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include "state_machine_types.h"

#include <stdlib.h>

typedef enum
{
    EV_INVALID,
    EV_ENTRY,
    EV_EXIT,
    EV_TIMER_RUNNING,
    EV_MAX,
} EVENT_T;

typedef enum
{
    TMR_SINGLE,
    TMR_REPEAT,
} TIMER_TYPE_T;

typedef struct
{
    TIMER_TYPE_T type;
    bool running;
    uint16_t ticks;
    uint16_t current_tick;
    EVENT_T ev;
} TIMER_T;

typedef uint8_t (*STATE_FUNC)(void *ptr, EVENT_T e, void *data);

typedef struct
{
    STATE_FUNC active_state_fn;
    STATE_FUNC next_state_fn;
    STATE_FUNC prev_state_fn;
    EVENT_T ev;
    EVENT_T nextev;
    uint8_t status;
    void *data;
    TIMER_T timer;
} STATE_T;

void StateMachine_Init(STATE_T *sptr, STATE_FUNC init_state_func, void *data);
uint8_t StateMachine_Run(STATE_T *sptr);
void StateMachine_Transition(STATE_T *sptr, STATE_FUNC fn);
void StateMachine_EmitEvent(STATE_T *sptr, uint8_t e);
void StateMachine_SetStatus(STATE_T *sptr, uint8_t s);
uint8_t StateMachine_GetStatus(STATE_T *sptr);
bool StateMachine_IsCurrentState(STATE_T *sptr, STATE_FUNC fn);

void StateMachine_StartTimer(STATE_T *sptr, TIMER_TYPE_T type, uint16_t ticks, uint8_t e);
void StateMachine_CancelTimer(STATE_T *sptr);

#endif // STATE_MACHINE_H_
