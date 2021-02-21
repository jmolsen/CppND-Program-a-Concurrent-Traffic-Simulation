#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T &&msg);

private:
    std::mutex _mutex;
    std::condition_variable _condition;
    std::deque<T> _queue;
};

enum TrafficLightPhase { red, green };

class TrafficLight : public TrafficObject
{
public:

    // constructor / desctructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void simulate();
    void waitForGreen();

private:
    // typical behaviour methods
    [[noreturn]] void cycleThroughPhases();

    std::condition_variable _condition;
    std::mutex _mutex;
    MessageQueue<TrafficLightPhase> _msgQueue;
    TrafficLightPhase _currentPhase;
};

#endif