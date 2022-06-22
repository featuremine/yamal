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

import msgpack
import ytp
from typing import Any


def tail(transact_file: str) -> Any:
    ''' tail CLI methid '''
    seq = ytp.sequence(transact_file)  # type: ignore[name-defined]
    i = 0

    def msg_clbck(*msg: Any) -> None:
        nonlocal i
        i += 1
        m = msgpack.unpackb(msg[3])
        print(i, msg[0], msg[1], msg[2], m)
    seq.data_callback("/", msg_clbck)
    while True:
        seq.poll()
