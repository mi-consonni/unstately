/*
 * Unstately - A very simple state machine implementation in C++
 *
 * Copyright (c) 2023 Michele Consonni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UNSTATELY_UNSTATELY_H_
#define UNSTATELY_UNSTATELY_H_

#include <cassert>
#include <memory>
#include <utility>

//! Library major version.
#define UNSTATELY_VERSION_MAJOR 0
//! Library minor version.
#define UNSTATELY_VERSION_MINOR 3
//! Library patch version.
#define UNSTATELY_VERSION_PATCH 0

/**
 * @brief Library root namespace.
 */
namespace unstately {

/**
 * @brief The inteface that each application-defined state shall implement to handle a
 *        specific event type.
 * @tparam C Type of the state machine context.
 * @tparam E The event type to handle.
 */
template <typename C, typename E>
class EventHandlerUnit {
public:
    virtual ~EventHandlerUnit() = default;

    /**
     * @brief Handles an incoming event, possibly requesting a state change via
     *        State::request_transition.
     * @param c State machine context.
     * @param e Event to handle.
     */
    virtual void handle(C& c, const E& e) = 0;
};

/**
 * @brief An intermediate class that allows to inherit multiple instances of EventHandlerUnit.
 *        The application shall not use this class directly but only through State inheritance.
 * @tparam C Type of the state machine context.
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename C, typename... EE>
class EventHandler : public EventHandlerUnit<C, EE>... {
public:
    virtual ~EventHandler() = default;
};

/**
 * @brief The base class for all the states.
 *        All application-defined states shall inherit from this class.
 * @tparam P Pointer type to be used to hold the state object.
 * @tparam C Type of the state machine context.
 * @tparam EE Type list of the events that the state will handle.
 */
template <template <typename> typename P, typename C, typename... EE>
class State : public EventHandler<C, EE...> {
public:
    /**
     * @brief Pointer type used to return the next requested state.
     */
    using Ptr = P<State<P, C, EE...>>;

    /**
     * @brief Type of the state machine context.
     */
    using Context = C;

    explicit State() = default;

    virtual ~State() = default;

    State(const State& rhs) = delete;

    State(State&& rhs) = default;

    State& operator=(const State& rhs) = delete;

    State& operator=(State&& rhs) = default;

    /**
     * @brief Entry action to be implemented by application-defined states.
     * @param c State machine context.
     */
    virtual void entry(C& c) = 0;

    /**
     * @brief Exit action to be implemented by application-defined states.
     * @param c State machine context.
     */
    virtual void exit(C& c) = 0;

    /**
     * @brief Reacts to an incoming event.
     *        This method shall usually not be called directly but through a StateMachine.
     * @tparam E Type of the event to react to.
     * @param c State machine context.
     * @param e Event to react to.
     * @return Ptr Pointer the next state. May be empty if no transition is required.
     */
    template <typename E>
    Ptr react(C& c, const E& e) {
        EventHandlerUnit<C, E>& handler = *this;
        handler.handle(c, e);
        auto next_state = std::move(next_state_);
        next_state_ = Ptr{};
        return next_state;
    }

protected:
    /**
     * @brief Sets the next state to be returned by the State::react method.
     *        Application-defined states shall call this method inside their
     *        EventHandlerUnit::handle implementations to trigger a state change.
     * @param next_state Pointer to the next requested state.
     */
    void request_transition(Ptr&& next_state) {
        next_state_ = std::move(next_state);
    }

private:
    Ptr next_state_{};
};

/**
 * @brief The class representing the state machine.
 *        It holds the current state and delegates to it the event handling.
 * @tparam S Base class of the states that this state machine can handle.
 */
template <typename S>
class StateMachine {
public:
    /**
     * @brief Base class of the states.
     */
    using State = S;

    /**
     * @brief Pointer type used to store the current state.
     */
    using StatePtr = typename State::Ptr;

    /**
     * @brief Type of the state machine context.
     */
    using Context = typename State::Context;

    /**
     * @brief Constructs a new state machine object initialized with the input initial state.
     * @param context       State machine context.
     * @param initial_state Initial state to start from. It shall be non null, otherwise the
     *                      behaviour is undefined.
     */
    explicit StateMachine(Context&& context, StatePtr&& initial_state)
        : context_{std::move(context)}, state_{std::move(initial_state)} {
        assert(state_);
        state_->entry(context_);
    }

    ~StateMachine() {
        // Non-null check is needed because the object may have been moved
        if (state_) {
            state_->exit(context_);
        }
    }

    StateMachine(const StateMachine& rhs) = delete;

    StateMachine(StateMachine&& rhs) = default;

    StateMachine& operator=(const StateMachine& rhs) = delete;

    StateMachine& operator=(StateMachine&& rhs) = default;

    /**
     * @brief Dispatches the incoming event, _i.e._, lets the current state
     *        react to the event.
     * @tparam E Type of the event to dispatch.
     * @param e  Event to dispatch.
     */
    template <typename E>
    void dispatch(const E& e) {
        if (auto next_state = state_->react(context_, e)) {
            state_->exit(context_);
            state_ = std::move(next_state);
            state_->entry(context_);
        }
    }

private:
    Context context_{};
    StatePtr state_{};
};

/**
 * @brief Collection of utilities to manage statically created states.
 */
namespace static_ptr {

/**
 * @brief Pointer able to store statically created states.
 *
 * @tparam T Type of the pointee object, usually the abstract State class.
 */
template <typename T>
using Ptr = T*;

/**
 * @brief Helper function to create a new unstately::static_ptr::Ptr.
 * @tparam T Concrete type of the state to create.
 * @tparam Args Type list of the arguments to forward to T constructor.
 * @param args Arguments to forward to T constructor.
 * @return Ptr<T> Pointer to the newly created state.
 */
template <typename T, typename... Args>
Ptr<T> make_state_ptr(Args&&... args) {
    static bool first_time = true;
    static T instance{std::forward<Args>(args)...};
    if (first_time) {
        first_time = false;
    } else {
        instance = T{std::forward<Args>(args)...};
    }
    return &instance;
}

} // namespace static_ptr

/**
 * @brief Shortcut for a State type using unstately::static_ptr::Ptr.
 * @tparam C Type of the state machine context.
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename C, typename... EE>
using StaticState = State<static_ptr::Ptr, C, EE...>;

/**
 * @brief Collection of utilities to manage dynamically created states.
 */
namespace unique_ptr {

/**
 * @brief Pointer able to store dynamically created states.
 * @tparam T Type of the pointee object, usually the abstract State class.
 */
template <typename T>
using Ptr = std::unique_ptr<T>;

/**
 * @brief Helper function to create a new unstately::unique_ptr::Ptr.
 * @tparam T Concrete type of the state to create.
 * @tparam Args Type list of the arguments to forward to T constructor.
 * @param args Arguments to forward to T constructor.
 * @return Ptr<T> Pointer to the newly created state.
 */
template <typename T, typename... Args>
Ptr<T> make_state_ptr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace unique_ptr

/**
 * @brief Shortcut for a State type using unstately::unique_ptr::Ptr.
 * @tparam C Type of the state machine context.
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename C, typename... EE>
using UniqueState = State<unique_ptr::Ptr, C, EE...>;

} // namespace unstately

#endif // UNSTATELY_UNSTATELY_H_
