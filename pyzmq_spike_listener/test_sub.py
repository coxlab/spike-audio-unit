#!/usr/bin/env python

from pylab import *

import zmq

from spike_wave_pb2 import SpikeWaveBuffer

ipcPath = "ipc:///tmp/spike_channels/0"

context = zmq.Context()

socket = context.socket(zmq.SUB)
socket.connect(ipcPath)
socket.setsockopt(zmq.SUBSCRIBE,"")

figure()
ion()

count = 0

while True:
    wb = SpikeWaveBuffer()
    wb.ParseFromString(socket.recv())
    print wb
    plot(wb.wave_sample)
    draw()