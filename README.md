Unstately
=========

Unstately is a very simple state machine implementation in C++17.

Its aim it is to provide an example of how to write a state machine without switch cases, nor transition tables.
It does not claim to be production-ready.
If you are looking for a more mature, perfomant, and featured state machine implementation, you may want to take a look to [TinyFSM](https://github.com/digint/tinyfsm).

Main features
-------------

* Single-file, header-only library.
* No switch-case, no transition tables.
* Events are dispatched by a vtable lookup.
* States can be instantiated either dynamically or with static lifetime.

Usage at a glance
-----------------

A simple toy example is available under `examples/turnstile`.  
Here is an extract.

```cpp
#include <unstately/unstately.h>

// Define the events.
struct CoinInserted {};
struct ArmPushed {};

// Define some useful shortcuts.
using State = unstately::UniqueState<CoinInserted, ArmPushed>;
using StateMachine = unstately::StateMachine<State>;
using unstately::unique_ptr::make_state_ptr;

// Every state must derive from `State` and shall, thus,
// implement entry/exit actions as well as all the event handlers
// (possibly empty).
class Locked : public State {
public:
    void entry() override { /* ... */ }
    void exit() override { /* ... */ }
    void handle(const CoinInserted&) override;
    void handle(const ArmPushed&) override { /* ... */ }
};

class Unlocked : public State {
public:
    void entry() override { /* ... */ }
    void exit() override { /* ... */ }
    void handle(const CoinInserted&) override { /* ... */ }
    void handle(const ArmPushed&) override {
        // Request transition to another state. The transition will be executed
        // by the state machine when this function returns.
        request_transition(make_state_ptr<Locked>());
    }
};

void Locked::handle(const CoinInserted&) {
    request_transition(make_state_ptr<Unlocked>());
}

int main() {
    // Create the state machine with an initial state.
    auto sm = StateMachine{make_state_ptr<Locked>()};
    // Dispatch the events.
    sm.dispatch(CoinInserted{});
    sm.dispatch(ArmPushed{});
}
```

Building
--------

Being header-only, the library does not need to be built.

Though, we provide a [CMake](https://cmake.org/) configuration to:
* Build the turnstile example;
* Build the documentation with [Doxygen](https://www.doxygen.nl/) and [Graphviz](https://graphviz.org/);
* Install the library and the aforementioned documentation.

Moreover, the installation procedure deploys a CMake file that lets you import the library from another CMake project by simply adding the following lines.

```
find_package(unstately REQUIRED)
add_executable(my-app main.cpp)
target_link_libraries(my-app PRIVATE unstately::unstately)
```

Versioning
----------

The library version numbering follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Coding conventions
------------------

The project uses [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) as code formatter and [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) for Git messages.

License
-------

Unstately is distributed under the terms of the [MIT license](LICENSE).
