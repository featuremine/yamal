"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import unittest
from yamal import yamal, data, streams, stream
import typing

class TestYamal8(unittest.TestCase):

    def test_closable(self):
        y = yamal(fname, closable=True)
        self.assertIsInstance(y, yamal)
        dat = y.data()
        self.assertIsInstance(dat, data)
        self.assertTrue(dat.closable())
        self.assertFalse(dat.closed())
        data.close()
        self.assertTrue(dat.closed())

    def test_unclosable(self):
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        dat = y.data()
        self.assertIsInstance(dat, data)
        self.assertFalse(dat.closable())
        self.assertFalse(dat.closed())
        self.assertRaises(data.close(), RuntimeError)

    def test_streams(self):
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertNotEqual(s.id(), 0)
        self.assertRaises(ss.announce("peer1", "ch1", "invalid"), RuntimeError)

        ls, lsenc = ss.lookup("peer1", "ch1")
        self.assertEqual(s, ls)

        self.assertIsNone(ss.lookup("peer1", "invalid"))
        self.assertIsNone(ss.lookup("invalid", "ch1"))

        sseqn, speer, sch, sencoding = y.announcement(s)
        self.assertEqual(sseqn, 1)
        self.assertEqual(speer, "peer1")
        self.assertEqual(sch, "ch1")
        self.assertEqual(sencoding, "encoding1")


    def test_iteration(self):
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
            self.assertEqual(seq, i)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i+=1

        # For on data
        i = 0
        for seq, ts, strm, msg in dat:
            self.assertEqual(seq, i)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i+=1

        # Direct iteration
        it = iter(dat)
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 0)
        self.assertEqual(ts, 0)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[0])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 1)
        self.assertEqual(ts, 1)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[1])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 2)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        self.assertRaises(next(it), StopIteration)

        # Reverse:

        # For on iterator
        it = reversed(dat)
        i = 2
        for seq, ts, strm, msg in it:
            self.assertEqual(seq, i)
            self.assertEqual(ts, i)
            self.assertEqual(strm, s)
            self.assertEqual(msg, messages[i])
            i-=1

        # Direct iteration
        it = reversed(dat)
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 2)
        self.assertEqual(ts, 2)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[2])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 1)
        self.assertEqual(ts, 1)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[1])
        seq, ts, strm, msg = next(it)
        self.assertEqual(seq, 0)
        self.assertEqual(ts, 0)
        self.assertEqual(strm, s)
        self.assertEqual(msg, messages[0])
        self.assertRaises(next(it), StopIteration)

    def test_serialization(self):
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertEqual(str(s), "48")
        self.assertEqual(repr(s), "48")

    def test_hashing(self):
        y = yamal(fname, closable=False)
        self.assertIsInstance(y, yamal)
        ss = y.streams()
        self.assertIsInstance(ss, streams)
        s = ss.announce("peer1", "ch1", "encoding1")
        self.assertIsInstance(s, stream)
        self.assertIsInstance(s, typing.Hashable)
        self.assertEqual(hash(s), 48)

if __name__ == '__main__':
    unittest.main()
