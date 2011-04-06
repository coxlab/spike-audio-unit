#!/usr/bin/env python

import os, random, time

import zmq

from spike_wave_pb2 import SpikeWaveBuffer

ipcPath = "ipc:///tmp/spike_channels/0"

if not os.path.exists('/tmp/spike_channels/'):
    os.makedirs('/tmp/spike_channels/')

context = zmq.Context()

socket = context.socket(zmq.PUB)
socket.bind(ipcPath)

while True:
    wb = SpikeWaveBuffer()
    wb.channel_id = 0
    wb.time_stamp = int(time.time())
    for i in xrange(100):
        wb.wave_sample.append(random.random())
    socket.send(wb.SerializeToString())
    time.sleep(0.5)