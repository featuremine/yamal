"""
        COPYRIGHT (c) 2020 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

import ytp
import msgpack
from typing import Any, Optional


def get(transact_file: str, seq: str, unpack: str = "False") -> Any:
    ''' Get CLI method '''
    seqnum = int(seq)
    ytp_seq = ytp.sequence(transact_file)  # type: ignore[name-defined]
    count = 0
    found = False
    ret: Optional[Any] = None

    def msg_clbck(*msg: Any) -> None:
        nonlocal count
        nonlocal seqnum
        nonlocal found
        nonlocal ret
        count += 1
        if count == seqnum:
            if unpack == "True":
                ret = [msg[0], msg[1], msg[2], msgpack.unpackb(msg[3])]
            else:
                ret = msg
            found = True

    ytp_seq.data_callback("/", msg_clbck)

    while not found and ytp_seq.poll():
        pass

    if ret is not None:
        return ret

    return "Unable to find transaction"
