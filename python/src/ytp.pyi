"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

    def __init__(self, file_path: str, readonly: Optional[bool]) -> None: ...

    def peer_callback(self, clbl: Callable[['peer', str], None]) -> None:
        ''' Set callback for peers in YTP file '''
        pass

    def channel_callback(self, clbl: Callable[['channel', 'peer', int, str], None]) -> None:
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

    def remove_callbacks(self) -> bool:
        ''' Remove all the registered callbacks '''
        pass


class transactions:
    ''' YTP transactions '''

    def __init__(self, file_path: str, readonly: Optional[bool]) -> None: ...

    def subscribe(self, pattern: str) -> None:
        ''' Subscribe to desired pattern '''
        pass

    def __iter__(self) -> "transactions":
        pass

    def __next__(self) -> Optional[Tuple[peer, channel, int, bytes]]:
        pass
