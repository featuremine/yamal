"""
        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

from typing import Optional, Callable, Tuple


class stream:
    ''' YTP Stream '''

    def __init__(self) -> None: ...

    def write(self, time: int, data: bytes) -> None:
        ''' Write message to YTP '''
        pass

    def channel(self) -> 'channel':
        ''' Obtain channel related to stream '''
        pass

    def peer(self) -> 'peer':
        ''' Obtain peer related to stream '''
        pass


class channel:
    ''' YTP Channel '''

    def __init__(self) -> None: ...

    def name(self) -> Optional[str]:
        ''' Channel name '''
        pass

    def id(self) -> int:
        ''' Channel id '''
        pass

    def data_callback(self, clbl: Callable[['peer', 'channel', int, bytes], None]) -> None:
        ''' Set callback for data in channel '''
        pass


class peer:
    ''' YTP Peer '''

    def __init__(self) -> None: ...

    def name(self) -> Optional[str]:
        ''' Peer name '''
        pass

    def id(self) -> int:
        ''' Peer id '''
        pass

    def stream(self, ch: channel) -> stream:
        ''' Obtain stream for desired channel '''
        pass

    def channel(self, time: int, channel_name: str) -> channel:
        ''' Obtain desired channel by name '''
        pass


class sequence:
    ''' YTP Sequence '''

    def __init__(self, file_path: str) -> None: ...

    def peer_callback(self, clbl: Callable[[peer], None]) -> None:
        ''' Set callback for peers in YTP file '''
        pass

    def channel_callback(self, clbl: Callable[[channel], None]) -> None:
        ''' Set callback for channels in YTP file '''
        pass

    def data_callback(self, pattern: str, clbl: Callable[['peer', 'channel', int, bytes], None]) -> None:
        ''' Set callback for data by channel pattern '''
        pass

    def peer(self, peer_name: str) -> peer:
        ''' Obtain desired peer by name '''
        pass

    def poll(self) -> bool:
        ''' Poll for messages in sequence file '''
        pass


class transactions:
    ''' YTP transactions '''

    def __init__(self, file_path: str) -> None: ...

    def subscribe(self, pattern: str) -> None:
        ''' Subscribe to desired pattern '''
        pass

    def __iter__(self) -> "transactions":
        pass

    def __next__(self) -> Optional[Tuple[peer, channel, int, bytes]]:
        pass
