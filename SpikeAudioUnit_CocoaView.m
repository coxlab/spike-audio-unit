/*
*	File:		Harvard UniversitySpikeAudioUnit_CocoaView.m
*	
*	Version:	1.0
* 
*	Created:	6/15/10
*	
*	Copyright:  Copyright © 2010 Harvard University, All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#import "SpikeAudioUnit_CocoaView.h"

enum {
	kThresholdParam =0,
	kNumberOfParameters=1
};

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____
void ParameterListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue) {
	SpikeAudioUnit_CocoaView *SELF = (SpikeAudioUnit_CocoaView *)inRefCon;
    
    [SELF _parameterListener:inObject parameter:inParameter value:inValue];
}

NSString *SpikeAudioUnit_GestureSliderMouseDownNotification = @"CAGestureSliderMouseDownNotification";
NSString *SpikeAudioUnit_GestureSliderMouseUpNotification = @"CAGestureSliderMouseUpNotification";

@implementation SpikeAudioUnit_GestureSlider

/*	We create our own custom subclass of NSSlider so we can do begin/end gesture notification
	We cannot override mouseUp: because it will never be called. Instead we do a clever trick in mouseDown to send mouseUp notifications */
- (void)mouseDown:(NSEvent *)inEvent {
	[[NSNotificationCenter defaultCenter] postNotificationName: SpikeAudioUnit_GestureSliderMouseDownNotification object: self];
	
	[super mouseDown: inEvent];	// this call does not return until mouse tracking is complete
								// once tracking is done, we know the mouse has been released, so we can send our mouseup notification

	[[NSNotificationCenter defaultCenter] postNotificationName: SpikeAudioUnit_GestureSliderMouseUpNotification object: self];
}
	
@end

