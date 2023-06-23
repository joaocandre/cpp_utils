#ifndef CPPUTILS_INCLUDE_EXTRA_CLI_HPP_
#define CPPUTILS_INCLUDE_EXTRA_CLI_HPP_

#include <memory>
#include <algorithm>
#include <mutex>
#include <vector>
#include <cassert>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <iomanip>
#include <string>
#include <mutex>
#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <utility>
#include <boost/asio.hpp>     // STDIN_FILENO
#include "extra/chrono.hpp"   // std::chrono::stopwatch
#include "extra/stream.hpp"   // std::to_string, std::format<>
#include "extra/math.hpp"     // math::ndigits

#ifndef CLI_PROGRESS_DEFAULT_LENGTH
#define CLI_PROGRESS_DEFAULT_LENGTH 50;
#endif  // CLI_PROGRESS_DEFAULT_LENGTH

//------------------------------------------------------------------------------

namespace std::style {

// usage: std::cout << std::style::[purple, ...] << "text" << std::color::end;

// @todo    define as a class/struct so that a common type can be attributed top each style e.g. passed as argument to formatting code

// colors
static const char* charcoal  = "\033[90m";  // GRAY
static const char* red       = "\033[91m";  // RED
static const char* green     = "\033[92m";  // GREEN
static const char* yellow    = "\033[93m";  // YELLOW
static const char* blue      = "\033[94m";  // BLUE
static const char* purple    = "\033[95m";  // PURPLE
static const char* cyan      = "\033[96m";  // CYAN
static const char* gray      = "\033[97m";  // GRAY
// ...

// colors (darker variants)
// cf. https://upload.wikimedia.org/wikipedia/commons/3/34/ANSI_sample_program_output.png
static const char* darkcharcoal = "\033[30m";  // GRAY
static const char* darkred      = "\033[31m";  // RED
static const char* darkgreen    = "\033[32m";  // GREEN
static const char* darkyellow   = "\033[33m";  // YELLOW
static const char* darkblue     = "\033[34m";  // BLUE
static const char* darkpurple   = "\033[35m";  // PURPLE
static const char* darkcyan     = "\033[36m";  // CYAN
static const char* darkgray     = "\033[97m";  // GRAY
// ...

// styles
static const char* bold      = "\033[1m";
static const char* underline = "\033[4m";
// ...

// clear formatting
static const char* none  = "";
static const char* clear = "\033[0m";

// @todo write function to add/compound multiple styles?


template < typename... Args >
std::string apply(const std::string& lstyle, const std::string& rstyle, Args&... others) {
    // recursive call to compound() w/ remaining arguments
    if constexpr (sizeof...(others) > 0) {
        return (lstyle + multi(rstyle, std::forward< Args >(others)...));
    }
    return (lstyle + rstyle);
}

}  // namespace std::style


//------------------------------------------------------------------------------

namespace cli {

//------------------------------------------------------------------------------
/// @brief      Gets the width (in characters) of current terminal window.
///
/// @return     Width of the terminal (in characters).
///
inline size_t get_terminal_width() {
#ifndef _WIN32
    #ifdef TIOCGSIZE
        struct ttysize ts;
        ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
        // int lines = ts.ts_lines;
        return ts.ts_cols;

    #elif defined(TIOCGWINSZ)
        struct winsize ts;
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
        // int lines = ts.ws_row;
        return ts.ws_col;
    #endif /* TIOCGSIZE */
#else
    // Windows implementation
    // cf. https://stackoverflow.com/a/23370070
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return (csbi.srWindow.Right - csbi.srWindow.Left + 1);
#endif
    return 0;
}


//------------------------------------------------------------------------------
/// @brief      Simple progress bar for CLI applications.
///
/// @todo       Format ETC text, to days/hours/mins/secs (duration_cast to chrono::date?)
///
/// @todo       Add derived/nested type to allow ranged operation e.g. for (auto& value : progress::over(container))
///
/// @note       Indented to provide high-level functionality similar to Python's *tqdm* module.
///
/// @note       Thread-safe implementation.
///             std::ostream::operator<<() used internally, and behavior is assumed to be thread-safe using standard output streams, particularly std::cout (default) or std::clog.
///             Single calls to operator<<() are already thread-safe (https://cplusplus.com/reference/ostream/ostream/operator%3C%3C/) and only a single dispatching call is used per update.
///
class progress {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Progress line configuration options.
    ///
    /// @todo       Add options to reorder indicators in progress line.
    ///
    struct options {
        //----------------------------------------------------------------------
        /// @brief      Pointer type alias, for verbosity.
        ///
        using ptr = std::shared_ptr< options >;

