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
#define UNSTATELY_VERSION_MINOR 1
//! Library patch version.
#define UNSTATELY_VERSION_PATCH 0

/**
 * @brief Library root namespace.
 */
namespace unstately {

/**
 * @brief The inteface that each application-defined state shall implement to handle a
 *        specific event type.
 *
 * @tparam E The event type to handle.
 */
template <typename E>
class EventHandlerUnit {
public:
    virtual ~EventHandlerUnit() = default;

    /**
     * @brief Handles an incoming event, possibly requesting a state change via
     *        State::request_transition.
     *
     * @param e Event to handle.
     */
    virtual void handle(const E& e) = 0;
};

/**
 * @brief An intermediate class that allows to inherit multiple instances of EventHandlerUnit.
 *        The application shall not use this class directly but only through State inheritance.
 *
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename... EE>
class EventHandler : public EventHandlerUnit<EE>... {
public:
    virtual ~EventHandler() = default;
};

/**
 * @brief The base class for all the states.
 *        All application-defined states shall inherit from this class.
 *
 * @tparam P  Pointer type to be used to hold the state object.
 * @tparam EE Type list of the events that the state will handle.
 */
template <template <typename> typename P, typename... EE>
class State : public EventHandler<EE...> {
public:
    /**
     * @brief Pointer type used to return the next requested state.
     */
    using Ptr = P<State<P, EE...>>;

    explicit State() : next_state_(Ptr{}) {}

    virtual ~State() = default;

    State(const State& rhs) = delete;

    State(State&& rhs) = default;

    State& operator=(const State& rhs) = delete;

    State& operator=(State&& rhs) = default;

    /**
     * @brief Entry action to be implemented by application-defined states.
     */
    virtual void entry() = 0;

    /**
     * @brief Exit action to be implemented by application-defined states.
     */
    virtual void exit() = 0;

    /**
     * @brief Reacts to an incoming event.
     *        This method shall usually not be called directly but through a StateMachine.
     *
     * @tparam E   Type of the event to react to.
     * @param e    Event to react to.
     * @return Ptr Pointer the next state. May be empty if no transition is required.
     */
    template <typename E>
    Ptr react(const E& e) {
        EventHandlerUnit<E>& handler = *this;
        handler.handle(e);
        auto next_state = std::move(next_state_);
        next_state_ = Ptr{};
        return next_state;
    }

protected:
    /**
     * @brief Sets the next state to be returned by the State::react method.
     *        Application-defined states shall call this method inside their
     *        EventHandlerUnit::handle implementations to trigger a state change.
     *
     * @param next_state Pointer to the next requested state.
     */
    void request_transition(Ptr&& next_state) {
        next_state_ = std::move(next_state);
    }

private:
    Ptr next_state_;
};

/**
 * @brief The class representing the state machine.
 *        It holds the current state and delegates to it the event handling.
 *
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
     * @brief Constructs a new state machine object initialized with the input initial state.
     *
     * @param initial_state Initial state to start from. It shall be non null, otherwise the
     *                      behaviour is undefined.
     */
    explicit StateMachine(StatePtr&& initial_state) : state_(std::move(initial_state)) {
        assert(state_);
        state_->entry();
    }

    ~StateMachine() {
        // Non-null check is needed because the object may have been moved
        if (state_) {
            state_->exit();
        }
    }

    StateMachine(const StateMachine& rhs) = delete;

    StateMachine(StateMachine&& rhs) = default;

    StateMachine& operator=(const StateMachine& rhs) = delete;

    StateMachine& operator=(StateMachine&& rhs) = default;

    /**
     * @brief Dispatches the incoming event, _i.e._, lets the current state
     *        react to the event.
     *
     * @tparam E Type of the event to dispatch.
     * @param e  Event to dispatch.
     */
    template <typename E>
    void dispatch(const E& e) {
        if (auto next_state = state_->react(e)) {
            state_->exit();
            state_ = std::move(next_state);
            state_->entry();
        }
    }

private:
    StatePtr state_;
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
 *
 * @warning When called more than once for any given type T, only the first
 *          call will actually create a new instance. All subsequent calls will return
 *          the previously created instance, thus ignoring any change in the input arguments.
 *
 * @tparam T      Concrete type of the state to create.
 * @tparam Args   Type list of the arguments to forward to T constructor.
 * @param args    Arguments to forward to T constructor.
 * @return Ptr<T> Pointer to the newly created state.
 */
template <typename T, typename... Args>
Ptr<T> make_state_ptr(Args&&... args) {
    static auto instance = T{std::forward<Args>(args)...};
    return &instance;
}

} // namespace static_ptr

/**
 * @brief Shortcut for a State type using unstately::static_ptr::Ptr.
 *
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename... EE>
using StaticState = State<static_ptr::Ptr, EE...>;

/**
 * @brief Collection of utilities to manage dynamically created states.
 */
namespace unique_ptr {

/**
 * @brief Pointer able to store dynamically created states.
 *
 * @tparam T Type of the pointee object, usually the abstract State class.
 */
template <typename T>
using Ptr = std::unique_ptr<T, std::default_delete<T>>;

/**
 * @brief Helper function to create a new unstately::unique_ptr::Ptr.
 *
 * @tparam T      Concrete type of the state to create.
 * @tparam Args   Type list of the arguments to forward to T constructor.
 * @param args    Arguments to forward to T constructor.
 * @return Ptr<T> Pointer to the newly created state.
 */
template <typename T, typename... Args>
Ptr<T> make_state_ptr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace unique_ptr

/**
 * @brief Shortcut for a State type using unstately::unique_ptr::Ptr.
 *
 * @tparam EE Type list of the events that the state will handle.
 */
template <typename... EE>
using UniqueState = State<unique_ptr::Ptr, EE...>;

} // namespace unstately

#endif // UNSTATELY_UNSTATELY_H_
