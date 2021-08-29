# Event-System
Simple event system (observer pattern) for C++ with support for "free" functions, member functions and lambdas.

# Usage
You can declare an Event with a similar syntax as an std::function.
You can also define **EVENT_DEBUG_INFO** to debug in console when a subscriber is being created, removed, updated, or ignored in case one with the same id and same instance (for member functions) is already subscribed to the event. This information is printed in the format **[functionID, \<owner>\*]**. For a version without the bloated code to print debug info use the header file inside the folder "No debug info".
<div style="text-align: right"><sub>*Only when the function is a member of a class.</sub> </div>

```cpp
class Foo {
    public:
    void MemberFunction(int x) {
        std::cout << "This is a member function - " << x << '\n';
    }
};

void FreeFunction(int x) {
    std::cout << "This is a free function - " << x << '\n';
}

int main() {
    // Example of and event for functions that return void and one integer as argument.
    Event<void(int)> event;

    event.Subscribe("FreeFunction", FreeFunction);
    // event.Unsubscribe("FreeFunction");
    Foo foo;
    event.Subscribe("MemberFunction", Foo::MemberFunction, &foo); // Note that you have to tell the event the class where
    								  // the member function is located, in this case Foo.
                                                                  // This can be skipped if you're subscribing the function
                                                                  // within the scope of the class that contains it
								  // event.Unsubscribe("MemberFunction", &foo);
    
    // Invoking the event
    event.Invoke(7); // You can use the overloaded operator (), as event(7), which will have the same effect.
}

// Output:
// This is a member function - 7
// This is a free function - 7
```

**NOTE:** This system currently only supports *void* as return type. This system is also not thread safe since I use it mainly in games with no multi threated events. 