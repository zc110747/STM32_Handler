
#pragma once

#include "main.h"

#define LOGGER_MAX_BUFFER_SIZE      256
#define LOGGER_MESSAGE_BUFFER_SIZE  16384
#define LOGGER_TX_QUEUE_NUM  		64

typedef enum
{
	LOG_TRACE = 0,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
}LOG_LEVEL;

typedef struct 
{
    char *ptr;
    int length;
}LOG_MESSAGE;

typedef struct 
{
    /// \brief memory_start_pointer_
    /// - memory point the start to get.
    char *memory_start_pointer_;
    
    /// \brief memory_end_pointer_
    /// - memory point the end.
    char *memory_end_pointer_;

    /// \brief log_level_
    /// - log level defined.
    LOG_LEVEL log_level_;
    
    SemaphoreHandle_t mutex_;

    QueueHandle_t tx_queue_;

    QueueHandle_t rx_queue_;

    uint8_t interface_;
    
    BaseType_t logger_init_ok;
}LOGGER_INFO;

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t logger_init(void);
int print_log(LOG_LEVEL level, const char* fmt, ...);
    
#ifdef __cplusplus
}
#endif
 
#define PRINT_NO_OS(...)                   printf(__VA_ARGS__);
#define PRINT_LOG(level, fmt, ...)       print_log(level, fmt, ##__VA_ARGS__);
