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
@package ytp_kafka.py
@author Andres Rangel
@date 29 Nov 2018
@brief File contains tool to publish YTP messages to kafka topic
"""

import msgpack
from ytp import transactions
import sys
from collections import deque
from kafka import KafkaProducer
from kafka.errors import KafkaError
from json import dumps
import os

timeout_ms = int(os.getenv('DROP_COPY_TIMEOUT', '5000'))
timeout = timeout_ms / 1000
producer = KafkaProducer(bootstrap_servers=os.environ['KAFKA_HOST'] + ":" + os.environ['KAFKA_PORT'],
                         value_serializer=lambda x: dumps(x).encode('utf-8'),
                         request_timeout_ms=timeout_ms,
                         linger_ms=0,
                         max_block_ms=timeout_ms)

topic = os.environ['KAFKA_TOPIC']
retries = int(os.getenv('DROP_COPY_ATTEMPTS', '5'))


def kafka_clbck(msg):
    global producer
    global topic
    global timeout
    global retries
    future = producer.send(topic, value=msg)
    for i in range(retries):
        try:
            record_metadata = future.get(timeout=timeout)
            print("Sent message:", msg)
            return
        except KafkaError as e:
            print("Retrying to send message:", msg)
            print("Error:", e)
    raise RuntimeError("Unable to send message: ", msg)


def __preproc_msg(message):
    if isinstance(message, list):
        return [__preproc_msg(x) for x in message]
    return message.decode('utf-8') if isinstance(message, bytes) else message


def preproc_msg(publisher, channel, time, message):
    procmsg = __preproc_msg(message)
    realmsg = {"pub": publisher, "channel": channel, "time": time, "msg": procmsg}
    print(realmsg)
    return realmsg


def proc_txs(data_file):

    t = transactions(data_file)

    for msg in t:
        if msg:
            realmsg = preproc_msg(msg[0], msg[1], msg[2], msgpack.unpackb(msg[3]))
            print("sending message: ", realmsg)
            kafka_clbck(realmsg)


def main():
    if len(sys.argv) != 2:
        print("YTP to Kafka utility.")
        print("Usage: python3", sys.argv[0], "file_path")
        return
    proc_txs(sys.argv[1])


if __name__ == "__main__":
    main()