        //----------------------------------------------------------------------
        /// @brief      Output stream to print progress line.
        ///
        std::ostream& stream = std::cout;

        //----------------------------------------------------------------------
        /// @brief      Width of the progress line (as number of characters).
        ///
        /// @note       Defaults to terminal width.
        ///
        size_t width = get_terminal_width();

        //----------------------------------------------------------------------
        /// @brief      Ratio to width for the completion (%) bar.
        ///
        /// @note       If == 0.0, bar is extended to fit available space.
        ///
        float bar_ratio = 0.3;

        //----------------------------------------------------------------------
        /// @brief      Character to print as done portion of the completion (%) bar.
        ///
        char bar_done = '#';

        //----------------------------------------------------------------------
        /// @brief      Character to print as to-do portion of the completion (%) bar.
        ///
        char bar_todo = '.';

        //----------------------------------------------------------------------
        /// @brief      Delimiter character to place before/after .
        ///
        char delimiter = ' ';

        //----------------------------------------------------------------------
        /// @brief      Whether to use color styles on completion (green) or abortion (red) of progression.
        ///
        bool colorize = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to print progress bar.
        ///
        bool enable_bar = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to print percentage completed.
        ///
        bool enable_pc = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to print number of steps completed.
        ///
        bool enable_step = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to print total elapsed time (TET).
        ///
        /// @note       Disabled by default, recommended to only have one of TET / ETC time measures, otherwise progress line will be too crowded on small screens/windows.
        ///             Assumed that ETC is more valuble before completion, and TET afterwards, hence *replace_etc_with_tet_on_completion* option.
        ///
        bool enable_tet = false;

        //----------------------------------------------------------------------
        /// @brief      Whether to print estimated time to conclusion (ETC).
        ///
        bool enable_etc = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to limit label printing to estimated width.
        ///
        /// @note       Recommended to enforce single-line progress, otherwise large labels / abort messages may exceed available characters and force a new line.
        ///
        bool truncate_label = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to detail ETC/TET format w/ hours, minutes and seconds (more intuitive and practical), instead of just bulk second count.
        ///
        bool detailed_time = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to auto-append line break when reaching 100%.
        ///
        bool new_line_on_completion = true;

        //----------------------------------------------------------------------
        /// @brief      Whether to replace ETC value w/ TET when reching 100%.
        ///
        /// @note       Ignored if *enable_etc* is false.
        ///
        /// @note       Not recommended if both TET and ETC are enabled, as it will in duplicate text.
        ///             By default, only ETC is enabled, but TET info may be more valuable after completion.
        ///
        /// @todo       Find a shorter name, and add similar option for abort procedure.
        ///
        bool replace_etc_with_tet_on_completion = true;

        //----------------------------------------------------------------------
        /// @brief      Decimal precision of percentage values.
        ///
        /// @note       Ignored if *enable_pc* is false.
        ///             Can be overriden if custom formats are set.
        ///
        size_t pc_precision = 1;

        //----------------------------------------------------------------------
        /// @brief      Decimal precision of ETC value.
        ///
        /// @note       Ignored if *enable_etc* is false.
        ///             Can be overriden if custom formats are set.
        ///
        size_t time_precision = 2;

        //----------------------------------------------------------------------
        /// @brief      Progress line base style (cf. std::style namespace)
        ///
        /// @note       Extra styles passed to print() are compounded if *colorize* is true.
        ///
        const char* base_style = std::style::bold;

