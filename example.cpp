#include <cassert>
#include <iostream>
#include <sstream>

bool _verbose = false;

bool verbose_logging_enabled() {
    return _verbose;
}

void enable_verbose_logging() {
    _verbose = true;
}

void disable_verbose_logging() {
    _verbose = false;
}

// Crazy variable names make collisions unlikely.
#define LOG_LOOP_VARIABLE _lOoP_oNcE_oNlY
#define LOG_STREAM _lOg_sTrEaM

// Use an intermediate `std::stringstream` to build the log message before
// inserting into `std::cerr`.  This is done to optimize the common case where
// `std::cerr` is unbuffered, and so we seek to insert into `std::cerr` only
// once. `std::stringstream` is used instead of `std::ostringstream` so that
// the stream's contents can be inserted into `std::cerr` directly from the
// stream buffer.
#define LOG_ERROR \
    for (bool LOG_LOOP_VARIABLE = true; LOG_LOOP_VARIABLE;) \
        for (std::stringstream LOG_STREAM; LOG_LOOP_VARIABLE; LOG_LOOP_VARIABLE = false, std::cerr << LOG_STREAM.rdbuf()) \
         LOG_STREAM << __FILE__ << ":" << __LINE__ << " "

// The cost of evaluating a `LOG_VERBOSE` statement when verbose logging is
// disabled:
//
// - Extra code in the function may inhibit inlining.
// - The branch might stall the instruction pipeline (though branch prediction
//   might mitigate this entirely).
// - `verbose_logging_enabled` might not be inlined, entailing a function call.
//
// Notably, though, the log message expression will not be evaluated at all.
#define LOG_VERBOSE \
    if (verbose_logging_enabled())  LOG_ERROR

int _counter = 0;
int next() {
    return ++_counter;
}

int main() {
    disable_verbose_logging();
    LOG_ERROR << "This will always print: " << next() << "\n";
    assert(_counter == 1);
    LOG_VERBOSE << "This will not print: " << next() << "\n";
    assert(_counter == 1);

    enable_verbose_logging();
    LOG_ERROR << "This too will always print: " << next() << "\n";
    assert(_counter == 2);
    LOG_VERBOSE << "This will print as well: " << next() << "\n";
    assert(_counter == 3);

    disable_verbose_logging();
    LOG_VERBOSE << "But not this: " << next() << "\n";
    assert(_counter == 3);
}
