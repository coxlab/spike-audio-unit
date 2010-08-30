/*
 *  CocoaGLStringRenderer.cpp
 *  SpikeVisualization
 *
 *  Created by David Cox on 6/14/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include "CocoaGLStringRenderer.h"
#import "CocoaGLString.h"
using namespace std;

GLuint CocoaGLStringRenderer::stringToTexture(string str, float font_size, float *width, float *height){

    // build up a string to a texture
    NSString *cocoa_str = [NSString stringWithCString:str.c_str() encoding:NSASCIIStringEncoding];
    NSFont *font = [NSFont fontWithName:@"Lucida Grande" size:2.0];//font_size];
    
    
    NSDictionary *attr = [NSDictionary dictionaryWithObject:font  forKey:@"NSFontAttributeName"];
    NSAttributedString *att_str = [[NSAttributedString alloc] initWithString:cocoa_str attributes:attr];
    
    
    CocoaGLString *cocoa_gl_str = [[CocoaGLString alloc] initWithAttributedString:att_str 
                                                                    withTextColor:[NSColor whiteColor]
                                                                    withBoxColor:[NSColor grayColor]
                                                                    withBorderColor:[NSColor blackColor]];
    [cocoa_gl_str useDynamicFrame];
    
    [cocoa_gl_str genTexture];
    GLuint tex = [cocoa_gl_str texName];
    
    NSSize size = [cocoa_gl_str texSize];
    *width = size.width;
    *height = size.height;
    
    [cocoa_gl_str releaseTexture]; // let me deal with it
    [cocoa_gl_str release];
    [att_str release];
    
    return tex;
}