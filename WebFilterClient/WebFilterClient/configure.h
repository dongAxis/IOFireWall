//
//  configure.h
//  WebFilterClient
//
//  Created by Axis on 8/5/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef WebFilterClient_configure_h
#define WebFilterClient_configure_h

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "LockGuard.h"

#define WORK_THEAD_NUM 10

#define SAFE_RELEASE(ptr) \
        do{ \
            if(ptr!=NULL){ \
                CFRelease(ptr); \
                ptr=NULL;   \
            }\
        }while(0);

template<typename T>
class ReadConfig
{
private:
    std::string& file_path;
    boost::property_tree::ptree pt;
    T lock;
public:
    inline ReadConfig(std::string& path):file_path(path)
    {
        LockGuard<T> guard(lock);
        read_xml(this->file_path.c_str(), pt);
    }

    template<typename L>
    inline L getValue(std::string property_value)
    {
        LockGuard<T> guard(lock);
        return pt.get<L>(property_value);
    }

    inline ~ReadConfig()
    {
    }
};

#endif
