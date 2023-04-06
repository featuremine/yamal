### Publishing messages

To be able to write messages to specific channels using specific peers, the sequence API can be used used instead.

```python
from yamal import ytp

#Create sequence object using the desired ytp file path
sequence = ytp.sequence(file_path)

#Create a peer object using the desired peer name with the help of your sequence object
peer = sequence.peer(peer_name)

#Create a channel object using the desired channel name with the help of your peer object
channel = peer.channel(current_time_ns, channel_name)

#Create a stream object that will be used to write to the desired channel identifying yourself as the desired peer
publishing_stream = peer.stream(channel)

#Write the desired data to YTP using the desired publishing time.
stream.write(current_time_ns, message_as_bytes)
```

### Receiving messages

The API allows for data callbacks for a specific channel to be set up. These callbacks will not be triggered with incoming data that belongs to other channels. It can be used in the following way:

```python
from yamal import ytp

#Create sequence object using the desired ytp file path
sequence = ytp.sequence(file_path)

#Create a peer object using the desired peer name with the help of your sequence object
peer = sequence.peer(peer_name)

#Create a channel object using the desired channel name with the help of your peer object
channel = peer.channel(current_time_ns, channel_name)

#Set up a callback to be called for every message written to the specified channel
channel.data_callback(your_callback)

#Poll the sequence object and process the next message. Returns True if processed a message, False if no messages were processed
sequence.poll()
```
