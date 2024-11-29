#include <iostream>
#include <variant>
#include <vector>

#include <unstately/unstately.h>

// By default, this example creates states on the heap.
// Uncomment the following line to create states with static storage instead.
// #define UNSTATELY_EXAMPLE_TURNSTILE_STATIC

// The `Context` class here represents the application-specific
// context in which the state machine acts.
// For example, we can access hardware-related resources.
class Context {
public:
    void lock_arm() {
        std::cout << "Arm is LOCKED" << '\n';
    }

    void unlock_arm() {
        std::cout << "Arm is UNLOCKED" << '\n';
    }

    void beep() {
        std::cout << "Buzzer BEEPED" << '\n';
    }
};

// Define the events.
struct CoinInserted {};
struct ArmPushed {};
using Event = std::variant<ArmPushed, CoinInserted>;

// Define some useful shortcuts.
#ifndef UNSTATELY_EXAMPLE_TURNSTILE_STATIC
using State = unstately::UniqueState<Context, CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::unique_ptr::make_state_ptr;
#else
using State = unstately::StaticState<Context, CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::static_ptr::make_state_ptr;
#endif

// Every state must derive from `State` and shall, thus,
// implement entry/exit actions as well as all the event handlers
// (possibly empty).
class Locked : public State {
public:
    void entry(Context& context) override {
        context.lock_arm();
    }

    void exit(Context&) override {}

    void handle(Context&, const CoinInserted&) override;

    void handle(Context& context, const ArmPushed&) override {
        context.beep();
    }
};

class Unlocked : public State {
public:
    void entry(Context& context) override {
        context.unlock_arm();
    }

    void exit(Context&) override {}

    void handle(Context&, const CoinInserted&) override {}

    void handle(Context&, const ArmPushed&) override {
        // Request transition to another state. The transition will be executed
        // by the state machine when this function returns.
        request_transition(make_state_ptr<Locked>());
    }
};

void Locked::handle(Context&, const CoinInserted&) {
    request_transition(make_state_ptr<Unlocked>());
}

int main() {
    // Emulate an event queue.
    std::vector<Event> event_queue = {
        ArmPushed{},
        CoinInserted{},
        ArmPushed{},
    };

    // Create the state machine with an initial state.
    auto sm = StateMachine{Context{}, make_state_ptr<Locked>()};

    // Dispatch the events.
    for (const auto& event : event_queue) {
        std::visit([&](auto&& e) { sm.dispatch(e); }, event);
    }
}
