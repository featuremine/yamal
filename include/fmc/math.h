/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @file math.h
 * @author Andres Rangel
 * @date 3 May 2021
 * @brief File contains C declaration of math API
 *
 * This file contains C declaration of math API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <float.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define fmc_sign(x) (((x) > 0) - ((x) < 0))

/**
 * @brief Round to nearest
 *
 * @param x
 * @return the value x rounded to the nearest integral
 */
FMMODFUNC long long fmc_llround(double x);

/**
 * @brief Returns the difference between x and x rounded to the nearest integral
 *
 * @param x
 * @return x - (double)fmc_llround(x)
 */
FMMODFUNC double fmc_remainder(double x);

/**
 * @brief Returns true if -epsilon < x < epsilon
 *
 * @param x
 * @return true if -epsilon < x < epsilon
 */
#define FMC_ALMOST_ZERO(x) !((x) > DBL_EPSILON || (x) < -DBL_EPSILON)

/**
 * @brief Returns true if -epsilon < x - y < epsilon
 *
 * @param x
 * @param y
 * @return true if -epsilon < x - y < epsilon
 */
#define FMC_ALMOST_EQUAL(x, y) FMC_ALMOST_ZERO((x) - (y))

/**
 * @brief Returns true if x - y < epsilon
 *
 * @param x
 * @param y
 * @return true if  x - y < epsilon
 */
#define FMC_ALMOST_LESS(x, y) ((x) - (y) < DBL_EPSILON)

#ifdef __cplusplus
}
#endif
