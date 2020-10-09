#include "logging.h"



int main() {
    logger_t logger = {};
    logger_open(&logger, NULL);

    logger_log(&logger, "Test\n");
    logger_log(&logger, "Did you know that 1 < 2?\n");

    logger_close(&logger);

    return 0;
}
