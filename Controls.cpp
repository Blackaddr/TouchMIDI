/*
 * Controls.cpp
 *
 *  Created on: Apr. 10, 2020
 *      Author: blackaddr
 */
#include "Controls.h"

constexpr unsigned CONTROL_SAMPLE_PERIOD_MS = 10;

std::mutex                controlInputMutex;
std::queue<ControlEvent> *controlInputQueue = new std::queue<ControlEvent>();

static void addToControlQueue(ControlEvent &controlEvent)
{
    {
        std::lock_guard<std::mutex> lock(controlInputMutex);
        if (controlInputQueue->size() >= CONTROL_QUEUE_MAX_SIZE) {
            // queue is full, pop the oldest then add
            controlInputQueue->pop();
        }
        controlInputQueue->emplace(controlEvent);
    }
}

void processControls(void *controlPtr)
{
    // it's okay to crash here if the pointers invalid to avoid failing silently
    Controls &controls = *(reinterpret_cast<Controls*>(controlPtr));

    while(true) {
        // Check the encoders
        for ( auto it = controls.m_encoders.begin(); it != controls.m_encoders.end(); ++it) {
            int encoderAdjust = (*it).getChange();
            if (encoderAdjust) {
                ControlEvent controlEvent = {
                        .eventType = ControlEventType::ENCODER,
                        .value = encoderAdjust
                };
                addToControlQueue(controlEvent);

            } // encoder adjust nonzeor
        } // end encoder for loop

        // Check the switches
        for ( auto it = controls.m_switches.begin(); it != controls.m_switches.end(); ++it) {
            if ((*it).update() && (*it).fallingEdge()) {
                ControlEvent controlEvent = {
                        .eventType = ControlEventType::SWITCH,
                        .value = 1
                };

                addToControlQueue(controlEvent);
            }
        }

        delay(CONTROL_SAMPLE_PERIOD_MS);
    }
}

bool getNextControlEvent(ControlEvent& controlEvent)
{
    bool isEventAvailable = false;
    {
        std::lock_guard<std::mutex> lock(controlInputMutex);
        controlEvent = controlInputQueue->front();
        if (!controlInputQueue->empty()) {
            controlInputQueue->pop();
            isEventAvailable = true;
        }
    } // mutex unlocks
    return isEventAvailable;
}