        //----------------------------------------------------------------------
        /// @brief      Text formatting to apply to percentage indicator.
        ///             Useful to bypass internal defaults.
        ///
        /// @note       Must account for the input of a single floating point value.
        ///             Direct editing of this value is not recommended, as wrongly-formed formats will result in unusable/invalid progress text.
        ///
        std::string pc_format = "";

        //----------------------------------------------------------------------
        /// @brief      Text formatting to apply to steps indicator.
        ///             Useful to bypass internal defaults.
        ///
        /// @note       Must account for the input of a two unsigned integer values.
        ///             Direct editing of this value is not recommended, as wrongly-formed formats will result in unusable/invalid progress text.
        ///
        std::string step_format = "";

        //----------------------------------------------------------------------
        /// @brief      Text formatting to apply to ETC indicator.
        ///             Useful to bypass internal defaults.
        ///
        /// @note       Must account for the input of a two unsigned integer values +  single floating point value if *detailed_time* is true, just a single floating point value otherwise.
        ///             Direct editing of this value is not recommended, as wrongly-formed formats will result in unusable/invalid progress text.
        ///
        std::string etc_format = "";

        //----------------------------------------------------------------------
        /// @brief      Text formatting to apply to TET indicator.
        ///             Useful to bypass internal defaults.
        ///
        /// @note       Must account for the input of a two unsigned integer values +  single floating point value if *detailed_time* is true, just a single floating point value otherwise.
        ///             Direct editing of this value is not recommended, as wrongly-formed formats will result in unusable/invalid progress text.
        ///
        std::string tet_format = "";

        //----------------------------------------------------------------------
        /// @brief      Static factory constructor
        ///
        /// @return     options::ptr instance with default configuration options.
        ///
        static ptr create() { return ptr(new options()); }

        //----------------------------------------------------------------------
        /// @brief      Generates/construct default indicator formats on given *config* (which requires *n_steps* @runtime).
        ///             *pc_format*, *steps_format* and *etc_format* are populated on call().
        ///
        /// @param      config       Configuration instance to generate formats for.
        /// @param[in]  n_steps      Total number of steps.
        /// @param[in]  overwrite    Whether to overwrite existing formats. Defautls to false.
        ///
        /// @note       When *overwrite* is false, only generates new/default formatting if format strings are empty.
        ///
        static void generate_formats(options::ptr& config, size_t n_steps, bool overwrite = false);
    };

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param[in]  label    Label to display on progress line.
    /// @param[in]  n_steps  Total number of steps (a.k.a. "ticks") to discretize progress.
    /// @param[in]  step     Step size for each update. Defaults to 1.
    /// @param[in]  config   Progress line configuration (cf. progress::options). Defaults to most sensible/common values.
    ///
    explicit progress(const std::string& label, size_t n_steps, size_t step = 1, const options::ptr& config = options::create());

    //--------------------------------------------------------------------------
    /// @brief      Destroys the object.
    ///
    virtual ~progress() = default;

    //--------------------------------------------------------------------------
    /// @brief      Checks if progress has reached 100% / conclusion.
    ///
    /// @return     True if progress is 100%, false otherwise.
    ///
    bool done() const;

    //--------------------------------------------------------------------------
    /// @brief      Updates the progress bar (printing to output stream).
    ///
    /// @param[in]  steps  Number of steps to increment, bypassing member step. Defaults to 0 i.e. default step is used.
    ///
    /// @return     True if progress is 100% *after update*, false otherwise.
    ///
    bool update(size_t steps = 0);

    //--------------------------------------------------------------------------
    /// @brief      Updates the progress bar w/ a new label (printing to output stream).
    ///             Useful when detailing each step with verbose descriptions.
    ///
    /// @param[in]  label  New label.
    /// @param[in]  steps  Number of steps to increment, bypassing member step. Defaults to 0 i.e. default step is used.
    ///
    /// @return     True if progress is 100% *after update*, false otherwise.
    ///
    /// @note       Convenience overload, equivalent to sequnrtial calls to progress::label(step_label) and progress::update(step);
    ///
    bool update(const std::string& label, size_t steps = 0);

