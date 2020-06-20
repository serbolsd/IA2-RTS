/*****************************************************************************/
/**
 * @file    geMacroUtil.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/16
 * @brief   Some utility macros defined for use on the libraries.
 *
 * Some utility macros defined for use on the libraries.
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Macro utilities
 */
/*****************************************************************************/
#if GE_COMPILER == GE_COMPILER_MSVC
# define MULTI_LINE_MACRO_BEGIN do {
# define MULTI_LINE_MACRO_END         \
    __pragma(warning(push))           \
    __pragma(warning(disable:4127))   \
    } while(0)                        \
    __pragma(warning(pop))
#else
# define MULTI_LINE_MACRO_BEGIN do {
# define MULTI_LINE_MACRO_END } while(0)
#endif

/*****************************************************************************/
/**
 * Parameter utilities.
 * There is a UNREFERENCED_PARAMETER in WinNT, but is not compliant
 */
/*****************************************************************************/
#ifndef GE_UNREFERENCED_PARAMETER
# define GE_UNREFERENCED_PARAMETER(P) (void)P
#endif
