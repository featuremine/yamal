YTP or Yamal Transport Protocol in the Featuremine Ecosystem is a
publish-subscribe protocol that facilitates subscription based
message transport between multiple peers on top of Yamal.

### Table of Contents

- [Requeriments](#requirements)
- [Layers](#layers)
  - [Yamal (Layer 0)](#yamal-layer-0)
  - [Peer (Layer 1)](#peer-layer-1)
  - [Channel (Layer 2)](#channel-layer-2)
  - [Time (Layer 3)](#time-layer-3)
  - [Control (Layer 4)](#control-layer-4)
    - [channel](#channel)
    - [subscribe](#subscribe)
    - [directory](#directory)
- [Simple Channel Directory Protocol](#simple-channel-directory-protocol)

### Requirements

-   Atomic: A guarantee of atomicity prevents updates to the bus
    occurring only partially.
-   Non-blocking: Memory message reserve and commit are not blocking.
-   Data consistency across processes.
-   Serialized:
-   Persistent: Upon a crash, peers can continue to operate normally.
-   Sequential: Message access and storage are in a chronological order.
-   Idempotent: Any YTP protocol, including application layer protocols,
    need to be idempotent. That is, if the same process is run twice
    over a yamal file, it will update it only once.
-   Zero-copy: data is not copied during writing or reading.
-   Availability: Data pointers stay valid at all times until
    application shutdown

## Layers

All fields specified in this section use network byte order.

### Yamal (Layer 0)

The transaction messaging bus Yamal.

### Peer (Layer 1)

A peer uniquely identifies a source of a messages.

<table>
    <tr>
        <th colspan="2">peer</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Data</td>
    </tr>
</table>

If Peer ID = 0, then it is an announcement message where data is the peer name.

<table>
    <tr>
        <th colspan="2">peer</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID = 0</td>
        <td>Peer name</td>
    </tr>
</table>

The peer needs to avoid publishing duplicated announcement messages if there is an announcement message in yamal.

### Channel (Layer 2)

A channel uniquely identifies a logical partition of a set of messages.

<table>
    <tr>
        <th colspan="3">peer/channel</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Channel ID</td>
        <td>Data</td>
    </tr>
</table>

### Time (Layer 3)

Timestamp is the original time of the message. Whenever the message is copied or forwarded, it should maintain the timestamp.

<table>
    <tr>
        <th colspan="4">peer/channel/time</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Channel ID</td>
        <td>Timestamp</td>
        <td>Data</td>
    </tr>
</table>

### Control (Layer 4)

A control channel is used for communicating peer, channel, publisher and subscription control information.

A publisher is a peer that may publish messages on a channel.

<table>
    <tr>
        <th colspan="3">peer/channel/time header</th>
        <th>control</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Channel ID</td>
        <td>Timestamp</td>
        <td>Payload</td>
    </tr>
</table>

#### stream

A peer announces a stream.

<table>
    <tr>
        <th colspan="3">peer/channel/time header</th>
        <th colspan="3">channel announcement payload</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>2 bytes</td>
        <td>variable</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Ctrl Channel ID = 0</td>
        <td>Timestamp</td>
        <td>Channel name length</td>
        <td>Channel name</td>
        <td>Stream encoding</td>
    </tr>
</table>

The peer needs to avoid publishing duplicated channel announcements messages for the same stream. The first channel announcement for a stream supersedes following announcements. Stream encoding is specified using [Channel Metadata Protocol](#channel-metadata-protocol) (see bellow).

#### subscribe

A peer announces a subscription to a channel.

<table>
    <tr>
        <th colspan="3">peer/channel/time header</th>
        <th>subscribe payload</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Ctrl Channel ID = 1</td>
        <td>Timestamp</td>
        <td>Channel ID</td>
    </tr>
</table>

### Channel Metadata Protocol

A Channel Metadata Protocol (CMP) is a new line separated string where each line is composed of a metakey, followed by a space character, followed by metadata. It can be empty. For example:

```
Content-Type application/msgpack
Content-Encoding gzip
Content-Schema ore1.2.4
Schema-Type AVRO
Schema {int}
```
[CONTENT-TYPE](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type)
