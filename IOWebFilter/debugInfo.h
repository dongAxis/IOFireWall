//
//  debugInfo.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef IOWebFilter_debugInfo_h
#define IOWebFilter_debugInfo_h

#include <IOKit/IOLib.h>
#include "common.h"

#define LOG_NONE	0
#define LOG_EMER	1
#define LOG_ALERT	2
#define LOG_CRIT	3
#define LOG_ERROR	4
#define LOG_WARN	5
#define LOG_NOTICE	6
#define LOG_INFO	7
#define LOG_DEBUG	8

static long current_level=LOG_DEBUG;

static char debug_text_info[][100] = {
    {"LOG_NONE"},
    {"LOG_EMER"},
    {"LOG_ALERT"},
    {"LOG_CRIT"},
    {"LOG_ERROR"},
    {"LOG_WARN"},
    {"LOG_NOTICE"},
    {"LOG_INFO"},
    {"LOG_DEBUG"}
};

#define LOG(debug_level ,args...)                                                                \
        do {                                                                                     \
            if(debug_level<=current_level){                                                      \
            IOLog("[IOWebFilter][%s] %s:", debug_text_info[debug_level],__FUNCTION__);          \
            IOLog(args);                                                                        \
            IOLog("\n");}                                                                       \
        }while(0)


#endif