    //--------------------------------------------------------------------------
    /// @brief      Abort execution before conclusion.
    ///
    /// @param[in]  message  Error/abort message (for verbosity);
    ///
    void abort(const std::string& message);

    //--------------------------------------------------------------------------
    /// @brief      Resets progress bar & applies configuration.
    ///
    /// @param[in]  index     New index. Defaults to 0.
    /// @param[in]  n_steps   New number of steps / length of execution. If = 0, not updated.
    /// @param[in]  step      New step size / length of increment/update. If = 0, not updated.
    ///
    /// @note       Default behavior '.reset()' just resets the counter.
    ///             Useful to reuse/repurpose an instantiated object for a new execution.
    ///
    void reset(size_t index = 0, size_t n_steps = 0, size_t step = 0);

    //--------------------------------------------------------------------------
    /// @brief      Edit/update progress bar label.
    ///
    /// @param[in]  label  New label to assign to progress bar. Will be printed on next call to update();
    ///
    /// @note       Useful to either reuse/repurpose an instantiated object, or to provide detailed information about each step/update.
    ///
    void label(const std::string& label);

    //--------------------------------------------------------------------------
    /// @brief      Provides an estimate of the time to conclusion (ETC) i.e. expected time to reach 100%, based on the elapsed time since start.
    ///
    /// @return     Time estimate, in seconds, of the ETC value.
    ///
    float etc() const;

    //--------------------------------------------------------------------------
    /// @brief      Construct progress bar indicator.
    ///
    /// @param[in]  completed_ratio   Ratio of completed
    /// @param[in]  width             Total width of the bar.
    /// @param[in]  done              Charater used on the completed portion of the progress bar. Defaults to '#'.
    /// @param[in]  todo              Charater used on the uncompleted portion of the progress bar. Defaults to '.'.
    /// @param[in]  align_right       Whether to alight the completed portion of the bar to the right. Defaults to false.
    ///
    /// @return     Text string with progress bar (enclosed by '[]').
    ///
    static std::string bar(float completed_ratio, size_t width, char done = '#', char todo = '.', bool align_right = false);

    //--------------------------------------------------------------------------
    /// @brief      Convert an duration value (in seconds) to a text string.
    ///             Optionally, can segment *sec* into day, hour, minute and second count, according to given *format*.
    ///
    /// @param[in]  etc          Duration value to convert (in seconds).
    /// @param[in]  format       Text format of returned string.
    /// @param[in]  detailed     Indicates if a detailed output is to be constructed. Defaults to false.
    /// @param[in]  null         Null character to place instead of numeric values when given *sec* is invalid i.e. < 0.0. Defaults to '-'.
    /// @param[in]  placeholder  Placeholder value to use before substituting w/ null charater when given *sec* is invalid i.e. < 0.0. Defaults to 0.
    ///                          Useful to parametrize as format may already include value (as character) and therefore substitution would mangle output.
    ///
    /// @return     Text string with duration value in given *format*.
    ///
    /// @todo       Make implementation generic and move out of class to std::chrono namespace. At this stage its scope is very specific to cli::progress class.
    ///
    static std::string sec2str(float sec, const std::string& format, bool detailed = false, char null = '-', size_t placeholder = 0);

    //--------------------------------------------------------------------------
    /// @brief      Prints progress bar to given output stream *ostream*.
    ///
    /// @param      ostream    The stream to write data to
    /// @param[in]  label      The label
    /// @param[in]  idx        The index
    /// @param[in]  steps      The steps
    /// @param[in]  etc        The etc
    /// @param[in]  bar_ratio  The bar ratio
    /// @param[in]  len        The length
    /// @param[in]  done       The done
    /// @param[in]  todo       The todo
    ///
    static void print(const std::string& label, size_t idx, size_t steps, float tet, float etc, const char* extra_style = std::style::none, const options::ptr& config = options::create());


    //--------------------------------------------------------------------------
    /// @brief      Configuration options.
    ///
    options::ptr config;

