//------------------------------------------------------------------------------
/// @file       basic_executor.hpp
/// @author     João André
///
/// @brief      Header file providing declaration & definition of std::basic_executor and std::basic_timed_excutor classes for custom multi-thread operation abstraction.
///
/// std::basic_executor and std::basic_timed_executor as abstract classes that provide a simple way to streamline development of threaded applications
/// while abstracting over <thread>/<mutex>/<atomic> implementations, for single and looped/timed use cases.
///
/// Derived types are required to override execute() base pure virtual, and both classes provide run() and launch()/wait() methods for synchronous and asynchronous workflow respectively.
///
/// std::basic_executor is intended for looped/sporadic operations, and std::basic_timed_executor extends its functionality by employing a timer that runs asynchronous to the main operation,
/// and executing operation repeatedly until timer expires. boost::asio library provides timer implementation.
///
/// Additionally, std::wait_for() and std::chain() are provided as free functions to manage execution lifespan of different executor types.
///
//------------------------------------------------------------------------------

#ifndef TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_BASICEXECUTOR_HPP_
#define TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_BASICEXECUTOR_HPP_

#include <chrono>
#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>
#include <future>
#include <utility>
#include <boost/asio.hpp>
#include <tuple>

#define LEGACY_ASYNC_IMPLEMENTATION

#ifndef LEGACY_ASYNC_IMPLEMENTATION
#endif  // LEGACY_ASYNC_IMPLEMENTATION


namespace std {

//------------------------------------------------------------------------------
/// @brief      Static type check over variadic parameter pack, testing if there is any value (!= reference) type in pack.
///
/// @tparam     Args  Variadic template parameter pack to check.
///
template < typename... Args >
struct contains_value_type : std::false_type { /* ... */ };

template < typename T, typename... Args >
struct contains_value_type< T, Args... > :
    std::conditional<
        !std::is_reference< T >() && !std::is_void< T >(),  // condition to test
        std::true_type,                                     // if condition is true -> if T is a value-type
        contains_value_type< Args... >                      // if contition is false -> recusively call w/ remaining arguments in pack
    >::type {
    /* ... */
};


//------------------------------------------------------------------------------
/// @brief      Simple extensible base class for looped/unlooped multi-threaded operations.
///             Provides a consistent interface to develop synchronous and asynchronous operations with only superficial familiarity with multi-threaded programming.
///
/// @note       Execution is looped as:
///             \code
///             start();
///             do {
///                 execute(args...);
///             } while (active());
///             stop();
///             \endcode
///             Therefore, derived implementations *must* override execute() method, and optionally *can* start(), stop()) and active().
///             Default base implementations are provided for start(), stop() and active() so that execute() is called only once i.e. single-time operation.
///
template < typename... Args >
class basic_executor {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Static assertion on data types/template arguments. At this stage only references (lvalue or rvalue) are supported as template arguments.
    ///
    // static_assert(!contains_value_type< Args... >(),
    //               "RUNTIME ARGUMENT TYPES MUST BE REFERENCE-ONLY!");

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param[in]  loop  Boolean flag for continuous operations.
    ///                   Defaults to false, wherein operation is killed after first run.
    ///
    explicit basic_executor(bool loop = false);

    //--------------------------------------------------------------------------
    /// @brief      Destroys the object.
    ///
    virtual ~basic_executor();

    //--------------------------------------------------------------------------
    /// @brief      Run operation synchronously. Calls specialized/derived implementation of execute().
    ///
    virtual void run(Args... args);

    //--------------------------------------------------------------------------
    /// @brief      Launches an operation asynchronously (in the background), not blocking/waiting for its completion.
    ///
    /// @note       Operation keeps executing independent of main thread, even if it exits/finishes @ different times.
    ///
    virtual void launch(Args... args);

    //--------------------------------------------------------------------------
    /// @brief      Waits for execution to finish.
    ///
    /// @note       Returns immediately if thread was already terminated.
    ///
    virtual void wait() const;

    //--------------------------------------------------------------------------
    /// @brief      Forcibly stop execution @ next execution loop.
    ///
    /// @note       If execute() blocks thread, executor will not be killed.
    ///
    virtual void kill();

    //--------------------------------------------------------------------------
    /// @brief      Check if execution condition is verified.
    ///
    /// @return     True if active, false otherwise.
    ///
    /// @note       Pure virtual, derived types must provide their own implementation.
    ///
    virtual bool active() const;

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Un-looped function called before execute() loop.
    ///
    virtual void start();

    //--------------------------------------------------------------------------
    /// @brief      Un-looped function called after execute() loop.
    ///
    virtual void stop();