@implementation SpikeAudioUnit_CocoaView
#pragma mark ____ (INIT /) DEALLOC ____
- (void)dealloc {
    [self _removeListeners];
    [super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU {
	// remove previous listeners
	if (mAU) [self _removeListeners];
	mAU = inAU;
    
   	mParameter[0].mAudioUnit = inAU;
	mParameter[0].mParameterID = kThresholdParam;
	mParameter[0].mScope = kAudioUnitScope_Global;
	mParameter[0].mElement = 0;	

	// add new listeners
	[self _addListeners];
	
	// initial setup
	[self _synchronizeUIWithParameterValues];
    
    // setup the timer
    [self setTimer: [NSTimer scheduledTimerWithTimeInterval: (1.0/30.0)
                                                     target: self
                                                   selector: @selector(updateSpikes:)
                                                   userInfo: nil
                                                    repeats: YES]];	
    
}


- (void) removeFromSuperview
{
	[capture_timer invalidate];
	//[[NSNotificationCenter defaultCenter] removeObserver: self];
    
	[super removeFromSuperview];
}


- (void) updateSpikes: (NSTimer*) t
{	
    
	UInt32 size = sizeof(TriggeredSpikes);
    TriggeredSpikes triggered_spikes;
    
	ComponentResult result = AudioUnitGetProperty(mAU,
                                  kAudioUnitProperty_TriggeredSpikes,
                                  kAudioUnitScope_Global,
                                  0,
                                  &triggered_spikes,
                                  &size);	
	
	if (result == noErr){
        if(triggered_spikes.n_spikes){
            //NSLog(@"got n triggers: %d", triggered_spikes.n_spikes);
            AUSpikeContainer *spike_waveform = triggered_spikes.spike_containers[0];
            float *buffer = spike_waveform->buffer;
            //NSLog(@"%f %f %f %f %f %f", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            shared_ptr<GLSpikeWave> wave(new GLSpikeWave(PRE_TRIGGER+POST_TRIGGER, -PRE_TRIGGER/44100., 1.0/44100., buffer));
            [self pushData:wave];
        }
    }
    
        
}


- (NSTimer *) timer {
	return [[capture_timer retain] autorelease];
}

- (void) setTimer: (NSTimer *) value {
	if ( capture_timer != value ) {
		[capture_timer release];
		capture_timer = [value retain];
	}
}


- (void) setTriggerThreshold:(Float32)value {
    NSLog(@"setting threshold...");
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[0], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()");

    [super setTriggerThreshold:value];
}

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaParam1Changed:(id)sender {
    float floatValue = [sender floatValue];
	NSAssert(	AUParameterSet(mParameterListener, sender, &mParameter[0], (Float32)floatValue, 0) == noErr,
                @"[SpikeAudioUnit_CocoaView iaParam1Changed:] AUParameterSet()");
    if (sender == uiParam1Slider) {
        [uiParam1TextField setFloatValue:floatValue];
    } else {
        [uiParam1Slider setFloatValue:floatValue];
    }
}

#pragma mark ____ NOTIFICATIONS ____

- (void)enterAdjustMode:(int)mode {

    NSLog(@"enter adjust");
    if(mode == SP_TRIGGER_THRESHOLD_SELECT){
        AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
		
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
        // pass that as the first argument to AUEventListenerNotify instead of NULL 
    }
}

- (void)exitAdjustMode:(int)mode {
    
    NSLog(@"exit adjust");
    if(mode == SP_TRIGGER_THRESHOLD_SELECT){
        AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
		
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
        // pass that as the first argument to AUEventListenerNotify instead of NULL 
    }
}


// This routine is called when the user has clicked on the slider. We need to send a begin parameter change gesture to alert hosts that the parameter may be changing value
-(void) handleMouseDown: (NSNotification *) aNotification {
	if ([aNotification object] == uiParam1Slider) {
		AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
		
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
														// pass that as the first argument to AUEventListenerNotify instead of NULL 
	}
}

-(void) handleMouseUp: (NSNotification *) aNotification {
	if ([aNotification object] == uiParam1Slider) {
		AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
	
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
														// pass that as the first argument to AUEventListenerNotify instead of NULL 
	}
}


#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)_addListeners {
	NSAssert (	AUListenerCreate(	ParameterListenerDispatcher, self, 
                                    CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.100, // 100 ms
                                    &mParameterListener	) == noErr,
                @"[SpikeAudioUnit_CocoaView _addListeners] AUListenerCreate()");
	
    int i;
    for (i = 0; i < kNumberOfParameters; ++i) {
        mParameter[i].mAudioUnit = mAU;
        NSAssert (	AUListenerAddParameter (mParameterListener, NULL, &mParameter[i]) == noErr,
                    @"[SpikeAudioUnit_CocoaView _addListeners] AUListenerAddParameter()");
    }
    
   	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(handleMouseDown:) name:SpikeAudioUnit_GestureSliderMouseDownNotification object: uiParam1Slider];
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(handleMouseUp:) name:SpikeAudioUnit_GestureSliderMouseUpNotification object: uiParam1Slider];
}

- (void)_removeListeners {

 
    
    int i;
    for (i = 0; i < kNumberOfParameters; ++i) {
        NSAssert (	AUListenerRemoveParameter(mParameterListener, NULL, &mParameter[i]) == noErr,
                    @"[SpikeAudioUnit_CocoaView _removeListeners] AUListenerRemoveParameter()");
    }
    
	NSAssert (	AUListenerDispose(mParameterListener) == noErr,
                @"[SpikeAudioUnit_CocoaView _removeListeners] AUListenerDispose()");
                
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void)_synchronizeUIWithParameterValues {
	Float32 value;
    int i;
    
    for (i = 0; i < kNumberOfParameters; ++i) {
        // only has global parameters
        NSAssert (	AudioUnitGetParameter(mAU, mParameter[i].mParameterID, kAudioUnitScope_Global, 0, &value) == noErr,
                    @"[SpikeAudioUnit_CocoaView synchronizeUIWithParameterValues] (x.1)");
        NSAssert (	AUParameterSet (mParameterListener, self, &mParameter[i], value, 0) == noErr,
                    @"[SpikeAudioUnit_CocoaView synchronizeUIWithParameterValues] (x.2)");
        NSAssert (	AUParameterListenerNotify (mParameterListener, self, &mParameter[i]) == noErr,
                    @"[SpikeAudioUnit_CocoaView synchronizeUIWithParameterValues] (x.3)");
    }
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue {
    //inObject ignored in this case.
    
	switch (inParameter->mParameterID) {
		case kThresholdParam:
            [self setTriggerThreshold:inValue];
            break;
	}
}


@end
