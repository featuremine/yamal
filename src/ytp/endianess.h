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
