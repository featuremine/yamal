"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

"""

"""
@package pyext.py
@author Maxim Trokhimtchouk
@date 5 Sep 2018
@brief File contains YTP python test
"""

import os
import tempfile
import ytp
import ytp_base
import pathlib


def get_tempfile_name(some_id='ttt'):
    return os.path.join(tempfile.gettempdir(), next(tempfile._get_candidate_names()) + "_" + some_id)


def data_simple_subscription_1():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    stream1.write(1000, b"ABCD")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("main/", seq_clbck)

    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def data_simple_subscription_2():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    stream1.write(1000, b"ABCD")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    channel1.data_callback(seq_clbck)

    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def data_multiple_channel_1():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0
    stream2 = producer1.stream(channel2)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("main/", seq_clbck)

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def data_multiple_channel_2():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0
    stream2 = producer1.stream(channel2)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("secondary/", seq_clbck)

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel2.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"EFGH"


def data_multiple_channel_3():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0
    stream2 = producer1.stream(channel2)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    channel1.data_callback(seq_clbck)

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def data_multiple_producers_1():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    producer2 = sequence.peer("producer2")
    assert producer2.id() != 0

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0
    stream2 = producer1.stream(channel2)

    channel3 = producer2.channel(0, "main/channel1")
    assert channel3.id() != 0
    stream3 = producer2.stream(channel3)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")
    stream3.write(1000, b"IJKL")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("main/", seq_clbck)

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Peer producer2
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll()  # IJKL message
    assert sequence.poll() == False

    assert len(output) == 2
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"

    assert output[1][0].id() == producer2.id()
    assert output[1][1].id() == channel1.id()
    assert output[1][2] == 1000
    assert output[1][3] == b"IJKL"


def data_multiple_producers_2():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    producer2 = sequence.peer("producer2")
    assert producer2.id() != 0

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    stream1 = producer1.stream(channel1)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0
    stream2 = producer1.stream(channel2)

    channel3 = producer2.channel(0, "main/channel1")
    assert channel3.id() != 0
    stream3 = producer2.stream(channel3)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")
    stream3.write(1000, b"IJKL")

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("main/", seq_clbck)

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Peer producer2
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll()  # IJKL message
    assert sequence.poll() == False

    assert len(output) == 2
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"

    assert output[1][0].id() == producer2.id()
    assert output[1][1].id() == channel1.id()
    assert output[1][2] == 1000
    assert output[1][3] == b"IJKL"


def data_subscription_first_1():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("main/", seq_clbck)

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0
    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0

    stream1 = producer1.stream(channel1)
    stream2 = producer1.stream(channel2)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def data_subscription_first_2():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    assert sequence.peer("consumer1").id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    output = []

    def seq_clbck(*args):
        nonlocal output
        output += [args]

    channel1 = producer1.channel(0, "main/channel1")
    assert channel1.id() != 0

    channel1.data_callback(seq_clbck)

    channel2 = producer1.channel(0, "secondary/channel2")
    assert channel2.id() != 0

    stream1 = producer1.stream(channel1)
    stream2 = producer1.stream(channel2)

    stream1.write(1000, b"ABCD")
    stream2.write(1000, b"EFGH")

    assert sequence.poll()  # Peer consumer1
    assert sequence.poll()  # Peer producer1
    assert sequence.poll()  # Channel main/channel1
    assert sequence.poll()  # Channel secondary/channel2
    assert sequence.poll()  # ABCD message
    assert sequence.poll()  # EFGH message
    assert sequence.poll() == False

    assert len(output) == 1
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1000
    assert output[0][3] == b"ABCD"


def channel_simple():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    ch_output = []

    def ch_clbck(*args):
        nonlocal ch_output
        ch_output += [args]

    sequence.channel_callback(ch_clbck)

    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    channel1 = producer1.channel(1001, "main/channel1")
    assert channel1.id() != 0
    channel2 = producer1.channel(1002, "secondary/channel2")
    assert channel2.id() != 0

    assert channel1.name() == "main/channel1"
    assert channel2.name() == "secondary/channel2"

    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()

    assert len(ch_output) == 2
    assert ch_output[0][0].id() == channel1.id()
    assert ch_output[1][0].id() == channel2.id()


def channel_wildcard():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    ch_output = []

    consumer1 = sequence.peer("consumer1")
    assert consumer1.id() != 0
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    output = []

    def data_clbck(*args):
        nonlocal output
        output += [args]

    sequence.data_callback("/", data_clbck)

    channel1 = producer1.channel(1001, "main/channel1")
    assert channel1.id() != 0
    channel2 = producer1.channel(1002, "secondary/channel2")
    assert channel2.id() != 0

    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll() == False

    wildcard_producer_output = []

    def wildcard_producer_data_clbck(*args):
        nonlocal wildcard_producer_output
        wildcard_producer_output += [args]

    sequence.data_callback("/", wildcard_producer_data_clbck)

    assert len(output) == 0

    stream1 = producer1.stream(channel1)
    stream2 = producer1.stream(channel2)

    stream1.write(1003, b"ABCD")
    stream2.write(1004, b"EFGH")

    assert sequence.poll()
    assert sequence.poll()
    assert sequence.poll() == False

    assert len(output) == 2
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1003
    assert output[0][3] == b"ABCD"

    assert output[1][0].id() == producer1.id()
    assert output[1][1].id() == channel2.id()
    assert output[1][2] == 1004
    assert output[1][3] == b"EFGH"

    assert len(wildcard_producer_output) == 2
    assert wildcard_producer_output[0][0].id() == producer1.id()
    assert wildcard_producer_output[0][1].id() == channel1.id()
    assert wildcard_producer_output[0][2] == 1003
    assert wildcard_producer_output[0][3] == b"ABCD"

    assert wildcard_producer_output[1][0].id() == producer1.id()
    assert wildcard_producer_output[1][1].id() == channel2.id()
    assert wildcard_producer_output[1][2] == 1004
    assert wildcard_producer_output[1][3] == b"EFGH"


def peer_simple():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    peer_output = []

    def peer_clbck(*args):
        nonlocal peer_output
        peer_output += [args]

    sequence.peer_callback(peer_clbck)

    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    producer2 = sequence.peer("producer2")
    assert producer2.id() != 0

    assert producer1.name() == "producer1"
    assert producer2.name() == "producer2"

    assert sequence.poll()
    assert sequence.poll()

    assert len(peer_output) == 2
    assert peer_output[0][0].id() == producer1.id()
    assert peer_output[1][0].id() == producer2.id()


def idempotence_simple():
    sequence_file = get_tempfile_name('sequence')
    sequence = ytp.sequence(sequence_file)

    consumer1 = sequence.peer("consumer1")
    assert consumer1.id() != 0
    consumer1_2 = sequence.peer("consumer1")
    assert consumer1_2.id() != 0
    assert consumer1.id() == consumer1_2.id()

    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    producer1_2 = sequence.peer("producer1")
    assert producer1_2.id() != 0
    assert producer1.id() == producer1_2.id()

    channel1 = producer1.channel(1001, "main/channel1")
    assert channel1.id() != 0
    channel1_2 = producer1.channel(1001, "main/channel1")
    assert channel1_2.id() != 0
    assert channel1.id() == channel1_2.id()

    stream1 = producer1.stream(channel1)

    stream1.write(1000, b"ABCD")

    cb_output = []

    def cb(*args):
        nonlocal cb_output
        cb_output += [args]

    def cb2(*args):
        nonlocal cb_output
        cb_output += [args]

    sequence.data_callback("main/", cb)
    sequence.data_callback("main/", cb2)

    consumer2 = sequence.peer("consumer2")
    assert consumer2.id() != 0
    consumer2_2 = sequence.peer("consumer2")
    assert consumer2_2.id() != 0
    assert consumer2.id() == consumer2_2.id()

    producer2 = sequence.peer("producer2")
    assert producer2.id() != 0
    producer2_2 = sequence.peer("producer2")
    assert producer2_2.id() != 0
    assert producer2.id() == producer2_2.id()

    channel2 = producer2.channel(1001, "main/channel2")
    assert channel2.id() != 0
    channel2_2 = producer2.channel(1001, "main/channel2")
    assert channel2_2.id() != 0
    assert channel2.id() == channel2_2.id()

    stream2 = producer2.stream(channel2)

    stream2.write(1000, b"EFGH")

    sequence.poll()  # consumer1
    sequence.poll()  # producer1
    sequence.poll()  # channel1
    sequence.poll()  # data ABCD
    sequence.poll()  # consumer2
    sequence.poll()  # producer2
    sequence.poll()  # channel2
    sequence.poll()  # data EFGH
    sequence.poll() == False

    assert len(cb_output) == 4
    assert cb_output[0][0].id() == producer1.id()
    assert cb_output[0][1].id() == channel1.id()
    assert cb_output[0][2] == 1000
    assert cb_output[0][3] == b"ABCD"

    assert cb_output[1][0].id() == producer1.id()
    assert cb_output[1][1].id() == channel1.id()
    assert cb_output[1][2] == 1000
    assert cb_output[1][3] == b"ABCD"

    assert cb_output[2][0].id() == producer2.id()
    assert cb_output[2][1].id() == channel2.id()
    assert cb_output[2][2] == 1000
    assert cb_output[2][3] == b"EFGH"

    assert cb_output[3][0].id() == producer2.id()
    assert cb_output[3][1].id() == channel2.id()
    assert cb_output[3][2] == 1000
    assert cb_output[3][3] == b"EFGH"


def transactions_wrapper():
    sequence_file = get_tempfile_name('sequence')
    tr = ytp.transactions(sequence_file)
    assert next(tr) is None

    sequence = ytp.sequence(sequence_file)
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0
    assert next(tr) is None

    channel1 = producer1.channel(1000, "main/channel1")
    assert channel1.id() != 0
    assert next(tr) is None

    stream1 = producer1.stream(channel1)

    stream1.write(1001, b"ABCD")
    assert next(tr) is None

    tr.subscribe("/")
    assert next(tr) is None

    stream1.write(1001, b"EFGH")
    data = next(tr)
    assert len(data) == 4
    assert data[0].id() == producer1.id()
    assert data[1].id() == channel1.id()
    assert data[2] == 1001
    assert data[3] == b"EFGH"

    channel2 = producer1.channel(1002, "secondary/channel2")
    assert channel2 != 0
    assert next(tr) is None

    stream2 = producer1.stream(channel2)

    stream2.write(1002, b"IJKL")
    stream1.write(1003, b"MNOP")

    data = next(tr)
    assert len(data) == 4
    assert data[0].id() == producer1.id()
    assert data[1].id() == channel2.id()
    assert data[2] == 1002
    assert data[3] == b"IJKL"

    data = next(tr)
    assert len(data) == 4
    assert data[0].id() == producer1.id()
    assert data[1].id() == channel1.id()
    assert data[2] == 1003
    assert data[3] == b"MNOP"

    assert next(tr) is None


def transactions_wrapper_iteration():
    sequence_file = get_tempfile_name('sequence')

    sequence = ytp.sequence(sequence_file)
    producer1 = sequence.peer("producer1")
    assert producer1.id() != 0

    channel1 = producer1.channel(1000, "main/channel1")
    assert channel1.id() != 0

    stream1 = producer1.stream(channel1)
    stream1.write(1001, b"ABCD")

    channel2 = producer1.channel(1002, "secondary/channel2")
    assert channel2.id() != 0

    stream2 = producer1.stream(channel2)

    stream2.write(1002, b"EFGH")
    stream1.write(1003, b"IJKL")

    tr = ytp.transactions(sequence_file)
    tr.subscribe("/")
    output = []
    for data in tr:
        if data is None:
            break
        output += [data]

    assert len(output) == 3
    assert output[0][0].id() == producer1.id()
    assert output[0][1].id() == channel1.id()
    assert output[0][2] == 1001
    assert output[0][3] == b"ABCD"

    assert output[1][0].id() == producer1.id()
    assert output[1][1].id() == channel2.id()
    assert output[1][2] == 1002
    assert output[1][3] == b"EFGH"

    assert output[2][0].id() == producer1.id()
    assert output[2][1].id() == channel1.id()
    assert output[2][2] == 1003
    assert output[2][3] == b"IJKL"


def type_validation_1():
    try:
        ytp.sequence(pathlib.Path(get_tempfile_name('sequence')))
    except TypeError:
        return

    raise AssertionError("ytp.sequence should throw an exception when using pathlib.Path")


def peer_sequential():
    test_batch = 10000
    batch_count = 500
    test_size = test_batch * batch_count

    peer_file = get_tempfile_name('peer')
    p = ytp_base.peer(peer_file)

    for i in range(1, test_size):
        peer_id = i + 1000
        p.write(peer_id, i.to_bytes(10, byteorder='big'))

    peer_two = ytp_base.peer(peer_file)
    count = 1
    last_idx = 0
    while True:
        msg = peer_two.read()
        if msg is None:
            break
        assert msg[0] == count + 1000
        idx = int.from_bytes(msg[1], byteorder='big')
        assert idx == count
        last_idx = idx
        count = count + 1

    assert last_idx == test_size - 1
    assert count == test_size


def permissions():

    file_path = get_tempfile_name('permissions')

    # To restrict scope of seq and delete it
    def write(path):
        seq = ytp.sequence(path)
        p = seq.peer("peer_name")
        ch = p.channel(0, "ch_name")
        p.stream(ch).write(0, b"data")

        return (p.id(), ch.id(), 0, b"data")

    message = write(file_path)

    os.chmod(file_path, 0o444)

    seq = ytp.sequence(file_path, readonly=True)

    messages = []

    def seq_clbck(p, ch, time, data):
        nonlocal messages
        messages.append((p.id(), ch.id(), time, data))

    seq.data_callback("/", seq_clbck)

    while seq.poll():
        pass

    assert len(messages) == 1
    assert messages[0] == message


def non_existing_file():

    file_path = get_tempfile_name('non_existing_file')

    try:
        seq = ytp.sequence(file_path, readonly=True)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Sequence open over non existing file did not fail as expected")


def empty_file():

    file_path = get_tempfile_name('empty_file')

    pathlib.Path(file_path).touch()

    try:
        seq = ytp.sequence(file_path, readonly=True)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Sequence open over empty file did not fail as expected")


if __name__ == "__main__":

    data_simple_subscription_1()
    data_simple_subscription_2()
    data_multiple_channel_1()
    data_multiple_channel_2()
    data_multiple_channel_3()
    data_multiple_producers_1()
    data_multiple_producers_2()
    data_subscription_first_1()
    data_subscription_first_2()
    channel_simple()
    channel_wildcard()
    peer_simple()
    idempotence_simple()
    transactions_wrapper()
    transactions_wrapper_iteration()
    type_validation_1()
    peer_sequential()
    permissions()
    non_existing_file()
    empty_file()
