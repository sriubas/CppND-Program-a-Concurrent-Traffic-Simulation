#include <iostream>
#include <random>
#include <stdlib.h>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
 
template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);

    _condition.wait(lock, [this]{return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() :_currentPhase(TrafficLightPhase::red) {
        _type = ObjectType::objectTrafficLight;
}

void TrafficLight::waitForGreen()
{
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        TrafficLightPhase phase = _lightQueue.receive();
        if (phase == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase && phase)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _currentPhase = std::move(phase);
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
 
    srand (time(NULL));
    int cycleLength = rand() % 2000 + 4000;

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();

    while(true){
        // compute time difference to stop watch
        long timeSinceLastUpdate =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                    - lastUpdate).count(); 
        if (timeSinceLastUpdate > cycleLength){ 
            switch (getCurrentPhase()){
                case TrafficLightPhase::green:
                    setCurrentPhase(TrafficLightPhase::red);
                    _lightQueue.send(TrafficLightPhase::red); 
                    break;
                case red:
                    setCurrentPhase(TrafficLightPhase::green);
                    _lightQueue.send(TrafficLightPhase::green); 
                    break;
            }
            lastUpdate = std::chrono::system_clock::now();
            cycleLength = rand() % 2000 + 4000;
        }
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

