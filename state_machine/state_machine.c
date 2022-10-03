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

static void StateMachine_TimerProcessTick(STATE_T *sptr);

void StateMachine_Init(STATE_T *sptr, STATE_FUNC init_state_func, void *data) {
    sptr->active_state_fn = init_state_func;
    sptr->next_state_fn = NULL;
    sptr->ev = EV_ENTRY;
    sptr->nextev = EV_INVALID;
    sptr->status = 0;
    sptr->data = data;
    sptr->timer.type = TMR_SINGLE;
    sptr->timer.running = FALSE;
    sptr->timer.ticks = 0;
    sptr->timer.current_tick = 0;
    sptr->timer.ev = EV_INVALID;
}

uint8_t StateMachine_Run(STATE_T *sptr) {
    StateMachine_TimerProcessTick(sptr);

    if(sptr->ev == EV_EXIT) {
        sptr->nextev = EV_ENTRY;
    }

    uint8_t status = sptr->active_state_fn(sptr, sptr->ev, sptr->data);

    if (sptr->nextev == EV_ENTRY)
    {
        sptr->prev_state_fn = sptr->active_state_fn;
        sptr->active_state_fn = sptr->next_state_fn;
        sptr->next_state_fn = NULL;
        sptr->ev = sptr->nextev;
        sptr->nextev = EV_INVALID;
    }
    return status;
}

void StateMachine_Transition(STATE_T *sptr, STATE_FUNC fn) {
    sptr->next_state_fn = fn;
    sptr->ev = EV_EXIT;
    sptr->nextev = EV_INVALID;
}


void StateMachine_EmitEvent(STATE_T *sptr, uint8_t e) {
    sptr->ev = (EVENT_T)e;
}

void StateMachine_SetStatus(STATE_T *sptr, uint8_t s) {
    sptr->status = s;
}

uint8_t StateMachine_GetStatus(STATE_T *sptr) {
    return sptr->status;
}

bool StateMachine_IsCurrentState(STATE_T *sptr, STATE_FUNC fn) {
    bool ret = sptr->active_state_fn == fn ? TRUE : FALSE;
    return ret;
}

void StateMachine_StartTimer(STATE_T *sptr, TIMER_TYPE_T type, uint16_t ticks, uint8_t e) {
    sptr->timer.current_tick = 0;
    sptr->timer.type = type;
    sptr->timer.ticks = ticks;
    sptr->timer.running = TRUE;
    sptr->timer.ev = (EVENT_T)e;
}

void StateMachine_CancelTimer(STATE_T *sptr) {
    sptr->timer.running = FALSE;
}

static void StateMachine_TimerProcessTick(STATE_T *sptr) {
    if(sptr->timer.running == TRUE)
    {
        sptr->timer.current_tick++;
        if (sptr->timer.current_tick == sptr->timer.ticks)
        {
            StateMachine_EmitEvent(sptr, sptr->timer.ev);
            if (sptr->timer.type == TMR_REPEAT)
            {
                StateMachine_StartTimer(sptr, sptr->timer.type, sptr->timer.ticks, sptr->timer.ev);
            }
            else
            {
                StateMachine_CancelTimer(sptr);
            }
        }
        else
        {
            StateMachine_EmitEvent(sptr, EV_TIMER_RUNNING);
        }
    }
}