    //--------------------------------------------------------------------------
    /// @brief      Executes operation. Default implementation calls execute() in a loop while active() returns true i.e.
    ///             \code
    ///             start()
    ///             do {
    ///                 execute(args...);
    ///             } while (active());
    ///             stop()
    ///             \endcode
    ///
    /// @note       Pure virtual, derived types must provide their own implementation.
    ///
    virtual void execute(Args... args) = 0;

    //--------------------------------------------------------------------------
    /// @brief      Future object holding execution status.
    ///
    future< void > _done;

    //--------------------------------------------------------------------------
    /// @brief      Kill flag, for shutting down executor from separate threads (e.g. when running asynchronously with launch()).
    ///
    atomic< bool > _kill;
};



//--------------------------------------------------------------------------
/// @cond
template < typename... Args >
inline basic_executor< Args... >::basic_executor(bool loop) : _kill(!loop) /* single loop unless derived types override active() */ {
    /* ... */
}



template < typename... Args >
inline basic_executor< Args... >::~basic_executor() {
    /* ... */
}



template < typename... Args >
inline void basic_executor< Args... >::run(Args... args) {
    start();
    do {
        execute(std::forward< Args >(args)...);
    } while (active());
    stop();
}



template < typename... Args >
inline void basic_executor< Args... >::launch(Args... args) {
    #ifdef LEGACY_ASYNC_IMPLEMENTATION
    // C++14-compatible implementation
    // only for passed-by-reference arguments - doesn't work with passed-by-value arguments
    // as capturing by reference would lead to dangling references as pased-by-value arguments are local
    // additionally, capturing by value is not efficient (reference arguments would be copied into lambda),
    _done = std::async(std::launch::async, [this, &args... ] {
        this->run(std::forward<Args>(args)...);
    });
    #else
    // C++17 onward implementation (needs further testing, low priority)
    // works with passed-by-value and passed-by-reference arguments
    // copy arguments into a tuple, capture the tuple by value, use std::apply() to call function from lambda
    // cf. https://stackoverflow.com/questions/70645885/how-to-properly-generically-forward-a-parameter-pack-into-a-lambda/70646403?noredirect=1#comment124887266_70646403
    // @note: may require signatures to be changed to '(Args...)' and '#include <tuple>'
    std::tuple< basic_executor*, Args... > t { this, args... };
    _done = std::async(std::launch::async, [ t  ] {
        std::apply(&basic_executor::run, t);  // @note: C++17 onwards!
    });
    #endif
}



template < typename... Args >
inline void basic_executor< Args... >::wait() const {
    if (_done.valid()) {
        _done.wait();
    }
}



template < typename... Args >
inline bool basic_executor< Args... >::active() const {
    /* ... */
    // base implementation disables loop i.e. execute() is run only once
    return (!_kill);
}



template < typename... Args >
inline void basic_executor< Args... >::start() {
    /* ... */
}



template < typename... Args >
inline void basic_executor< Args... >::stop() {
    /* ... */
}



template < typename... Args >
inline void basic_executor< Args... >::kill() {
    /* ... */
    _kill = true;
    // wait();  // dangerous to wait for loop to exit. if e.g. kill() is called within execute(), it gets stuck in a loop!
}

/// @endcond


//------------------------------------------------------------------------------
/// @brief      Simple extensible base class for timed multi-threaded operations.
///             Extends std::basic_executor by implementing an operation deadline through the use of a timer.
///             Provides a consistent interface to develop synchronous and asynchronous operations with superficial familiarity with multi-threaded programming.
///
/// @note       The purpose is to provide a base type for different similar operations that allows easy management.
///
template < typename... Args >
class basic_timed_executor : public basic_executor< const chrono::milliseconds&, Args... > {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Timer type. Using boost::asio timers allows for greater felxiblity than std::chrono clocks (a timer class would need to be implemeted)
    ///
    using timer = boost::asio::steady_timer;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    basic_timed_executor();

    //--------------------------------------------------------------------------
    /// @brief      Waits for execution to finish.
    ///
    /// @note       Returns immediatly if thread was already terminated.
    ///
    void wait() const override;

    //--------------------------------------------------------------------------
    /// @brief      Check if executor is currently active (timer has elapsed).
    ///
    /// @return     True if active, false otherwise.
    ///
    bool active() const override;

    //--------------------------------------------------------------------------
    /// @brief      Access remaining time in execution.
    ///
    /// @return     Time remaining (in us).
    ///
    template < typename DurationType = chrono::milliseconds >
    DurationType remaining() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Run operation. Calls specialized/derived implementation of execute().
    ///
    void run(const chrono::milliseconds& time, Args... args) override;

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Executes operation. Default implementation calls execute() in a loop while active() returns true i.e.
    ///             \code
    ///             start()
    ///             do {
    ///                 execute(args...);
    ///             } while (active());
    ///             stop()
    ///             \endcode
    ///
    /// @note       Pure virtual, derived types must provide their own implementation.
    ///
    virtual void execute(Args... args) = 0;

