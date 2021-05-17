#ifndef __EVENT_H__
#define __EVENT_H__

// #define EVENT_DEBUG_INFO

#include <algorithm>
#ifdef EVENT_DEBUG_INFO
#include <iostream>
#endif
#include <unordered_map>

#include "FastDelegate.h"

using namespace fastdelegate;

template <class>
class EventHandler;

template <class R, class... Args>
class EventHandler<R(Args...)> {
   private:
    using Delegate = FastDelegate<R(Args...)>;
    std::unordered_map<std::string, Delegate> ownedDelegates;

   public:
    EventHandler() = default;

    EventHandler(const char* id, Delegate delegate)
        : ownedDelegates{{id, delegate}} {}

    EventHandler(EventHandler&& handler) : 
        ownedDelegates{std::move(handler.ownedDelegates)} {}

    EventHandler(const EventHandler& handler) : 
        ownedDelegates{handler.ownedDelegates} {}

    ~EventHandler() = default;

    void Add(const char* id, Delegate delegate) {
        ownedDelegates.emplace(id, delegate);
    }

    void Remove(const char* id) {
        ownedDelegates.erase(id);
    }

    void CallAll(Args... args) {
        for (auto i{ownedDelegates.begin()}; i != ownedDelegates.end(); ++i) {
            (*i).second(args...);
        }
    }
};

template <class>
class Event;

/**
 * @brief 
 * 
 * @tparam R Return type
 * @tparam Args Function arguments
 */
template <class R, class... Args>
class Event<R(Args...)> {
   private:
    using Delegate = FastDelegate<R(Args...)>;
    std::unordered_map<void*, EventHandler<R(Args...)>> listeners;
    std::unordered_map<std::string, Delegate> freeListeners;

   public:
    Event() = default;

    /**
     * @brief Subscribes a member function to be called by an event.
     *        
     * @tparam Invoker The instance type to call the member function
     * @tparam Type The type containing the member function
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     * @param invoker The instance owning the member function
     */
    template <class Invoker, class Type>
    void Subscribe(const char* id, R (Type::*func)(Args... args), Invoker* invoker) {
        Delegate deleg{invoker, func};

        auto found{listeners.find(invoker)};

        if (found != listeners.end()) {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Listener [" << id << "] updated.\n";
#endif
            (*found).second.Add(id, deleg);
        } else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "New listener [" << id << "] added.\n";
#endif
            listeners.emplace(invoker, EventHandler<R(Args...)>{id, deleg});
        }
    }

    /**
     * @brief Subscribes a member function to be called by an event.
     *        
     * @tparam Invoker The instance type to call the member function
     * @tparam Type The type containing the member function
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     * @param invoker The instance owning the member function
     */
    template <class Invoker, class Type>
    void Subscribe(const char* id, R (Type::*func)(Args... args) const, Invoker* invoker) {
        Delegate deleg{invoker, func};

        auto found{listeners.find(invoker)};

        if (found != listeners.end()) {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Listener [" << id << "] updated.\n";
#endif
            (*found).second.Add(id, deleg);
        } else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "New listener [" << id << "] added.\n";
#endif
            listeners.emplace(invoker, EventHandler<R(Args...)>{id, deleg});
        }
    }

    /**
     * @brief Subscribes a free function to be called by an event
     * 
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     */
    void Subscribe(const char* id, R (*func)(Args... args)) {
        Delegate deleg{func};

        auto found{freeListeners.find(id)};

        // assert(found == freeListeners.end() 
        //     && "Free listener already registered.");

        if (found == freeListeners.end()) {
#ifdef EVENT_DEBUG_INFO
            std::cout << "New free listener [" << id << "] added.\n";
#endif
            freeListeners.emplace(id, deleg);
        } 
#ifdef EVENT_DEBUG_INFO
        else {
            std::cout << "Free listener [" << id << "] already registered.\n";
        }
#endif
    }

    /**
     * @brief Unsubscribes a member function from an event
     * 
     * @tparam Invoker 
     * @param id Unique identifier of the subscribed function
     * @param invoker The instance owning the member function
     */
    template <class Invoker>
    void Unsubscribe(const char* id, Invoker* invoker) {
        try {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Removing " << id << "...\n";
#endif
            EventHandler<R(Args...)>& listener{listeners.at(invoker)};
            listener.Remove(id);
        } catch (std::exception ex) {
            std::cout << "Exception Unsubscribing " << id << ": " << ex.what() << '\n';
        }
    }

    /**
     * @brief Unsubscribes a free function from an event
     * 
     * @param id Unique identifier of the subscribed function
     */
    void Unsubscribe(const char* id) {
#ifdef EVENT_DEBUG_INFO
        std::cout << "Free listener removed [" << id << "]\n";
#endif
        freeListeners.erase(id);
    }

    template <class Invoker>
    void RemoveListener(Invoker* invoker) {
        listeners.erase(invoker);
    }

    /**
     * @brief 
     * 
     * @param args 
     */
    void Invoke(Args... args) {
        for (auto i{listeners.begin()}; i != listeners.end(); ++i) {
            (*i).second.CallAll(args...);
        }
        for (auto i{freeListeners.begin()}; i != freeListeners.end(); ++i) {
            (*i).second(args...);
        }
    }

    /**
     * @brief 
     * 
     * @param args 
     */
    void operator()(Args... args) {
        Invoke(args...);
    }
};

#endif  // __EVENT_H__