//
//  SpikeWaveView.h
//  spike_visualization
//
//  Created by David Cox on 6/14/10.
//  Copyright 2010 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpikeRenderer.h"
#import "SpikeWave.h"

using namespace spike_visualization;

@interface SpikeWaveView : NSOpenGLView {

    SpikeRenderer *renderer;
    
    NSTimer *junk_data_timer;
    
    int adjust_mode;

    GLfloat old_adjust_value;

}

- (void)awakeFromNib;
- (void)drawRect:(NSRect)dirtyRect;
- (void)reshape;

- (void) pushData:(shared_ptr<GLSpikeWave>) wave;
- (void) pushJunkData;

// to be overidden
- (void) enterAdjustMode:(int)mode;
- (void) exitAdjustMode:(int)mode;

- (void) setTriggerThreshold:(Float32)value;
- (void) setTimeRangeMin:(Float32)value;
- (void) setTimeRangeMax:(Float32)value;
- (void) setAmplitudeRangeMin:(Float32)value;
- (void) setAmplitudeRangeMax:(Float32)value;
@end
