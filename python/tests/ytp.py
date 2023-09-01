"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

"""
@package ytp.py
@author Maxim Trokhimtchouk
@date 5 Sep 2018
@brief File contains YTP python test
"""

import os
import tempfile
from yamal import ytp
import pathlib
import unittest


def get_tempfile_name(some_id='ttt'):
    return os.path.join(tempfile.gettempdir(), next(tempfile._get_candidate_names()) + "_" + some_id)


class data_simple_subscription(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        stream1.write(1000, b"ABCD")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("main/", seq_clbck)

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


    def test_2(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        stream1.write(1000, b"ABCD")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        channel1.data_callback(seq_clbck)

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


class data_multiple_channel(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)
        stream2 = producer1.stream(channel2)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("main/", seq_clbck)

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


    def test_2(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)
        stream2 = producer1.stream(channel2)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("secondary/", seq_clbck)

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel2.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"EFGH")


    def test_3(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)
        stream2 = producer1.stream(channel2)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        channel1.data_callback(seq_clbck)

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


class data_multiple_producers(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        producer2 = sequence.peer("producer2")
        self.assertNotEqual(producer2.id(), 0)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)
        stream2 = producer1.stream(channel2)

        channel3 = producer2.channel(0, "main/channel1")
        self.assertNotEqual(channel3.id(), 0)
        stream3 = producer2.stream(channel3)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")
        stream3.write(1000, b"IJKL")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("main/", seq_clbck)

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Peer producer2
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertTrue(sequence.poll())  # IJKL message
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 2)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")

        self.assertEqual(output[1][0].id(), producer2.id())
        self.assertEqual(output[1][1].id(), channel1.id())
        self.assertEqual(output[1][2], 1000)
        self.assertEqual(output[1][3], b"IJKL")


    def test_2(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        producer2 = sequence.peer("producer2")
        self.assertNotEqual(producer2.id(), 0)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        stream1 = producer1.stream(channel1)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)
        stream2 = producer1.stream(channel2)

        channel3 = producer2.channel(0, "main/channel1")
        self.assertNotEqual(channel3.id(), 0)
        stream3 = producer2.stream(channel3)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")
        stream3.write(1000, b"IJKL")

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("main/", seq_clbck)

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Peer producer2
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertTrue(sequence.poll())  # IJKL message
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 2)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")

        self.assertEqual(output[1][0].id(), producer2.id())
        self.assertEqual(output[1][1].id(), channel1.id())
        self.assertEqual(output[1][2], 1000)
        self.assertEqual(output[1][3], b"IJKL")


class data_subscription_first(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("main/", seq_clbck)

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)

        stream1 = producer1.stream(channel1)
        stream2 = producer1.stream(channel2)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


    def test_2(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        self.assertNotEqual(sequence.peer("consumer1").id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        output = []

        def seq_clbck(*args):
            nonlocal output
            output += [args]

        channel1 = producer1.channel(0, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)

        channel1.data_callback(seq_clbck)

        channel2 = producer1.channel(0, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)

        stream1 = producer1.stream(channel1)
        stream2 = producer1.stream(channel2)

        stream1.write(1000, b"ABCD")
        stream2.write(1000, b"EFGH")

        self.assertTrue(sequence.poll())  # Peer consumer1
        self.assertTrue(sequence.poll())  # Peer producer1
        self.assertTrue(sequence.poll())  # Channel main/channel1
        self.assertTrue(sequence.poll())  # Channel secondary/channel2
        self.assertTrue(sequence.poll())  # ABCD message
        self.assertTrue(sequence.poll())  # EFGH message
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 1)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1000)
        self.assertEqual(output[0][3], b"ABCD")


