#ifndef COMMON_H
#define COMMON_H

// This sets sockets correctly - DO NOT REMOVE
#pragma comment (lib, "Ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/attr.hpp>

namespace ll = boost::log::trivial;

// -----------------------------------------------------------------------------

#define PRINTING // take away if you don't want to print anything

#ifdef _DEBUG
    #define PROJ_DEBUG
#else
//    #define PROJ_DEBUG // Uncomment this to display debugging output
#endif

#define DEFAULT_BUFLEN 65536
#define MAX_PACKET_SIZE 10000000


// -----------------------------------------------------------------------------

namespace expr = boost::log::expressions;

inline void init_logger()
{
	boost::log::add_common_attributes();
	boost::log::add_file_log
    (
        boost::log::keywords::file_name = "universe_log_%N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,                                   
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0), 
        boost::log::keywords::format = "[%TimeStamp%]: %Message%",
        boost::log::keywords::auto_flush = true
    );

	boost::log::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%]: %Message%");

#ifdef _DEBUG
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#else
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif
}

static HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
#ifdef PRINTING
#define P_ERR(x) std::cerr << __FUNCTION__ << ": "; \
                SetConsoleTextAttribute(h_console, 12); \
                std::cerr << x; \
                SetConsoleTextAttribute(h_console, 7);
#define P_WARN(x) std::cerr << __FUNCTION__ << ": "; \
                SetConsoleTextAttribute(h_console, 14); \
                std::cerr << x; \
                SetConsoleTextAttribute(h_console, 7);
#define P_INFO(x) std::cerr << __FUNCTION__ << ": "; \
                SetConsoleTextAttribute(h_console, 15); \
                std::cerr << x; \
                SetConsoleTextAttribute(h_console, 7);
#define P_CLEAR() system("cls");

#ifdef PROJ_DEBUG
#define P_DEBUG(x) SetConsoleTextAttribute(h_console, 11); \
                 std::cerr << "DEBUG: " << __FUNCTION__ << ": " << x; \
                 SetConsoleTextAttribute(h_console, 7);
#else
#define P_DEBUG(x) do { } while(0)
#endif
#else
#define P_ERR(x) do { } while(0)
#define P_WARN(x) do { } while(0)
#define P_INFO(x) do { } while(0)
#define P_CLEAR(x) do { } while(0)
#define P_DEBUG(x) do { } while(0)
#endif // PRINTING

#endif // !COMMON_H_
