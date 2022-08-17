/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file platform.h
 * @author Maxim Trokhimtchouk
 * @date 26 May 2019
 * @brief File common C declaration useful in includes
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef NO_DLL_DECORATOR
#define DLL_PUBLIC
#else
#ifdef BUILDING_DLL
#ifdef __GNUC__
#define DLL_PUBLIC __attribute__((dllexport))
#else
#define DLL_PUBLIC __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define DLL_PUBLIC __attribute__((dllimport))
#else
#define DLL_PUBLIC __declspec(dllimport)
#endif
#endif
#endif
#else
#if __GNUC__ >= 4
#define DLL_PUBLIC __attribute__((visibility("default")))
#else
#define DLL_PUBLIC
#endif
#endif

#define FMMODFUNC DLL_PUBLIC

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) ||            \
    defined(_M_AMD64)
#define FMC_AMD64
#endif

#if defined(__linux__)
#define FMC_SYS_LINUX
#define FMC_SYS_UNIX
#elif defined(__APPLE__) && defined(__MACH__)
#define FMC_SYS_UNIX
#define FMC_SYS_MACH
#elif defined(_WIN64) || defined(_WIN32)
#define FMC_SYS_WIN
#endif

#ifdef __aarch64__
#define FMC_SYS_ARM
#endif

#if !defined(FMC_SYS_ARM) && !defined(FMC_SYS_WIN)
#define FMC_LICENSE
#endif

// macro for python module initialisation
#if defined(FMC_SYS_UNIX)
#define FMPYMODPUB DLL_PUBLIC
#else
#define FMPYMODPUB
#endif

// macro for check _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
#if defined(FMC_SYS_LINUX)
#define FMC_GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY                                  \
  static_assert(_GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY == 0, "should be zero")
#else
#define FMC_GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
#endif
