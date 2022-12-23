//
// Created by dingjing on 12/23/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_LOG_H
#define GRACEFUL_TERMINATOR_GT_LOG_H
#include <glib.h>

#define DEBUG(...) \
    g_log_structured(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,       \
                    "FILE", __FILE__,                       \
                    "LINE", __LINE__,                       \
                    "FUNC", __FUNCTION__,                   \
                    "MESSAGE", __VA_ARGS__);


#define INFO(...) \
    g_log_structured(G_LOG_DOMAIN, G_LOG_LEVEL_INFO,        \
                    "FILE", __FILE__,                       \
                    "LINE", __LINE__,                       \
                    "FUNC", __FUNCTION__,                   \
                    "MESSAGE", __VA_ARGS__);

#define WARNING(...) \
    g_log_structured(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,     \
                    "FILE", __FILE__,                       \
                    "LINE", __LINE__,                       \
                    "FUNC", __FUNCTION__,                   \
                    "MESSAGE", __VA_ARGS__);

#define ERROR(...) \
    g_log_structured(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,       \
                    "FILE", __FILE__,                       \
                    "LINE", __LINE__,                       \
                    "FUNC", __FUNCTION__,                   \
                    "MESSAGE", __VA_ARGS__);



#endif //GRACEFUL_TERMINATOR_GT_LOG_H
