#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdarg.h>

#include <string>
#include <iostream>

#ifdef DEBUG

#define PREFIX "[trace] "

void _trace(const char *format, ...);

#define traceIn \
    _trace("%s> %s: %s: %d", PREFIX, __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stdout, "\n")

#else

#define traceIn

#endif

class Logger
{

    std::ostream& stream;
    std::string component;

public:

    Logger(const std::string& component);
    virtual ~Logger();

    void info(const std::string&);
    void debug(const std::string&);
    void error(const std::string&);

    std::ostream& out();
    std::ostream& err();
    std::ostream& dbg();

    std::ostream& operator<<(int i);
    std::ostream& operator<<(const char*);

};

#endif /* LOGGER_H_ */
