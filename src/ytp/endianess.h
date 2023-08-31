/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include <fmc/endianness.h>

#if !defined(YTP_USE_BIG_ENDIAN)
#define ye64toh(x) fmc_le64toh(x)
#define htoye64(x) fmc_htole64(x)
#define ye32toh(x) fmc_le32toh(x)
#define htoye32(x) fmc_htole32(x)
#define ye16toh(x) fmc_le16toh(x)
#define htoye16(x) fmc_htole16(x)
#if FMC_BYTE_ORDER == FMC_LITTLE_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#else
#define ye64toh(x) fmc_be64toh(x)
#define htoye64(x) fmc_htobe64(x)
#define ye32toh(x) fmc_be32toh(x)
#define htoye32(x) fmc_htobe32(x)
#define ye16toh(x) fmc_be16toh(x)
#define htoye16(x) fmc_htobe16(x)
#if FMC_BYTE_ORDER == FMC_BIG_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#endif
