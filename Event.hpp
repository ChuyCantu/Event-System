#ifndef __EVENT_H__
#define __EVENT_H__

// #define EVENT_DEBUG_INFO

#include <functional>
#include <string>
#include <unordered_map>
#ifdef EVENT_DEBUG_INFO
#include <iostream>
#include <string_view>

// Credits to this function: https://stackoverflow.com/a/56766138/576911
// from thread https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
template <typename T>
#if (__cplusplus == 201703L) // C++17
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
#else // C++17
const char* type_name() {
    typedef typename std::remove_reference<T>::type TR;
    return typeid(TR).name();
#endif // C++17
}
#endif

//* Utility functions to create std::functions without std::placeholder_1 , ...
template<class Type, class R, class... Args>
std::function<R(Args...)> EasyBind(R(Type::*func)(Args... args), Type* invoker) {
    return [=](auto&&... args) {
        return (invoker->*func)(std::forward<decltype(args)>(args)...);
    };
}

template<class Type, class R, class... Args>
std::function<R(Args...)> EasyBind(R(Type::*func)(Args... args) const, const Type* invoker) {
    return [=](auto&&... args) {
        return (invoker->*func)(std::forward<decltype(args)>(args)...);
    };
}

template <class>
class Event;

/**
 * @brief Event that can call all of its subscribers
 * 
 * @tparam R Return type
 * @tparam Args Function arguments
 */
template <class R, class... Args>
class Event<R(Args...)> {
private:
    using Delegate = std::function<R(Args...)>;
    using FuncMap = std::unordered_map<std::string, Delegate>;
    std::unordered_map<void*, FuncMap> listeners;
    std::unordered_map<const void*, FuncMap> constListeners;

public:
    Event() = default;

