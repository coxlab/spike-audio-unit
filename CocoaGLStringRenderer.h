/*
 *  CocoaGLStringRenderer.h
 *  SpikeVisualization
 *
 *  Created by David Cox on 6/14/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef COCOA_GL_STRING_RENDERER_H_
#define COCOA_GL_STRING_RENDERER_H_

#include "GLStringRenderer.h"
#include <OpenGL/gl.h>


class CocoaGLStringRenderer : public GLStringRenderer {
    
    public:
        
        GLuint stringToTexture(std::string str, float font_size,float *width, float *height);
};

#endif
