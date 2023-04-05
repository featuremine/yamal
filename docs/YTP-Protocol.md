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

#### channel

A peer announces a channel.

<table>
    <tr>
        <th colspan="3">peer/channel/time header</th>
        <th>channel announcement payload</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Ctrl Channel ID = 0</td>
        <td>Timestamp</td>
        <td>Channel name</td>
    </tr>
</table>

The peer needs to avoid publishing duplicated channel messages if the last channel message in yamal has the same payload.

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
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Ctrl Channel ID = 1</td>
        <td>Timestamp</td>
        <td>Payload</td>
    </tr>
</table>

**Payload**: If it doesn't end with character "", a channel name; otherwise, a prefix.

The peer needs to avoid publishing duplicated subscribe messages if the last subscription message in yamal has the same payload.

#### directory

Describes the messages that a particular peer will publish on a
particular channel.

<table>
    <tr>
        <th colspan="3">peer/channel/time header</th>
        <th>directory payload</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Peer ID</td>
        <td>Ctrl Channel ID = 2</td>
        <td>Timestamp</td>
        <td>Payload</td>
    </tr>
</table>

**Payload**: An Simple Channel Directory Protocol (SCDP) encoded string.

Take a look at [Simple Channel Directory Protocol](#simple-channel-directory-protocol). This field can be empty.

The peer needs to avoid publishing duplicated directory message if the last directory message in yamal has the same payload.

### Simple Channel Directory Protocol

A Simple Channel Directory Protocol (SCDP) is a new line separated string where each line is composed of a metakey, followed by a space character, followed by metadata. It can be empty. For example:

```
CHANNEL marketdata/IBM
APP ORE1.2.4
ENCODING messagepack
SCHEMATYPE AVRO
SCHEMA {int}
```