    // // // nested class for range loops
    // // // wraps around a STL container
    // // // @todo    extensively test
    // // // @todo    add SFINAE check to ensure T is a valid STL container (must proved begin() and end(), and have ::iterator typedef)
    // template < typename T >
    // class range : public progress {
    //  public:
    //     template< typename... Args, typename = typename std::enable_if< std::is_constructible< progress, Args... >::value >::type >
    //     explicit range(T& container, Args&& args...);

    //     ~range() = default;

    //     T::iterator begin();
    //     T::iterator end();

    //     T::const_iterator begin();
    //     T::const_iterator end();
    // };

    // template < typename T >
    // range progress over(T& container, size_t len = 50);

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Label to print along progress line.
    ///
    std::string _label;

    //--------------------------------------------------------------------------
    /// @brief      Current step.
    ///
    size_t _current;

    //--------------------------------------------------------------------------
    /// @brief      Total number of steps until conclusion.
    ///
    size_t _n_steps;

    //--------------------------------------------------------------------------
    /// @brief      Step increment between update().
    ///
    /// @note       Can be bypassed if different step is given to update().
    ///
    size_t _step;

    //--------------------------------------------------------------------------
    /// @brief      Stopwatch to measure time elapsed in-between updates.
    ///
    std::chrono::stopwatch _clock;

