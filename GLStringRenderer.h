/*
 *  GLStringRenderer.h
 *  SpikeVisualization
 *
 *  Created by David Cox on 6/14/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */
#ifndef GL_STRING_RENDERER_H_
#define GL_STRING_RENDERER_H_

#include <OpenGL/gl.h>
#include <string>


class GLStringRenderer {

    public:

        virtual GLuint stringToTexture(std::string str, float font_size,float *width, float *height) = 0;
};


#endif
