#ifndef CORE_HANDLER_H
#define CORE_HANDLER_H

#include "CommandProcessor.h"

class CoreHandler {
public:
    // Core 0 main loop function
    static void core0Main();
    
    // Core 1 entry function
    static void core1Entry();
    
private:
    // Private constructor - this is a static utility class
    CoreHandler() {}
};

#endif // CORE_HANDLER_H