    //--------------------------------------------------------------------------
    /// @brief      Forward declaration of friend function (stream operator overload)
    ///
    friend std::ostream& operator<<(std::ostream& ostream, const progress& pg);
};


//------------------------------------------------------------------------------
/// @brief      Simple status printer for verbosity / debug
///
/// @param[in]  prefix  Descriptor string to place before status
/// @param[in]  idx     Current index/state
/// @param[in]  total   Total number of steps.
///
/// @note       Performance not an issue, overhead should be minimal.
///             Proper progressbars should be updated on separate threads.
///             cf. https://ezprogressbar.sourceforge.net/
///
/// @todo       Move to extra/io.hpp [cpp_utils]
///
/// @todo       Implement as a stream modifier
///
inline void status(const std::string& prefix, size_t idx, size_t total) {
    char status[50];
    double pc_done = static_cast< double >(100.0 * ((idx + 1.0) / total));
    snprintf(status, sizeof(status), "[%4lu/%4lu] %3.1f%%", idx + 1, total, pc_done);
    std::mutex m;
    std::scoped_lock lock(m);
    std::cout << '\r' << std::left/* << std::setw(10)*/ << prefix << std::right /*<< std::setw(20)*/ << status << (pc_done == 100 ? '\n' : ' ') << std::flush;
}


//------------------------------------------------------------------------------
/// @brief      Searches content of *argv* for given option *name* and returns its value.
///
/// @param[in]  argc          The count of arguments
/// @param[in]  argv          The arguments array
/// @param[in]  name          Option name.
/// @param[in]  vless         Enable/disable value-less option. Useful to parse flags e.g. '-enable_tool'. Defaults to false.
/// @param[in]  vless_return  Return value when value-less option is found. Null string "" is returned if not found. Defaults to "yes".
/// @param[in]  pre           Prefixing character before option name. Defaults to '-'. Useful to separate between different set of options/arguments.
/// @param[in]  sep           Separator character between option name and value. Defaults to '='.
///
/// @return     Content of the option, as the remaing content of argv entry after given *sep* charater e.g. '-option=value' returns 'value'.
///             Empty string if option is not found.
///
/// @note       Useful to simplify parsing CLI arguments, makes code more verbose at highest level.
///
/// @note       Option 'value' returned as text. Numeric values need to be cast/converted afterwards.
///
/// @todo       Turn into function template by return type. Use stringstream to convert to <T> and return. Potential issue: no 'null' value to return. Using empty string may actually be more flexible.
///             Using bool as return type would require saving both the option value/content and it's *found* status on the caller (?)
///
/// @todo       Rename as 'arg' or 'argument', more appropriate/fitting. + rename 'vless' arguments as 'flag'
///
/// @todo       Add bool* instead of 'vless_return' argument.
///
inline std::string option(int argc, char const *argv[], const std::string& name, bool vless = false, char pre = '-', char sep = '=', const std::string& vless_return = "yes") {
    assert(vless_return.empty() == false);
    // loop over arguments (first is ignored)
    bool parse = true;
    for (size_t i = 1; i < argc; i++) {
        // if parsing is disabled, return full argument
        if (!parse) {
            return std::string(argv[i]);
            parse = true;
        }
        // check if valid argument, ignore otherwise
        if (argv[i][0] != pre) {
            continue;
        }
        // create string from char array, and find separator
        std::string arg(argv[i]);
        if (sep == ' ') {
            parse = false;
            continue;
        }
        size_t pos = arg.find_first_of(sep, 1 /* discard first character */);
        // check position of sep character, if not found, skip argument
        if (pos == std::string::npos) {
            // if sep character is not found but vless argument has matching name, return given vless_return
            if (vless && arg.substr(1) == name) {
                return vless_return;
            }
            continue;
        }
        // compare option name, if doesn't match skip argument
        if (arg.substr(1, pos - 1) != name) {
            continue;
        }
        // return substring with option value (as text)
        return arg.substr(pos + 1);
    }

    return "";
}


//------------------------------------------------------------------------------
/// @brief      Searches content of *argv* for *name* flag and returns true if present, false otherwise.
///
/// @param[in]  argc  The count of arguments
/// @param[in]  argv  The arguments array
/// @param[in]  name  Flag name.
/// @param[in]  pre   Prefixing character before option name. Defaults to '-'. Useful to separate between different set of options/arguments.
///
/// @return     True if flag is present (e.g. -enable_tool), false otherwise.
///
inline bool flag(int argc, char const *argv[], const std::string& name, char pre = '-') {
    if (option(argc, argv, name, true, pre).empty()) {
        return false;
    }
    return true;
}


//------------------------------------------------------------------------------
/// @cond

void progress::options::generate_formats(options::ptr& config, size_t n_steps, bool overwrite) {
    if (overwrite || config->pc_format.empty()) {
        config->pc_format = "%." + std::to_string(config->pc_precision) + "f%%";
    }
    if (overwrite || config->step_format.empty()) {
        auto len = math::ndigits(n_steps);
        config->step_format = "(%0" + std::to_string(len) + "lu/%lu)";
    }
    if (overwrite || config->tet_format.empty()) {
        if (config->detailed_time) {
            config->tet_format = "TET: %02luh %02lum %0" + std::to_string(config->time_precision + 3 /* units + '.' */) + "." + std::to_string(config->time_precision) + "fs";
        } else {
            config->tet_format = "TET: %." + std::to_string(config->time_precision) + "fs";
        }
        // replace spaces with custom delimiter character if provided
        if (config->delimiter != ' ') {
            std::replace(config->tet_format.begin(), config->tet_format.end(), ' ', config->delimiter);
        }
    }
    if (overwrite || config->etc_format.empty()) {
        if (config->detailed_time) {
            config->etc_format = "ETC: %02luh %02lum %0" + std::to_string(config->time_precision + 3 /* units + '.' */) + "." + std::to_string(config->time_precision) + "fs";
        } else {
            config->etc_format = "ETC: %." + std::to_string(config->time_precision) + "fs";
        }
        // replace spaces with custom delimiter character if provided
        if (config->delimiter != ' ') {
            std::replace(config->etc_format.begin(), config->etc_format.end(), ' ', config->delimiter);
        }
    }
}


progress::progress(const std::string& label, size_t n_steps, size_t step, const progress::options::ptr& config) :
    config(config),
    _label(label),
    _n_steps(n_steps),
    _step(step),
    _current(0) {
        // ensure total number of steps is > 0
        // progress is useless otherwise
        assert(_n_steps > 0);

        // check if valid width parameter, revert to hardcoded defauults otherwise
        // @note    may happen if system/low-level calls to get terminal width fail
        if (!config->width) {
            // in case invalid width
            config->width = CLI_PROGRESS_DEFAULT_LENGTH;
        }

        // reset progress state & print empty progress line
        reset(_current);
}


bool progress::done() const {
    if (_current < _n_steps) {
        return false;
    }
    return true;
}


bool progress::update(size_t step) {
    if (!step) {
        // use member step count
        step = _step;
    }

    // increment tick counter
    _current += step;

    // print to output stream
    print(_label, _current, _n_steps, _clock.peek(), etc(), (config->colorize && done()) ? std::style::green : std::style::none, config);

    return done();
}


bool progress::update(const std::string& step_label, size_t step) {
    label(step_label);
    return update(step);
}


void progress::abort(const std::string& message) {
    print(_label + config->delimiter + "[" + message + "]", _current, _n_steps, _clock.peek(), etc(), config->colorize ? std::style::red : std::style::none, config);
    config->stream << '\n';
}


void progress::reset(size_t index, size_t n_steps, size_t step) {
    // update member parametrization
    // @note    total steps and step size are only updated if positive values are passed
    //          (progress line is useless if any of them are null).
    _current = index;
    if (n_steps) {
        _n_steps = n_steps;
    }
    if (step) {
        _step = step;
    }

    // construct formatting strings for each progress line component
    // @note    executed on reset() to avoid re-constructing the same format strings on each update, which can become costly when updating at high frequency.
    //          assumed format doesn't change until reset() is called.
    options::generate_formats(config, _n_steps);

    // print empty/reset progress line
    // @note    update() is not called as it would increment the counter.
    // @note    dummy ETC printing, can't estimate until first step is concluded
    print(_label, _current, _n_steps, 0.0, -1.0, std::style::none, config);

    // reset clock
    _clock.reset();
}


void progress::label(const std::string& label) {
    _label = label;
}


float progress::etc() const {
    // estimate time to conclusion
    // @note     simple rule of three (assumes linearity!)
    //           a more comprehensive estimate is not needed.
    float ratio = static_cast< float >(_current) / _n_steps;
    if (ratio < 1.0 && ratio > 0.0) {
        return (_clock.peek() * (1.0 - ratio) / ratio);
    }

    return 0.0;
}


std::string progress::bar(float completed_ratio, size_t width, char done, char todo, bool align_right) {
    // ensure ratio upperbound, otherwise bar will be larger than given *width*
    if (completed_ratio > 1.0) {
        completed_ratio = 1.0;
    }

    std::stringstream sstream;
    sstream << '[';
    sstream << (align_right ? std::right : std::left);
    sstream << std::setw(width) << std::setfill(todo);
    sstream << std::string(completed_ratio * width, done);
    sstream << ']';

    return sstream.str();
}


std::string progress::sec2str(float sec, const std::string& format, bool detailed, char null, size_t placeholder) {
    // auto-check if detailed or not (number of values)
    // @note    adds unnecessary overhead, although it may be negligible
    // @todo    make function dynamically segment input according to format (discard "detailed" flag)
    // size_t n = std::count(format.begin(), format.end(), '%');
    std::string text = "";
    if (detailed) {
        if (sec >= 0.0) {
            // convert total second count to hours + minutes + seconds
            float hours = std::chrono::stoh(sec);
            float mins  = std::chrono::stom(3600.0 * (hours - static_cast< size_t >(hours)));
            float secs  = 60.0 * (mins - static_cast< size_t >(mins));
            text += std::format< 20 >(format, static_cast< size_t >(hours), static_cast< size_t >(mins), secs);
        } else {
            // use null values and replace '0' with null charater
            // @note    only works if there are no additional '0' on format
            text += std::format< 20 >(format, placeholder, placeholder, placeholder);
            std::replace(text.begin(), text.end(), static_cast< char >(placeholder + 48), null);
        }
    } else {
        if (sec >= 0.0) {
            // print sec value directly
            text += std::format< 20 >(format, sec);
        } else {
            // use null values and replace '0' with null charater
            // @note    only works if there are no additional '0' on format
            text += std::format< 20 >(format, placeholder);
            std::replace(text.begin(), text.end(), static_cast< char >(placeholder + 48), null);
        }
    }

    return text;
}


void progress::print(const std::string& label, size_t current, size_t n_steps, float tet, float etc, const char* extra_style, const progress::options::ptr& config) {
    if (current > n_steps) {
        current = n_steps;
    }

    // compute ratio completed (percentage)
    float ratio = static_cast< float >(current) / n_steps;

    // construct XXX% (percentage concluded) indicator
    std::string pc_indicator = "";
    if (config->enable_pc) {
        pc_indicator += config->delimiter;
        pc_indicator += std::format(config->pc_format, ratio * 100.0);
    }

    // construct (steps/total) indicator
    // @note    format is enforced to ensure constant length for all updates -> keeps progress bar at same position throughout update
    std::string step_indicator = "";
    if (config->enable_step) {
        step_indicator += config->delimiter;
        step_indicator += std::format(config->step_format, current, n_steps);
    }

    // construct etc indicator
    std::string tet_indicator = "";
    if (config->enable_tet) {
        tet_indicator += config->delimiter;
        tet_indicator += progress::sec2str(tet, config->tet_format, config->detailed_time);
    }

    // construct etc indicator
    std::string etc_indicator = "";
    if (config->enable_etc) {
        etc_indicator += config->delimiter;
        if (ratio == 1.0 && config->replace_etc_with_tet_on_completion) {
            etc_indicator += progress::sec2str(tet, config->tet_format, config->detailed_time);
        } else {
            etc_indicator += progress::sec2str(etc, config->etc_format, config->detailed_time);
        }
    }

    // compute length of label portion and progress bar
    std::string bar = "";
    if (config->enable_bar) {
        size_t bar_width = 0;
        if (!config->bar_ratio) {
            bar_width = config->width - label.length() - step_indicator.length() - pc_indicator.length() - tet_indicator.length() - etc_indicator.length() - 2 /* enclosing brackets */ - 1 /* delimiter space before bar */;
        } else {
            bar_width = config->bar_ratio * config->width;
        }
        bar += config->delimiter;
        bar += progress::bar(ratio, bar_width, config->bar_done, config->bar_todo);
    }

    // initialize local scope string stream before printing to output stream
    // streamlines progress line construction before dispatching to output stream
    // facilitates thread safety as only a single serialization is done
    std::stringstream sstream;
    sstream << config->base_style;
    sstream << extra_style;

    // print label to output stream
    // @note    estimate label width from the total minus the size of the other (higher-priority) components
    size_t label_width = config->width - bar.length() - step_indicator.length() - pc_indicator.length() - tet_indicator.length() - etc_indicator.length();
    sstream << std::left << std::setw(label_width) << std::setfill(config->delimiter);
    // truncate label if larger than available space
    if (config->truncate_label && label.length() > label_width) {
        sstream << (label.substr(0, label_width - 4) + "..." + label.back());  // must be a single string, otherwise fill will be printed before '...'
    } else {
        sstream << label;
    }

    // write indicator components/elements
    // @note    if not enabled, each indicator will be empty and nothing will be printed
    sstream << pc_indicator;
    sstream << bar;
    sstream << step_indicator;
    sstream << tet_indicator;
    sstream << etc_indicator;

    // clear styles, flush (may be redundant) and break line if completed
    sstream << std::style::clear << '\r' << std::flush;
    sstream << (config->new_line_on_completion && ratio == 1.0 ? "\n" : "");

    // write to output stream (ensuring thread-safety)
    std::mutex mt;
    {
        std::scoped_lock lock(mt);
        config->stream << sstream.str() << std::flush;
    }
}


std::ostream& operator<<(std::ostream& ostream, const progress& pg) {
    progress::print(pg._label, pg._current, pg._n_steps, pg._clock.peek(), pg.etc(), (pg.config->colorize && pg.done()) ? std::style::green : std::style::none, pg.config);
    return ostream;
}

/// @endcond
//------------------------------------------------------------------------------

}  // namespace cli

#endif  // CPPUTILS_INCLUDE_EXTRA_CLI_HPP_
