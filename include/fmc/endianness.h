/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file endianness.h
 * @author Maxim Trokhimtchouk
 * @date 20 Mar 2021
 * @brief File common C declaration for endianness
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#if defined(FMC_SYS_MACH)
#include <libkern/OSByteOrder.h>
#include <machine/endian.h>

#define fmc_htobe16(x) OSSwapHostToBigInt16(x)
#define fmc_htole16(x) OSSwapHostToLittleInt16(x)
#define fmc_be16toh(x) OSSwapBigToHostInt16(x)
#define fmc_le16toh(x) OSSwapLittleToHostInt16(x)

#define fmc_htobe32(x) OSSwapHostToBigInt32(x)
#define fmc_htole32(x) OSSwapHostToLittleInt32(x)
#define fmc_be32toh(x) OSSwapBigToHostInt32(x)
#define fmc_le32toh(x) OSSwapLittleToHostInt32(x)

#define fmc_htobe64(x) OSSwapHostToBigInt64(x)
#define fmc_htole64(x) OSSwapHostToLittleInt64(x)
#define fmc_be64toh(x) OSSwapBigToHostInt64(x)
#define fmc_le64toh(x) OSSwapLittleToHostInt64(x)

#define FMC_BYTE_ORDER BYTE_ORDER
#define FMC_BIG_ENDIAN BIG_ENDIAN
#define FMC_LITTLE_ENDIAN LITTLE_ENDIAN
#define FMC_PDP_ENDIAN PDP_ENDIAN
#elif defined(FMC_SYS_LINUX)
#include <endian.h>

#define fmc_htobe16(x) htobe16(x)
#define fmc_htole16(x) htole16(x)
#define fmc_be16toh(x) be16toh(x)
#define fmc_le16toh(x) le16toh(x)

#define fmc_htobe32(x) htobe32(x)
#define fmc_htole32(x) htole32(x)
#define fmc_be32toh(x) be32toh(x)
#define fmc_le32toh(x) le32toh(x)

#define fmc_htobe64(x) htobe64(x)
#define fmc_htole64(x) htole64(x)
#define fmc_be64toh(x) be64toh(x)
#define fmc_le64toh(x) le64toh(x)

#define FMC_BYTE_ORDER BYTE_ORDER
#define FMC_BIG_ENDIAN BIG_ENDIAN
#define FMC_LITTLE_ENDIAN LITTLE_ENDIAN
#define FMC_PDP_ENDIAN PDP_ENDIAN
#elif defined(FMC_SYS_WIN)
#include <winsock2.h>

#if BYTE_ORDER == LITTLE_ENDIAN

#define fmc_htobe16(x) htons(x)
#define fmc_htole16(x) (x)
#define fmc_be16toh(x) ntohs(x)
#define fmc_le16toh(x) (x)

#define fmc_htobe32(x) htonl(x)
#define fmc_htole32(x) (x)
#define fmc_be32toh(x) ntohl(x)
#define fmc_le32toh(x) (x)

#define fmc_htobe64(x) htonll(x)
#define fmc_htole64(x) (x)
#define fmc_be64toh(x) ntohll(x)
#define fmc_le64toh(x) (x)

#elif BYTE_ORDER == BIG_ENDIAN

#define fmc_htobe16(x) (x)
#define fmc_htole16(x) __builtin_bswap16(x)
#define fmc_be16toh(x) (x)
#define fmc_le16toh(x) __builtin_bswap16(x)

#define fmc_htobe32(x) (x)
#define fmc_htole32(x) __builtin_bswap32(x)
#define fmc_be32toh(x) (x)
#define fmc_le32toh(x) __builtin_bswap32(x)

#define fmc_htobe64(x) (x)
#define fmc_htole64(x) __builtin_bswap64(x)
#define fmc_be64toh(x) (x)
#define fmc_le64toh(x) __builtin_bswap64(x)

#else

#error byte order not supported

#endif

#define FMC_BYTE_ORDER BYTE_ORDER
#define FMC_BIG_ENDIAN BIG_ENDIAN
#define FMC_LITTLE_ENDIAN LITTLE_ENDIAN
#define FMC_PDP_ENDIAN PDP_ENDIAN
#else

#error platform not supported

#endif