    /**
     * @brief Subscribes a member function to the event.
     *        
     * @tparam Invoker The instance type to call the member function
     * @tparam Type The type containing the member function
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     * @param invoker The instance owning the member function
     */
    template <class Invoker, class Type>
    void Subscribe(const std::string& id, R(Type::*func)(Args... args), Invoker* invoker) {
        Delegate deleg {std::move(EasyBind(func, invoker))};

        auto found{listeners.find(invoker)}; 

        if (found != listeners.end()) {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Listener [" << id << ", " << type_name<Type>() << "] updated.";
#endif
            auto iter {found->second.find(id)};
            if (iter != found->second.end()) {
                iter->second = std::move(deleg);
#ifdef EVENT_DEBUG_INFO
                std::cout << " *Function was replaced.\n";
#endif
            }
            else { 
                found->second.emplace(id, std::move(deleg));
#ifdef EVENT_DEBUG_INFO
                std::cout << '\n';
#endif
            }
        } else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "New listener [" << id << ", " << type_name<Type>() << "] registered.\n";
#endif
            listeners.emplace(invoker, FuncMap{{id, {std::move(deleg)}}});
        }
    }

    /**
     * @brief Subscribes a const member function to the event.
     *        
     * @tparam Invoker The instance type to call the member function
     * @tparam Type The type containing the member function
     * @param id Unique identifier of the subscribing function
     * @param func The const function to call when the event is raised
     * @param invoker The instance owning the member function
     */
    template <class Invoker, class Type>
    void Subscribe(const std::string& id, R (Type::*func)(Args... args) const, const Invoker* invoker) { 
        Delegate deleg {std::move(EasyBind(func, invoker))};

        auto found{constListeners.find(invoker)};

        if (found != constListeners.end()) {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Listener [" << id << ", " << type_name<Type>() << "] updated.";
#endif
            auto iter {found->second.find(id)};
            if (iter != found->second.end()) {
                iter->second = std::move(deleg);
#ifdef EVENT_DEBUG_INFO
                std::cout << " *Function was replaced.\n";
#endif
            }
            else {
                found->second.emplace(id, std::move(deleg));
#ifdef EVENT_DEBUG_INFO
                std::cout << '\n';
#endif
            }
        } else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "New listener [" << id << ", const " << type_name<Type>() << "] registered.\n";
#endif
            constListeners.emplace(invoker, FuncMap{{id, {std::move(deleg)}}});
        }
    }

    /**
     * @brief Subscribes free function, lambda or capture lambda to the event
     * 
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     */
    void Subscribe(const std::string& id, std::function<R(Args...)>&& func) {
        auto found{listeners.find(nullptr)};

        if (found != listeners.end()) {
            auto iter {found->second.find(id)};
            if (iter != found->second.end()) {
                iter->second = std::move(func);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Free listener [" << id << "] updated. Function replaced.\n";
#endif
            }
            else {    
                found->second.emplace(id, func);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Free listener [" << id << "] registered.\n";
#endif
            }
        }
        else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Free listener [" << id << "] registered.\n";
#endif
            listeners.emplace(nullptr, FuncMap{{id, {std::move(func)}}});
        }
    }

    /**
     * @brief Subscribes a free function or a std::function reference to the event
     * 
     * @param id Unique identifier of the subscribing function
     * @param func The function to call when the event is raised
     */
    void Subscribe(const std::string& id, std::function<R(Args...)>& func) {
        auto found{listeners.find(nullptr)};

        if (found != listeners.end()) {
            auto iter{found->second.find(id)};
            if (iter != found->second.end()) {
                iter->second = func;
#ifdef EVENT_DEBUG_INFO
                std::cout << "Free listener [" << id << "] updated. Function replaced.\n";
#endif
            }
            else {
                found->second.emplace(id, func);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Free listener [" << id << "] registered.\n";
#endif
            }
        } else {
#ifdef EVENT_DEBUG_INFO
            std::cout << "Free listener [" << id << "] registered.\n";
#endif
            listeners.emplace(nullptr, FuncMap{{id, {func}}});
        }
    }

    /**
     * @brief Unsubscribes a member function from an event
     * 
     * @tparam Invoker 
     * @param id Unique identifier of the subscribed function
     * @param invoker The instance owning the member function
     */
    template <class Invoker>
    void Unsubscribe(const std::string& id, Invoker* invoker) {    
        auto found {listeners.find(invoker)};

        if (found != listeners.end()) {
            auto iter {found->second.find(id)};
            if (iter != found->second.end()) {
                found->second.erase(iter);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Listener [" << id << ", " << type_name<decltype(invoker)>() << "] removed.\n";
#endif
            }
            if (found->second.empty()) {
                listeners.erase(found);
            }
        }
#ifdef EVENT_DEBUG_INFO
        else {
            std::cout << "No member function [" << id << ", " << type_name<decltype(invoker)>() << "] was found.\n";
        }
#endif
    }

    /**
     * @brief Unsubscribes a const member function from an event
     * 
     * @tparam const Invoker 
     * @param id Unique identifier of the subscribed const function
     * @param invoker The instance owning the member function
     */
    template <class Invoker>
    void Unsubscribe(const std::string& id, const Invoker* invoker) {
        auto found{constListeners.find(invoker)};

        if (found != constListeners.end()) {
            auto iter{found->second.find(id)};
            if (iter != found->second.end()) {
                found->second.erase(iter);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Listener [" << id << ", " << type_name<decltype(invoker)>() << "] removed.\n";
#endif
            }
            if (found->second.empty()) {
                listeners.erase(found);
            }
        }
#ifdef EVENT_DEBUG_INFO
        else {
            std::cout << "No member function [" << id << ", " << type_name<decltype(invoker)>() << "] was found.\n";
        }
#endif
    }

        /**
     * @brief Unsubscribes a free function/lambda from an event
     * 
     * @param id Unique identifier of the subscribed function/lambda
     */
    void Unsubscribe(const std::string& id) {
        auto found{listeners.find(nullptr)};

        if (found != listeners.end()) {
            auto iter{found->second.find(id)};
            if (iter != found->second.end()) {
                found->second.erase(iter);
#ifdef EVENT_DEBUG_INFO
                std::cout << "Free listener [" << id << "] removed.\n";
#endif
            }
            if (found->second.empty()) {
                listeners.erase(found);
            }
        }
#ifdef EVENT_DEBUG_INFO
        else {
            std::cout << "No free function [" << id << "] was found.\n";
        }
#endif
    }

    /**
     * @brief Unsubscribes all functions owned by the invoker instance from this event
     * 
     * @param invoker Instance subscribed to this event
     */
    template <class Invoker>
    void RemoveListener(Invoker* invoker) {
        auto found {listeners.find(invoker)};
        if (found != listeners.end()) {
            listeners.erase(found);
#ifdef EVENT_DEBUG_INFO
            std::cout << "All member functions from an instance of type <" << type_name<decltype(invoker)>() << "> were removed.\n";
#endif
        }
    }

    /**
     * @brief Unsubscribes all functions owned by the invoker const instance from this event
     * 
     * @param invoker Const instance subscribed to this event
     */
    template <class Invoker>
    void RemoveListener(const Invoker* invoker) {
        auto found{constListeners.find(invoker)};
        if (found != constListeners.end()) {
            constListeners.erase(found);
#ifdef EVENT_DEBUG_INFO
            std::cout << "All member functions from an instance of type <" << type_name<decltype(invoker)>() << "> were removed.\n";
#endif
        }
    }

    /**
     * @brief Unsubscribes all free functions/lambdas from this event
     */
    void RemoveFreeFunctions() {
        auto found{listeners.find(nullptr)};
        if (found != listeners.end()) {
            listeners.erase(found);
#ifdef EVENT_DEBUG_INFO
            std::cout << "All free functions were removed.\n";
#endif
        }
    }

    // TODO: Check if args should be lvalues or not. 
    /**
     * @brief Calls all subscribed functions 
     * 
     * @param args 
     */
    void Invoke(Args... args) {
#ifdef EVENT_DEBUG_INFO
        std::cout << "\n>> Calling all listeners...\n\n";
#endif
        for (auto& listener : listeners) {
            for (auto& func : listener.second) {
                func.second(std::forward<decltype(args)>(args)...);
            }
        }

        for (auto& listener : constListeners) {
            for (auto& func : listener.second) {
                func.second(std::forward<decltype(args)>(args)...);
            }
        }
    }

    /**
     * @brief Calls all subscribed functions (this is equivalent to Invoke())
     * 
     * @param args 
     */
    void operator()(Args... args) {
        Invoke(std::forward<decltype(args)>(args)...);
    }

};
#endif // __EVENT_H__