class channel_simple(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        ch_output = []

        def ch_clbck(*args):
            nonlocal ch_output
            ch_output += [args]

        sequence.channel_callback(ch_clbck)

        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        channel1 = producer1.channel(1001, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        channel2 = producer1.channel(1002, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)

        self.assertEqual(channel1.name(), "main/channel1")
        self.assertEqual(channel2.name(), "secondary/channel2")

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(ch_output), 2)
        self.assertEqual(ch_output[0][0].id(), channel1.id())
        self.assertEqual(ch_output[1][0].id(), channel2.id())


class channel_wildcard(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        ch_output = []

        consumer1 = sequence.peer("consumer1")
        self.assertNotEqual(consumer1.id(), 0)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        output = []

        def data_clbck(*args):
            nonlocal output
            output += [args]

        sequence.data_callback("/", data_clbck)

        channel1 = producer1.channel(1001, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        channel2 = producer1.channel(1002, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        wildcard_producer_output = []

        def wildcard_producer_data_clbck(*args):
            nonlocal wildcard_producer_output
            wildcard_producer_output += [args]

        sequence.data_callback("/", wildcard_producer_data_clbck)

        self.assertEqual(len(output), 0)

        stream1 = producer1.stream(channel1)
        stream2 = producer1.stream(channel2)

        stream1.write(1003, b"ABCD")
        stream2.write(1004, b"EFGH")

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(output), 2)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1003)
        self.assertEqual(output[0][3], b"ABCD")

        self.assertEqual(output[1][0].id(), producer1.id())
        self.assertEqual(output[1][1].id(), channel2.id())
        self.assertEqual(output[1][2], 1004)
        self.assertEqual(output[1][3], b"EFGH")

        self.assertEqual(len(wildcard_producer_output), 2)
        self.assertEqual(wildcard_producer_output[0][0].id(), producer1.id())
        self.assertEqual(wildcard_producer_output[0][1].id(), channel1.id())
        self.assertEqual(wildcard_producer_output[0][2], 1003)
        self.assertEqual(wildcard_producer_output[0][3], b"ABCD")

        self.assertEqual(wildcard_producer_output[1][0].id(), producer1.id())
        self.assertEqual(wildcard_producer_output[1][1].id(), channel2.id())
        self.assertEqual(wildcard_producer_output[1][2], 1004)
        self.assertEqual(wildcard_producer_output[1][3], b"EFGH")


class peer_simple(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        peer_output = []

        def peer_clbck(*args):
            nonlocal peer_output
            peer_output += [args]

        sequence.peer_callback(peer_clbck)

        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        producer2 = sequence.peer("producer2")
        self.assertNotEqual(producer2.id(), 0)

        self.assertEqual(producer1.name(), "producer1")
        self.assertEqual(producer2.name(), "producer2")

        self.assertTrue(sequence.poll())
        self.assertTrue(sequence.poll())
        self.assertFalse(sequence.poll())

        self.assertEqual(len(peer_output), 2)
        self.assertEqual(peer_output[0][0].id(), producer1.id())
        self.assertEqual(peer_output[1][0].id(), producer2.id())


class idempotence_simple(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        sequence = ytp.sequence(sequence_file)

        consumer1 = sequence.peer("consumer1")
        self.assertNotEqual(consumer1.id(), 0)
        consumer1_2 = sequence.peer("consumer1")
        self.assertNotEqual(consumer1_2.id(), 0)
        self.assertEqual(consumer1.id(), consumer1_2.id())

        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        producer1_2 = sequence.peer("producer1")
        self.assertNotEqual(producer1_2.id(), 0)
        self.assertEqual(producer1.id(), producer1_2.id())

        channel1 = producer1.channel(1001, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        channel1_2 = producer1.channel(1001, "main/channel1")
        self.assertNotEqual(channel1_2.id(), 0)
        self.assertEqual(channel1.id(), channel1_2.id())

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
        self.assertNotEqual(consumer2.id(), 0)
        consumer2_2 = sequence.peer("consumer2")
        self.assertNotEqual(consumer2_2.id(), 0)
        self.assertEqual(consumer2.id(), consumer2_2.id())

        producer2 = sequence.peer("producer2")
        self.assertNotEqual(producer2.id(), 0)
        producer2_2 = sequence.peer("producer2")
        self.assertNotEqual(producer2_2.id(), 0)
        self.assertEqual(producer2.id(), producer2_2.id())

        channel2 = producer2.channel(1001, "main/channel2")
        self.assertNotEqual(channel2.id(), 0)
        channel2_2 = producer2.channel(1001, "main/channel2")
        self.assertNotEqual(channel2_2.id(), 0)
        self.assertEqual(channel2.id(), channel2_2.id())

        stream2 = producer2.stream(channel2)

        stream2.write(1000, b"EFGH")

        self.assertTrue(sequence.poll())  # consumer1
        self.assertTrue(sequence.poll())  # producer1
        self.assertTrue(sequence.poll())  # channel1
        self.assertTrue(sequence.poll())  # data ABCD
        self.assertTrue(sequence.poll())  # consumer2
        self.assertTrue(sequence.poll())  # producer2
        self.assertTrue(sequence.poll())  # channel2
        self.assertTrue(sequence.poll())  # data EFGH
        self.assertFalse(sequence.poll())

        self.assertEqual(len(cb_output), 4)
        self.assertEqual(cb_output[0][0].id(), producer1.id())
        self.assertEqual(cb_output[0][1].id(), channel1.id())
        self.assertEqual(cb_output[0][2], 1000)
        self.assertEqual(cb_output[0][3], b"ABCD")

        self.assertEqual(cb_output[1][0].id(), producer1.id())
        self.assertEqual(cb_output[1][1].id(), channel1.id())
        self.assertEqual(cb_output[1][2], 1000)
        self.assertEqual(cb_output[1][3], b"ABCD")

        self.assertEqual(cb_output[2][0].id(), producer2.id())
        self.assertEqual(cb_output[2][1].id(), channel2.id())
        self.assertEqual(cb_output[2][2], 1000)
        self.assertEqual(cb_output[2][3], b"EFGH")

        self.assertEqual(cb_output[3][0].id(), producer2.id())
        self.assertEqual(cb_output[3][1].id(), channel2.id())
        self.assertEqual(cb_output[3][2], 1000)
        self.assertEqual(cb_output[3][3], b"EFGH")


class transactions_wrapper(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')
        tr = ytp.transactions(sequence_file)
        self.assertIsNone(next(tr))

        sequence = ytp.sequence(sequence_file)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)
        self.assertIsNone(next(tr))

        channel1 = producer1.channel(1000, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)
        self.assertIsNone(next(tr))

        stream1 = producer1.stream(channel1)

        stream1.write(1001, b"ABCD")
        self.assertIsNone(next(tr))

        tr.subscribe("/")
        self.assertIsNone(next(tr))

        stream1.write(1001, b"EFGH")
        data = next(tr)
        self.assertEqual(len(data), 4)
        self.assertEqual(data[0].id(), producer1.id())
        self.assertEqual(data[1].id(), channel1.id())
        self.assertEqual(data[2], 1001)
        self.assertEqual(data[3], b"EFGH")

        channel2 = producer1.channel(1002, "secondary/channel2")
        self.assertNotEqual(channel2, 0)
        self.assertIsNone(next(tr))

        stream2 = producer1.stream(channel2)

        stream2.write(1002, b"IJKL")
        stream1.write(1003, b"MNOP")

        data = next(tr)
        self.assertEqual(len(data), 4)
        self.assertEqual(data[0].id(), producer1.id())
        self.assertEqual(data[1].id(), channel2.id())
        self.assertEqual(data[2], 1002)
        self.assertEqual(data[3], b"IJKL")

        data = next(tr)
        self.assertEqual(len(data), 4)
        self.assertEqual(data[0].id(), producer1.id())
        self.assertEqual(data[1].id(), channel1.id())
        self.assertEqual(data[2], 1003)
        self.assertEqual(data[3], b"MNOP")

        self.assertIsNone(next(tr))


class transactions_wrapper_iteration(unittest.TestCase):
    def test_1(self):
        sequence_file = get_tempfile_name('sequence')

        sequence = ytp.sequence(sequence_file)
        producer1 = sequence.peer("producer1")
        self.assertNotEqual(producer1.id(), 0)

        channel1 = producer1.channel(1000, "main/channel1")
        self.assertNotEqual(channel1.id(), 0)

        stream1 = producer1.stream(channel1)
        stream1.write(1001, b"ABCD")

        channel2 = producer1.channel(1002, "secondary/channel2")
        self.assertNotEqual(channel2.id(), 0)

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

        self.assertEqual(len(output), 3)
        self.assertEqual(output[0][0].id(), producer1.id())
        self.assertEqual(output[0][1].id(), channel1.id())
        self.assertEqual(output[0][2], 1001)
        self.assertEqual(output[0][3], b"ABCD")

        self.assertEqual(output[1][0].id(), producer1.id())
        self.assertEqual(output[1][1].id(), channel2.id())
        self.assertEqual(output[1][2], 1002)
        self.assertEqual(output[1][3], b"EFGH")

        self.assertEqual(output[2][0].id(), producer1.id())
        self.assertEqual(output[2][1].id(), channel1.id())
        self.assertEqual(output[2][2], 1003)
        self.assertEqual(output[2][3], b"IJKL")


class type_validation(unittest.TestCase):
    def test_1(self):
        try:
            ytp.sequence(pathlib.Path(get_tempfile_name('sequence')))
        except TypeError:
            return

        raise AssertionError("ytp.sequence should throw an exception when using pathlib.Path")


class permissions(unittest.TestCase):
    def test_1(self):
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

        self.assertEqual(len(messages), 1)
        self.assertEqual(messages[0], message)


class non_existing_file(unittest.TestCase):
    def test_1(self):
        file_path = get_tempfile_name('non_existing_file')

        try:
            seq = ytp.sequence(file_path, readonly=True)
        except RuntimeError:
            pass
        else:
            raise RuntimeError("Sequence open over non existing file did not fail as expected")


class empty_file(unittest.TestCase):
    def test_1(self):
        file_path = get_tempfile_name('empty_file')

        pathlib.Path(file_path).touch()

        try:
            seq = ytp.sequence(file_path, readonly=True)
        except RuntimeError:
            pass
        else:
            raise RuntimeError("Sequence open over empty file did not fail as expected")


if __name__ == '__main__':
    unittest.main()
