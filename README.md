# Event-System
Simple event system (observer pattern) for C++ using **Fast Delegate** from Don Clugston (You can learn more about it in [here](https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible)).

# Usage
You can declare an Event with a similar syntax as an std::function, supporting up to 8 function parameters (limitation by current Fast Delegate code, but it's very easy to extend as you wish).
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
**NOTE:** This system currently only supports *void* as return type since the need to have to return something with events is very small, yet the user still have to type *void* so the user can extend the system to allow return types if they wish (and to keep its syntax similar as a std::function and).
