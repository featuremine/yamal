The Yamal Python API exposes the following components:

- [Yamal module](#yamal-module)
  - [YTP module](#ytp-module)
    - [Transaction](#transaction)
    - [Sequence](#sequence)
    - [Peer](#peer)
    - [Channel](#channel)
    - [Stream](#stream)

# yamal module
```python
import yamal
```

## ytp module

```python
from yamal import ytp
```

### Sequence

```python
from yamal.ytp import transactions
```

Initializes a transactions object. 

- file_path: File path as a string
- readonly: Read only mode as a boolean
**return value**: transactions object

```python
tx = transactions(file_path, readonly)
```

#### Subscribe

Subscribe to desired pattern of channel names.

- pattern: Subscription pattern as a string

```python
tx.subscribe(pattern)
```

#### Iteration

Iteration protocol for the transactions object allows you to fetch new messages from the Yamal queue using the transactions object as an iterator either manually or in a for loop.

```python
# Manually:
it = iter(tx)
dat = next(it)
if dat is not None:
    p, ch, time, data = dat

# Using a for loop:
for dat in tx:
    if dat is not None:
        p, ch, time, data = dat
```

### Sequence

YTP Sequence

```python
from yamal.ytp import sequence
```

Initializes a sequence object. 

- file_path: File path as a string
- readonly: Read only mode as a boolean
**return value**: sequence object

```python
seq = sequence(file_path, readonly)
```

#### Peer callbacks

Set callback for peers in YTP file

- callback: Callback to trigger when a peer is discovered

```python
def callback(ch: peer) -> None:
    ...
seq.peer_callback(callback)
```

#### Channel callbacks

Set callback for channels in YTP file

- callback: Callback to trigger when a channel is discovered

```python
def callback(p: channel) -> None:
    ...
seq.channel_callback(callback)
```

#### Data callbacks

Set callback for data by channel pattern

- pattern: Channel pattern as a string
- callback: Callback to trigger with discovered data

```python
def callback(p: peer, ch: channel, ts: int, data: bytes) -> None:
    ...
seq.data_callback(pattern, callback)
```

#### Peer creation

Obtain desired peer by name

- peer_name: Desired peer name as a string
**return value**: peer object

```python
p = seq.peer(peer_name)
```

#### Polling

Poll for messages in sequence file

**return value**: boolean signaling if the sequence processed any messages

```python
polled = seq.poll()
```

#### Remove Callbacks

Remove all the registered callbacks

```python
seq.remove_callbacks()
```

### Peer

YTP Peer

```python
from yamal.ytp import peer
```

#### Name
```python
name = p.name()
```

#### Identifier
```python
identifier = p.id()
```

#### Obtain channel
```python
ch = p.channel(time, ch_name)
```

#### Obtain stream
```python
s = p.stream(ch)
```

### Channel

```python
from yamal.ytp import channel # for access to the class
ch = p.channel(time, peer_name)
```

#### Name
```python
name = ch.name()
```

#### Identifier
```python
identifier = ch.id()
```

#### Data callbacks
```python
ch.data_callback(callback)
```

### Stream

```python
from yamal.ytp import stream # for access to the class
s = p.stream(ch)
```

#### Write
```python
s.write(time, data)
```

#### Channel
```python
ch = s.channel()
```

#### Peer
```python
p = s.peer()
```
