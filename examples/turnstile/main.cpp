#include <functional>
#include <iostream>
#include <variant>

#include <unstately/unstately.h>

// By default, this example creates states on the heap.
// Uncomment the following line to create states with static storage instead.
// #define UNSTATELY_EXAMPLE_TURNSTILE_STATIC

// Define the events.
struct CoinInserted {};
struct ArmPushed {};
using Event = std::variant<ArmPushed, CoinInserted>;

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

    void push_event(Event&&) {
        // Push into the event queue.
    }

    Event pop_event() {
        // Pop from the event queue (here we use a mock version).
        static unsigned int counter = 0;
        auto event = (counter % 2 == 0) ? Event{ArmPushed{}} : Event{CoinInserted{}};
        counter += 1;
        return event;
    }
};

// Define some useful shortcuts.
#ifndef UNSTATELY_EXAMPLE_TURNSTILE_STATIC
using State = unstately::UniqueState<CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::unique_ptr::make_state_ptr;
#else
using State = unstately::StaticState<CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::static_ptr::make_state_ptr;
#endif

// Every state must derive from `State` and shall, thus,
// implement entry/exit actions as well as all the event handlers
// (possibly empty).
class Locked : public State {
public:
    explicit Locked(Context& context) : context_{context} {}

    void entry() override {
        context_.get().lock_arm();
    }

    void exit() override {}

    void handle(const CoinInserted&) override;

    void handle(const ArmPushed&) override {
        context_.get().beep();
    }

private:
    std::reference_wrapper<Context> context_;
};

class Unlocked : public State {
public:
    explicit Unlocked(Context& context) : context_{context} {}

    void entry() override {
        context_.get().unlock_arm();
    }

    void exit() override {}

    void handle(const CoinInserted&) override {}

    void handle(const ArmPushed&) override {
        // Request transition to another state. The transition will be executed
        // by the state machine when this function returns.
        request_transition(make_state_ptr<Locked>(context_));
    }

private:
    std::reference_wrapper<Context> context_;
};

void Locked::handle(const CoinInserted&) {
    request_transition(make_state_ptr<Unlocked>(context_));
}

int main() {
    static Context context{};

    // Create the state machine with an initial state.
    auto sm = StateMachine{make_state_ptr<Locked>(context)};

    // Dispatch the events.
    for (int i = 0; i < 3; ++i) {
        auto event = context.pop_event();
        std::visit([&](auto&& e) { sm.dispatch(e); }, event);
    }
}
