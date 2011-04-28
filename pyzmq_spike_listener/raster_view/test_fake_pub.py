#!/usr/bin/env python

import os, random, time
from numpy.random import poisson

import zmq

from spike_wave_pb2 import SpikeWaveBuffer

class Spiker:
    def __init__(self, hz):
        self.hz = hz
    def update(self,dt):
        """
        dt : seconds
        returns the number of spikes within the update interval
        """
        if dt > 0:
            return poisson(self.hz*dt)
        else:
            return 0

global zmqContext
zmqContext = None

class SocketSpiker (Spiker):
    def __init__(self, hz, ipcPath, id):
        Spiker.__init__(self, hz)
        global zmqContext
        if zmqContext == None:
            zmqContext = zmq.Context()
        self.id = id
        self.socket = zmqContext.socket(zmq.PUB)
        self.socket.bind(ipcPath)
    
    def update(self, dt):
        nSpikes = Spiker.update(self, dt)
        if nSpikes:
            t = time.time() * 44100 # convert time to microseconds
            for i in xrange(nSpikes):
                wb = SpikeWaveBuffer()
                wb.channel_id = self.id
                wb.time_stamp = int(t)
                for i in xrange(100):
                    wb.wave_sample.append(random.random())
                self.socket.send(wb.SerializeToString())
        return nSpikes

ipcPath = "ipc:///tmp/spike_channels/"
NChannels = 32
Hz = 0.1

if not os.path.exists('/tmp/spike_channels/'):
    os.makedirs('/tmp/spike_channels/')

spikers = []
for i in xrange(NChannels):
    spikers.append(SocketSpiker(Hz,'%s/%i' % (ipcPath,i),i))

prevTime = time.time()
while True:
    currTime = time.time()
    dt = currTime - prevTime
    [spiker.update(dt) for spiker in spikers]
    prevTime = currTime
    time.sleep(0.00001)
