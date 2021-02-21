#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    // modify queue while under lock
    std::unique_lock<std::mutex> uLock(_mutex);
    // pass lock to condition
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    // get last message and remove from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // expect RVO to prevent copying
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // modify queue while under lock
    std::lock_guard<std::mutex> uLock(_mutex);
    // add msg to queue and notify client
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_msgQueue.receive() == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
[[noreturn]] void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    std::random_device randomDevice;     // one-time seed for generator
    std::mt19937 rng(randomDevice());    // random number generator using Mersenne-Twister
    std::uniform_int_distribution<int> distribution(4000000, 6000000); // between 4 and 6 seconds (in microseconds)
    auto random_duration = distribution(rng);

    auto duration = 0;
    std::chrono::high_resolution_clock::time_point prevTime;
    std::chrono::high_resolution_clock::time_point currTime = std::chrono::high_resolution_clock::now();
    while (true)
    {

        prevTime = currTime;
        currTime = std::chrono::high_resolution_clock::now();
        duration += std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevTime).count();
        if (duration >= random_duration)
        {
            if (_currentPhase == TrafficLightPhase::green)
            {
                _currentPhase = TrafficLightPhase::red;
            }
            else
            {
                _currentPhase = TrafficLightPhase::green;
            }
            random_duration = distribution(rng);
            duration = 0;
        }
        _msgQueue.send(std::move(_currentPhase));

        // sleep to prevent overuse of the CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}