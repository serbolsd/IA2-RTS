/*****************************************************************************/
/**
 * @file    gePrerequisitesUtil.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/09
 * @brief   Utility include with the most basic defines needed
 *
 * Utility include with the most basic defines needed for compilation
 * This is the file that must be included for use on external objects
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Most basic includes (required for everything else)
 */
/*****************************************************************************/
#include <cassert>

/*****************************************************************************/
/**
 * GE_THREAD_SUPPORT
 * 0 - No thread support
 * 1 - Render system is thread safe (TODO: NOT WORKING and will probably be removed)
 * 2 - Thread support but render system can only be accessed from main thread
 */
/*****************************************************************************/
#define GE_THREAD_SUPPORT		2

/*****************************************************************************/
/**
 * GE_PROFILING_ENABLED - Enabled/Disable Profiling
 */
/*****************************************************************************/
#define GE_PROFILING_ENABLED	1

/*****************************************************************************/
/**
 * Version tracking constants
 */
/*****************************************************************************/
#define GE_VER_DEV      1           //Development Version
#define GE_VER_PREVIEW  2           //Preview Version
#define GE_VER          GE_VER_DEV  //This is the one that will be checked

/*****************************************************************************/
/**
 * Platform specific stuff
 */
/*****************************************************************************/
#include "gePlatformDefines.h"

#if GE_COMPILER == GE_COMPILER_CLANG
  /** @ref scriptBindingMacro */
# define GE_SCRIPT_EXPORT(...) __attribute__((annotate("se," #__VA_ARGS__)))
#else
  /** @ref scriptBindingMacro */
# define GE_SCRIPT_EXPORT(...)
#endif

/*****************************************************************************/
/**
 * Include of short-hand names for various built-in types
 */
/*****************************************************************************/
#include "gePlatformTypes.h"        //Platform types
#include "geMacroUtil.h"            //Utility Macros
#include "geMemoryAllocator.h"      //Memory Allocator
#include "geStdHeaders.h"           //Commonly used standard headers

#include "geThreadDefines.h"        //Threading Defines
#include "geFwdDeclUtil.h"          //Forward declarations
#include "geRTTIPrerequisites.h"    //Run-time type information Prerequisites

#include "geString.h"               //String objects
#include "geStringID.h"             //String IDs for string fast comparison

#include "geMessageHandlerFwd.h"    //Forward declarations of the message objects
#include "geFlags.h"                //Flags template
#include "geUtil.h"                 //Hashing utilities
#include "gePath.h"                 //Paths to files
#include "geEvent.h"                //Event objects and handlers
#include "gePlatformUtility.h"      //Utilities for the specific operating system
#include "geNonCopyable.h"          //Interface for Non-copyable objects
#include "geCrashHandler.h"         //Crash handling functions
