//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_LOGGER_H
#define IOTA_SIMPLEWALLET_LOGGER_H
void init_logger();

enum LOG_LEVEL {
    INFO = 1,
    DEBUG = 2,
    ERR = 3,
    FATAL = 4,
};
void _log(enum LOG_LEVEL, const char* format, ...);
#define log_wallet_info(x, ...) _log(INFO, x,  __VA_ARGS__);
#define log_wallet_debug(x, ...) _log(DEBUG, x,  __VA_ARGS__);
#define log_wallet_error(x, ...) _log(ERR, x,  __VA_ARGS__);
#define log_wallet_fatal(x, ...) _log(FATAL, x,  __VA_ARGS__);

#endif //IOTA_SIMPLEWALLET_LOGGER_H
