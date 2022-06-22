#!/usr/bin/env python3
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
@package kafka_yamal.py
@author Andres Rangel
@date 29 Nov 2018
@brief File contains tool to publish kafka messages to yamal file
"""

from kafka import KafkaConsumer
from json import loads
import os
import sys
from transport_writer import TransportWriter

consumer = KafkaConsumer(os.environ['KAFKA_TOPIC'], auto_offset_reset='latest',
                         bootstrap_servers=os.environ['KAFKA_HOST'] + ":" + os.environ['KAFKA_PORT'],
                         value_deserializer=lambda m: loads(m))


def __preproc_msg(message):
    if isinstance(message, list):
        return [__preproc_msg(x) for x in message]
    return message.encode('utf-8') if isinstance(message, str) else message


def preproc_msg(msg):
    return {'pub': msg['pub'], 'msg': __preproc_msg(msg['msg'])}


def pub_txs(id_file, data_file, publisher_name):

    t = TransportWriter(id_file, data_file, publisher_name)

    while True:
        data = consumer.poll()
        if data:
            assert (len(data) == 1)
            msg = next(iter(data.values()))
            for m in msg:
                args = (preproc_msg(m.value)['msg'])[2:]
                print(args)
                t.write(*args)


def main():
    if (len(sys.argv) < 4 or len(sys.argv) > 4) or (len(sys.argv) == 2 and sys.argv[1]) == "--help":
        print("Kafka to Yamal.")
        print("Usage: python3", sys.argv[0], "id_file data_file pub_name")
        return
    pub_txs(sys.argv[1], sys.argv[2], sys.argv[3])


if __name__ == "__main__":
    main()
