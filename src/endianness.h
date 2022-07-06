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
 * @file endianness.h
 * @author Featuremine Corporation
 * @date 20 Mar 2021
 * @brief File common C declaration for endianness
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_ENDIANNESS_H__
#define __FM_YTP_ENDIANNESS_H__

#include <apr.h>

#if defined(DARWIN)
#include <libkern/OSByteOrder.h>
#include <machine/endian.h>

#define _htobe16(x) OSSwapHostToBigInt16(x)
#define _htole16(x) OSSwapHostToLittleInt16(x)
#define _be16toh(x) OSSwapBigToHostInt16(x)
#define _le16toh(x) OSSwapLittleToHostInt16(x)

#define _htobe32(x) OSSwapHostToBigInt32(x)
#define _htole32(x) OSSwapHostToLittleInt32(x)
#define _be32toh(x) OSSwapBigToHostInt32(x)
#define _le32toh(x) OSSwapLittleToHostInt32(x)

#define _htobe64(x) OSSwapHostToBigInt64(x)
#define _htole64(x) OSSwapHostToLittleInt64(x)
#define _be64toh(x) OSSwapBigToHostInt64(x)
#define _le64toh(x) OSSwapLittleToHostInt64(x)

#define _BYTE_ORDER BYTE_ORDER
#define _BIG_ENDIAN BIG_ENDIAN
#define _LITTLE_ENDIAN LITTLE_ENDIAN
#define _PDP_ENDIAN PDP_ENDIAN
#elif defined(__linux__)
#include <endian.h>

#define _htobe16(x) htobe16(x)
#define _htole16(x) htole16(x)
#define _be16toh(x) be16toh(x)
#define _le16toh(x) le16toh(x)

#define _htobe32(x) htobe32(x)
#define _htole32(x) htole32(x)
#define _be32toh(x) be32toh(x)
#define _le32toh(x) le32toh(x)

#define _htobe64(x) htobe64(x)
#define _htole64(x) htole64(x)
#define _be64toh(x) be64toh(x)
#define _le64toh(x) le64toh(x)

#define _BYTE_ORDER BYTE_ORDER
#define _BIG_ENDIAN BIG_ENDIAN
#define _LITTLE_ENDIAN LITTLE_ENDIAN
#define _PDP_ENDIAN PDP_ENDIAN
#elif defined(WIN32)
#include <winsock2.h>

#if BYTE_ORDER == LITTLE_ENDIAN

#define _htobe16(x) htons(x)
#define _htole16(x) (x)
#define _be16toh(x) ntohs(x)
#define _le16toh(x) (x)

#define _htobe32(x) htonl(x)
#define _htole32(x) (x)
#define _be32toh(x) ntohl(x)
#define _le32toh(x) (x)

#define _htobe64(x) htonll(x)
#define _htole64(x) (x)
#define _be64toh(x) ntohll(x)
#define _le64toh(x) (x)

#elif BYTE_ORDER == BIG_ENDIAN

#define _htobe16(x) (x)
#define _htole16(x) __builtin_bswap16(x)
#define _be16toh(x) (x)
#define _le16toh(x) __builtin_bswap16(x)

#define _htobe32(x) (x)
#define _htole32(x) __builtin_bswap32(x)
#define _be32toh(x) (x)
#define _le32toh(x) __builtin_bswap32(x)

#define _htobe64(x) (x)
#define _htole64(x) __builtin_bswap64(x)
#define _be64toh(x) (x)
#define _le64toh(x) __builtin_bswap64(x)

#else

#error byte order not supported

#endif

#define _BYTE_ORDER BYTE_ORDER
#define _BIG_ENDIAN BIG_ENDIAN
#define _LITTLE_ENDIAN LITTLE_ENDIAN
#define _PDP_ENDIAN PDP_ENDIAN
#else

#error platform not supported

#endif

#endif // __FM_YTP_ENDIANNESS_H__