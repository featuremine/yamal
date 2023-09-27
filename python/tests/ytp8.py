"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import unittest
from yamal import yamal, data, streams, stream
import typing
import os

def cleanup_file(path):
    try:
        os.remove(path)
    except BaseException:
        pass

class TestYamal8(unittest.TestCase):

    def test_closable(self):
        fname = "test_closable.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=True)
        self.assertIsInstance(y, yamal)
        dat = y.data()
        self.assertIsInstance(dat, data)
        self.assertTrue(dat.closable())
        self.assertFalse(dat.closed())
        dat.close()
        self.assertTrue(dat.closed())

    def test_unclosable(self):
        fname = "test_unclosable.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        dat = y.data()
        self.assertIsInstance(dat, data)
        self.assertFalse(dat.closable())
        self.assertFalse(dat.closed())
        self.assertRaises(RuntimeError, dat.close)

    def test_streams(self):
        fname = "test_streams.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertNotEqual(s.id, 0)
        self.assertRaises(RuntimeError, ss.announce, "peer1", "ch1", "invalid")

        ls, lsenc = ss.lookup("peer1", "ch1")
        self.assertEqual(s, ls)

        self.assertRaises(KeyError, ss.lookup, "peer1", "invalid")
        self.assertRaises(KeyError, ss.lookup, "invalid", "ch1")

        sseqn, speer, sch, sencoding = y.announcement(s)
        self.assertEqual(sseqn, 1)
        self.assertEqual(speer, "peer1")
        self.assertEqual(sch, "ch1")
        self.assertEqual(sencoding, "encoding1")
        self.assertEqual(s.seqno, 1)
        self.assertEqual(s.peer, "peer1")
        self.assertEqual(s.channel, "ch1")
        self.assertEqual(s.encoding, "encoding1")


    def test_iteration(self):
        fname = "test_iteration.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        dat = y.data()
        self.assertIsInstance(dat, data)

        messages = [
            b"msg1",
            b"msg2",
            b"msg3"
        ]

        i = 0
        for message in messages:
            s.write(i, message)
            i+=1

        # Forward:

        # For on iterator
        it = iter(dat)
        i = 0
        for seq, ts, strm, msg in it:
            self.assertEqual(seq, i + 1)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i+=1
        self.assertEqual(i, len(messages))

        message = b"msg4"
        s.write(3, message)
        messages += [message]

        for seq, ts, strm, msg in it:
            self.assertEqual(seq, i + 1)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i+=1
        self.assertEqual(i, len(messages))

        # For on data
        i = 0
        for seq, ts, strm, msg in dat:
            self.assertEqual(seq, i + 1)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i+=1
        self.assertEqual(i, len(messages))

        # Direct iteration
        it = iter(dat)
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 1)
        self.assertEqual(ts, 0)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[0])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 2)
        self.assertEqual(ts, 1)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[1])
        midoffset = int(it)
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 3)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 4)
        self.assertEqual(ts, 3)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[3])
        self.assertRaises(StopIteration, next, it)

        message = b"msg5"
        s.write(4, message)
        messages += [message]

        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 5)
        self.assertEqual(ts, 4)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[4])
        self.assertRaises(StopIteration, next, it)

        forwardit = dat.seek(midoffset)

        seq, ts, strm, msg = next(forwardit)
        self.assertEqual(seq, 3)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        seq, ts, strm, msg = next(forwardit)
        self.assertEqual(seq, 4)
        self.assertEqual(ts, 3)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[3])
        seq, ts, strm, msg = next(forwardit)
        self.assertEqual(seq, 5)
        self.assertEqual(ts, 4)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[4])
        self.assertRaises(StopIteration, next, forwardit)

        # Reverse:

        # For on iterator
        it = reversed(dat)
        i = len(messages)
        for seq, ts, strm, msg in it:
            self.assertEqual(seq, i)
            self.assertEqual(ts, i - 1)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i - 1])
            i-=1
        self.assertEqual(i, 0)

        # Direct iteration
        it = reversed(dat)
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 5)
        self.assertEqual(ts, 4)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[4])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 4)
        self.assertEqual(ts, 3)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[3])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 3)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 2)
        self.assertEqual(ts, 1)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[1])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 1)
        self.assertEqual(ts, 0)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[0])
        self.assertRaises(StopIteration, next, it)

        reverseit = reversed(dat.seek(midoffset))

        seq, ts, strm, msg = next(reverseit)
        self.assertEqual(seq, 3)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        seq, ts, strm, msg = next(reverseit)
        self.assertEqual(seq, 2)
        self.assertEqual(ts, 1)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[1])
        seq, ts, strm, msg = next(reverseit)
        self.assertEqual(seq, 1)
        self.assertEqual(ts, 0)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[0])
        self.assertRaises(StopIteration, next, reverseit)

    def test_serialization(self):
        fname = "test_serialization.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertEqual(str(s), "stream_t(id=48,seqno=1,peer=peer1,channel=ch1,encoding=encoding1)")
        self.assertEqual(repr(s), "stream_t(id=48,seqno=1,peer=peer1,channel=ch1,encoding=encoding1)")

    def test_hashing(self):
        fname = "test_hashing.ytp"
        cleanup_file(fname)
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertIsInstance(s, typing.Hashable)
        self.assertNotEqual(hash(s), 0)

if __name__ == '__main__':
    unittest.main()