    //--------------------------------------------------------------------------
    /// @brief      Explicit declaration of start() in order to remove from public interface.
    ///
    using basic_executor< const chrono::milliseconds&, Args... >::start;

    //--------------------------------------------------------------------------
    /// @brief      Explicit declaration of stop() in order to remove from public interface.
    ///
    using basic_executor< const chrono::milliseconds&, Args... >::stop;

    //--------------------------------------------------------------------------
    /// @brief      Resets timer to *time* & starts counting.
    ///
    void reset(const chrono::milliseconds& time);

    //--------------------------------------------------------------------------
    /// @brief      Future instance holding result of timer call.
    ///
    future< void > _elapsed;

    //--------------------------------------------------------------------------
    /// @brief      IO handler that runs asynchronous timer.
    ///
    boost::asio::io_context _io;

    //--------------------------------------------------------------------------
    /// @brief      Timer object.
    ///
    timer _timer;

 private:
    //--------------------------------------------------------------------------
    /// @brief      Dummy/placeholder overrride to disable inherited pure virtual execute(const milliseconds&, Args...).
    ///             Under the assumption that execute() does not need access to the run/launch duration, it is replaced by execute(Args...)
    ///
    /// @note       Declared private and final, i.e. derived executors will be unware of it.
    ///
    void execute(const chrono::milliseconds& time, Args... args) final;
};



//--------------------------------------------------------------------------
/// @cond

template < typename... Args >
inline basic_timed_executor< Args... >::basic_timed_executor() :
    basic_executor< const chrono::milliseconds&, Args... >(true),
    _timer(_io) {
        /* ... */
}



template < typename... Args >
inline void basic_timed_executor< Args... >::run(const chrono::milliseconds& time, Args... args) {
    reset(time);
    start();
    do {
        execute(std::forward<Args>(args)...);
    } while (active());
    stop();
}



template < typename... Args >
template < typename DurationType >
inline DurationType basic_timed_executor< Args... >::remaining() const noexcept {
    return std::chrono::duration_cast< DurationType >(_timer.expires_from_now());
}



template < typename... Args >
inline bool basic_timed_executor< Args... >::active() const {
    // test if std::future has valid state == timer async function has elapsed!
    return (_elapsed.wait_for(std::chrono::nanoseconds(0)) != std::future_status::ready);
}



template < typename... Args >
inline void basic_timed_executor< Args... >::wait() const {
    // wait for timer to expire if currently running
    if (_elapsed.valid()) {
        _elapsed.wait();
    }
    // call base class method to wait for executer finish
    basic_executor< const chrono::milliseconds&, Args... >::wait();
}



template < typename... Args >
inline void basic_timed_executor< Args... >::reset(const chrono::milliseconds& time) {
    // reset timer and call async_wait
    _timer.expires_after(std::chrono::duration_cast< timer::duration >(time));
    _timer.async_wait([this](const boost::system::error_code&) { /* no need to do anything */ });
    // wait for timer to expire if currently running
    if (_elapsed.valid()) {
        _elapsed.wait();
    }
    // start io_run asynchronously
    _elapsed = std::async(std::launch::async, [this] { _io.run(); });
}



template < typename... Args >
inline void basic_timed_executor< Args... >::execute(const chrono::milliseconds& time, Args... args) {
    /* ... */
}

/// @endcond


//------------------------------------------------------------------------------
/// @brief      Waits for the completion of a single given *executor*.
///
/// @param[in]  executor  Executor to wait for.
///
template < typename Exec >
inline void wait_for(Exec& executor) {
    executor.wait();
}



//------------------------------------------------------------------------------
/// @brief      Waits for the completion of given *executors*.
///
/// @param[in]  executor  Executor to wait for.
/// @param      others    Optional list of additional executors to wait for.
///
/// @note       Provided for conveninece when handling multiple executors.
///
/// @note       Could be solved with static if call (if constexpr () {}) but that would require C++17!
///
template < typename Exec, typename... Execs >
inline void wait_for(Exec& executor, Execs&... others) {
    executor.wait();  // wait must be defined!
    wait_for(others...);
}





//------------------------------------------------------------------------------
/// @brief      Chains execution of given *executors*.
///
/// @param      executors   Executor list.
/// @param[in]  sequential  Whether executores are launched sequentially.
///                         If true, waits for the completion of each before launching the next.
///                         If false, all executors are started in parallel.
///
template < typename... Args >
inline void chain(std::initializer_list< basic_executor< Args... >* >& executors, bool sequential = false) {
    for (const auto& executor : executors) {
        executor->launch();
        if (sequential == true) {
            executor->wait();
        }
    }
}

}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_BASICEXECUTOR_HPP_
