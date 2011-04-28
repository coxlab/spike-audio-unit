#!/usr/bin/env python

import sys, time

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *

from spike_listener import SpikeListener
from glraster import GLRaster

if __name__ == '__main__':
    # setup opengl
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
    glutInitWindowSize(1024,128)
    glutCreateWindow("Spike Raster")
    glClearColor(0., 0., 0., 1.)
    
    # setup raster view
    global raster
    NRows = 32
    raster = GLRaster(NRows,time.time())
    
    # setup spike listener
    global sl
    sl = SpikeListener("ipc:///tmp/spike_channels/", xrange(32))
    def process_spike(wb): # overload process_spike
        global raster
        raster.add_event(wb.time_stamp/44100., wb.channel_id)
    sl.process_spike = process_spike
    
    # this is a rather ugly way of getting the cursor to move when no events are being received
    global timeShift
    timeShift = 0.
    
    def draw():
        global raster, timeShift
        raster.draw(time.time()+timeShift) # passing in new time
        glutSwapBuffers()
    glutDisplayFunc(draw)
    
    global prevT
    prevT = time.time()
    
    def idle():
        global prevT, sl
        sl.update()
        
        dt = time.time() - prevT
        
        if dt > 0.03:
            prevT = time.time()
            glutPostRedisplay()
    
    glutIdleFunc(idle)
    
    # get initial raster time
    while not len(raster.newEvents):
        sl.update()
        time.sleep(0.01)
    timeShift = raster.newEvents[0][0] - time.time() + .5
    raster.reset(raster.newEvents[0][0])
    
    glClear(GL_COLOR_BUFFER_BIT)
    glutMainLoop()