#!/usr/bin/env python

import random, sys, time

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *

class GLRaster:
    def __init__(self, NRows, startingX):
        self.NRows = NRows
        self.newEvents = []
        self.rowHeight = 2. / NRows # gl units
        self.xScale = 0.1 # gl units per update tick
        self.startingX = startingX
        self.cursorX = startingX
        self.cursorXShift_gl = 0.2
        self.left = -1.
        self.top = 1.
        self.width_gl = 2.
        self.height_gl = self.rowHeight * self.NRows
    
    def draw(self,newX=None):
        if newX == None:
            if len(self.newEvents) == 0:
                return # nothing to draw
            else:
                newX = self.newEvents[0][0]
                for e in self.newEvents:
                    newX = max(newX,e[0])
        newX += self.cursorXShift_gl # shift newX forward a bit so that we don't draw over spikes
        newX_gl = (newX - self.startingX) * self.xScale + self.left
        oldX_gl = (self.cursorX - self.startingX) * self.xScale + self.left
        # draw over old cursor
        glColor(0.,0.,0.,1.)
        glBegin(GL_LINES)
        glVertex(oldX_gl,self.top)
        glVertex(oldX_gl,self.top-self.height_gl)
        glEnd()

        # blank out area between old and new cursor
        glColor(0.,0.,0.,1.)
        glBegin(GL_QUADS)
        glVertex2f(oldX_gl,self.top)
        glVertex2f(newX_gl,self.top)
        glVertex2f(newX_gl,self.top-self.height_gl)
        glVertex2f(oldX_gl,self.top-self.height_gl)
        glEnd()
        
        # draw new events
        for e in self.newEvents:
            t, r = e
            x_gl = (t - self.startingX) * self.xScale + self.left
            top_gl = self.top - r * self.rowHeight
            glColor(1.,0.,0.,1.)
            glBegin(GL_LINES)
            glVertex(x_gl,top_gl)
            glVertex(x_gl,top_gl-self.rowHeight)
            glEnd()
        self.newEvents = []
        
        # reset cursor if beyond the right edge
        if newX_gl > (self.left + self.width_gl):
            self.startingX = newX
            newX_gl = (newX - self.startingX) * self.xScale + self.left
        
        # draw new cursor
        glColor(1.,1.,1.,1.)
        glBegin(GL_LINES)
        glVertex(newX_gl,self.top)
        glVertex(newX_gl,self.top-self.height_gl)
        glEnd()
        
        self.cursorX = newX
        # # test QUAD
        # glBegin(GL_QUADS)
        # glColor(1.,0.,0.,1.)
        # glVertex2f(0., 1.)
        # glColor(0.,1.,0.,1.)
        # glVertex2f(1., 1.)
        # glColor(0.,0.,1.,1.)
        # glVertex2f(1., 0.)
        # glVertex2f(0., 0.)
        # glEnd()
    
    def reset(self, newX):
        glClear(GL_COLOR_BUFFER_BIT)
        self.startingX = newX
    
    def add_event(self, time, row):
        self.newEvents.append((time,row))

if __name__ == '__main__':
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
    glutInitWindowSize(1024,128)
    glutCreateWindow("Raster Test")
    glClearColor(0., 0., 0., 1.)
    
    global raster
    NRows = 32
    raster = GLRaster(NRows,time.time())
    
    def draw():
        global raster
        raster.draw()#time.time()) # passing in new time
        glutSwapBuffers()
    glutDisplayFunc(draw)
    
    global prevT
    prevT = time.time()
    
    global acts
    acts = [random.random() for i in xrange(NRows)]
    
    def idle():
        global prevT, raster
        dt = time.time() - prevT
        
        if dt > 0.03:
            # add random events
            global acts
            
            for i in xrange(len(acts)):
                acts[i] += dt * 0.5 # freq of events
                acts[i] += 0.001 * random.random() # noise
                if acts[i] > 1:
                    acts[i] -= 1
                    raster.add_event(prevT - acts[i],i)
            
            # update display
            prevT = time.time()
            glutPostRedisplay()
    glutIdleFunc(idle)
    
    glClear(GL_COLOR_BUFFER_BIT)
    glutMainLoop()