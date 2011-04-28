#!/usr/bin/env python

#from pylab import *

import zmq

from spike_wave_pb2 import SpikeWaveBuffer

class SpikeListener:
    def __init__(self, ipcPath, channelIndices, zmqContext=None):
        if zmqContext == None:
            zmqContext = zmq.Context()
        
        self.socket = zmqContext.socket(zmq.SUB)
        for i in channelIndices:
            self.socket.connect("%s/%i" % (ipcPath,i))
        self.socket.setsockopt(zmq.SUBSCRIBE,"")
        self._wb = SpikeWaveBuffer()
    
    def update(self):
        try:
            packet = self.socket.recv(zmq.NOBLOCK)
            self._wb.ParseFromString(packet)
            self.process_spike(self._wb)
        except:
            return
    
    def process_spike(self, wb):
        pass


if __name__ == '__main__':
    sl = SpikeListener("ipc:///tmp/spike_channels/", xrange(32))
    def process_spike(wb): # overload process_spike
        print wb.time_stamp, wb.channel_id
    sl.process_spike = process_spike
    
    while True:
        sl.update()