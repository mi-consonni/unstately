#include <unstately/unstately.h>

// Define the state machine context.
struct Context {};

// Define the events.
struct CoinInserted {};
struct ArmPushed {};

// Define some useful shortcuts.
using State = unstately::UniqueState<Context, CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::unique_ptr::make_state_ptr;

// Every state must derive from `State` and shall, thus,
// implement entry/exit actions as well as all the event handlers
// (possibly empty).
class Locked : public State {
public:
    void entry(Context&) override { /* ... */ }
    void exit(Context&) override { /* ... */ }
    void handle(Context&, const CoinInserted&) override;
    void handle(Context&, const ArmPushed&) override { /* ... */ }
};

class Unlocked : public State {
public:
    void entry(Context&) override { /* ... */ }
    void exit(Context&) override { /* ... */ }
    void handle(Context&, const CoinInserted&) override { /* ... */ }
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
    // Create the state machine with an initial state.
    auto sm = StateMachine{Context{}, make_state_ptr<Locked>()};
    // Dispatch the events.
    sm.dispatch(CoinInserted{});
    sm.dispatch(ArmPushed{});
}
