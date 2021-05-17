// Test program for events

#include <iostream>
#include "../include/Event.hpp"

class A {
   public:
    std::string name;
    Event<void()> event;
    Event<void(int)> event2;

    A(const char* name) : name{name} {
        // std::cout << "A was created\n";
    }

    void SayHi() {
        std::cout << name << " say hi\n";
    }

    void Damage(int dmg) {
    }

    void InvokeEvents() {
    }
};

class B {
   public:
    std::string name;
    int health;
    A& a;

    B(const char* name, A& a) : name{name}, a{a} {
        // std::cout << "B was created\n";
        health = 10;
        a.event.Subscribe("SayHi", SayHi, this);
        a.event2.Subscribe("TakeDamage", TakeDamage, this);
        //a.event.Unsubscribe("SayHi", this);
    }

    ~B() {
    }

    virtual void SayHi() {
        std::cout << "B: " << name << " say hi\n";
    }

    void TakeDamage(int dmg) {
        health -= dmg;
        std::cout << name << " took " << dmg << " dmg. health: " << health << '\n';
    }
};

class D : public B {
   public:

    D(const char* name, A& a) : B(name, a) {
        // std::cout << "B was created\n";
        health = 10;
        // a.event.Subscribe("SayHi", SayHi, this);
        // a.event2.Subscribe("TakeDamage", TakeDamage, this);
        //a.event.Unsubscribe("SayHi", this);
    }

    ~D() {
    }

    virtual void SayHi() {
        std::cout << "D: " << name << " say hi\n";
    }

    void TakeDamage(int dmg) {
        health -= dmg;
        std::cout << name << " took " << dmg << " dmg. health: " << health << '\n';
    }
};

class C : public D {
   public:

    C(const char* name, A& a) : D(name, a) {
        // std::cout << "C was created\n";
        //a.event.Subscribe("SayHi", SayHi, this);
        a.event.Subscribe("SayHola", SayHola, this);
        //a.event.Unsubscribe("SayHi", this);
        // a.event.Unsubscribe("SayHi", this);
    }

    ~C() {
    }

    void SayHi() override {
        std::cout << "C: " << name << " say hi\n";
    }

    void SayHola() const {
        std::cout << "C: " << name << " say hola\n";
    }
};

void FreeEvent() {
    std::cout << "This is a free function\n";
}



int main() {
    A a{"Juan"};
    //A a2{"mike"};
    B b{"Mike", a};
    C c{"Sara", a};
    B b2{"Mario", a};
    C c2{"Jules", a};
    //a.event.RemoveListener(&c);
    a.event2(1);
    a.event2(3);
    a.event.Subscribe("FreeEvent", FreeEvent);
    a.event.Subscribe("FreeEvent", FreeEvent);
    //a.event.Unsubscribe("FreeEvent");
    a.event();
    return 0;
}