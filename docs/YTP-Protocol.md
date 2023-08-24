YTP or Yamal Transport Protocol in the Featuremine Ecosystem is a
publish-subscribe protocol that facilitates subscription based
message transport between multiple peers on top of Yamal.

### Table of Contents

- [Requeriments](#requirements)
- [Layers](#layers)
  - [Yamal (Layer 0)](#yamal-layer-0)
  - [Time (Layer 1)](#time-layer-1)
  - [Stream (Layer 2)](#stream-layer-2)
      - [List 0 (data)](#list-0-list-0-data)
      - [List 1 (announcement)](#list-1-announcement)
      - [List 2 (subscription)](#list-2-subscription)
      - [List 3 (index)](#list-3-index)
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

All fields specified in this section use big endian byte order.

### Yamal (Layer 0)

The transaction messaging bus Yamal. Every message includes a sequence number.

<table>
    <tr>
        <th colspan="2">yamal layer</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Data</td>
    </tr>
</table>

### Time (Layer 1)

Timestamp is the original time of the message. Whenever the message is copied or forwarded, it should maintain the timestamp.

<table>
    <tr>
        <th>yamal layer</th>
        <th colspan="2">time layer</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Timestamp</td>
        <td>Data</td>
    </tr>
</table>

### Stream (Layer 2)

Uses 4 yamal lists:
  - List 0: used for **user data** messages.
  - List 1: used for **stream** announcements.
  - List 2: used for **subscription** messages.
  - List 3: used for **index** messages.

#### List 0 (data)

The list 0 of the yamal file has all the user data messages for all the streams.

<table>
    <tr>
        <th>yamal layer</th>
        <th>time layer</th>
        <th colspan="2">data payload</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Timestamp</td>
        <td>Stream ID</td>
        <td>Data</td>
    </tr>
</table>

#### List 1 (announcement)

A peer announces a stream.

<table>
    <tr>
        <th>yamal layer</th>
        <th colspan="7">announcement message</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>4 bytes</td>
        <td>4 bytes</td>
        <td>variable</td>
        <td>variable</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Original ID</td>
        <td>Subscription ID</td>
        <td>Peer name length</td>
        <td>Channel name length</td>
        <td>Peer name</td>
        <td>Channel name</td>
        <td>Stream encoding</td>
    </tr>
</table>

The peer needs to avoid publishing duplicated stream announcements messages for the same stream. The first stream announcement supersedes following announcements.

The ID of a stream is the offset in the file of the first message.

Stream encoding is specified using [Channel Metadata Protocol](#channel-metadata-protocol) (see bellow).

#### List 2 (subscription)

A subscription announcement to a stream.

<table>
    <tr>
        <th>yamal layer</th>
        <th>subscribe message</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Stream ID</td>
    </tr>
</table>

#### List 3 (index)

An index message that references a message in the data list.

<table>
    <tr>
        <th>yamal layer</th>
        <th colspan="3">index message</th>
    </tr>
    <tr>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>8 bytes</td>
        <td>variable</td>
    </tr>
    <tr>
        <td>Seqno</td>
        <td>Stream ID</td>
        <td>Data message offset</td>
        <td>Payload</td>
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
