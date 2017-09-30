#include <iostream>

#include "Logger.h"

using namespace std;

#ifndef DEBUG
static ostream NULLSTREAM(0);
#endif

#ifdef DEBUG
void _trace(const char *format, ...) {
    va_list(arglist);
    va_start(arglist, format);
    vfprintf(stdout, format, arglist);
}
#endif

Logger::Logger(const string& component)
    : stream(cout), component(component)
{

}

Logger::~Logger()
{

}

void Logger::info(const string& str) {
    cout << component << ": " << str << endl;
}

void Logger::error(const string& str) {
    cerr << component << ": " << str << endl;
}

void Logger::debug(const string& str) {
#ifdef DEBUG
    cout << PREFIX << component << ": " << str << endl;
#endif
}

ostream& Logger::operator<<(int i) {
    stream << component << ": " << i;
    return stream;
}

ostream& Logger::operator<<(const char* txt) {
    stream << component << ": " << txt;
    return stream;
}

ostream& Logger::out() {
    cout << component << ": ";
    return cout;
}

ostream& Logger::err() {
    cerr << component << ": ";
    return cerr;
}

ostream& Logger::dbg() {
#ifdef DEBUG
    cout << PREFIX << component << ": ";
    return cout;
#else
    return NULLSTREAM;
#endif

}
