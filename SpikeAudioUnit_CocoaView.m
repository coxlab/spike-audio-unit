/*
*	File:		Harvard UniversitySpikeAudioUnit_CocoaView.m
*	
*	Version:	1.0
* 
*	Created:	6/15/10
*	
*	Copyright:  Copyright � 2010 Harvard University, All Rights Reserved
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
#include "spike_wave.pb.h"
#include "ctl_message.pb.h"
#include <string>
#include <sstream>

using namespace std;

#include "SharedTypes.h"


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
    
    message_socket = NULL;
    
	// remove previous listeners
	if (mAU) [self _removeListeners];
	mAU = inAU;
    
    channel_id = 0;
    
   	for(int i = 0; i < kNumberOfParameters; i++){
        mParameter[i].mAudioUnit = inAU;
        mParameter[i].mScope = kAudioUnitScope_Global;
        mParameter[i].mElement = 0;
        mParameter[i].mParameterID = i; // ugly
    }
    
	// add new listeners
	[self _addListeners];
	
	// initial setup
	[self _synchronizeUIWithParameterValues];
    

    message_context = zmq_init(1);

    // setup the timer 
    [self setTimer: [NSTimer scheduledTimerWithTimeInterval: (1.0/20.0)
                                                     target: self
                                                   selector: @selector(updateSpikes:)
                                                   userInfo: nil
                                                    repeats: YES]];	
    
}

- (void) viewWillMoveToSuperview:(NSView *)view{
    
    if(view != Nil && capture_timer != Nil){
    
        // setup the timer
        [self setTimer: [NSTimer scheduledTimerWithTimeInterval: (1.0/20.0)
                                                     target: self
                                                   selector: @selector(updateSpikes:)
                                                   userInfo: nil
                                                    repeats: YES]];	
    }
    
}


- (void) removeFromSuperview
{
	[capture_timer invalidate];
    capture_timer = Nil;
    
	[super removeFromSuperview];
}


- (void) updateSpikes: (NSTimer*) t
{	
    
    int rc;
	zmq_msg_t msg;
    
    rc = zmq_msg_init (&msg);
    assert (rc == 0);
    
    // Receive a message 
    rc = zmq_recv (message_socket, &msg, ZMQ_NOBLOCK);
    
    int count = 0;
    
    while(rc == 0){

        string data((const char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
        SpikeWaveBuffer wave;
        wave.ParseFromString(data);
        
        
        Float32 data_buffer[PRE_TRIGGER+POST_TRIGGER];
        
        for(int i = 0; i < wave.wave_sample_size(); i++){
        
            data_buffer[i] = wave.wave_sample(i);
        }
        
        shared_ptr<GLSpikeWave> gl_wave(new GLSpikeWave(PRE_TRIGGER+POST_TRIGGER, -PRE_TRIGGER/44100., 1.0/44100., data_buffer));
        
        [self pushData:gl_wave];
        zmq_msg_close(&msg);
        rc = zmq_msg_init (&msg);
        assert (rc == 0);
        
        count++;
        #define WAVE_HWM    200
        if(count > WAVE_HWM){
            break;
        }
        
        rc = zmq_recv (message_socket, &msg, ZMQ_NOBLOCK);
    }

    [self setNeedsDisplay: YES];
        
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


- (void) autothresholdUp {
    
    int current = renderer->getAutoThresholdState();
    
    if(current == AUTO_THRESHOLD_HIGH){
        NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdHighParam], false, 0) == noErr,
                 @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()");
                 
        renderer->setAutoThresholdState(AUTO_THRESHOLD_OFF);

        return;
    }
    
    
    if(current == AUTO_THRESHOLD_LOW){
        NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdLowParam], false, 0) == noErr,
                 @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()");
    }

    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdHighParam], true, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()"); 
    renderer->setAutoThresholdState(AUTO_THRESHOLD_HIGH);
    
}


- (void) autothresholdDown {
    
    int current = renderer->getAutoThresholdState();
    
    if(current == AUTO_THRESHOLD_LOW){
        NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdLowParam], false, 0) == noErr,
                 @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()"); 
        renderer->setAutoThresholdState(AUTO_THRESHOLD_OFF);
        return;
    }
    
    
    if(current == AUTO_THRESHOLD_HIGH){
        NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdHighParam], false, 0) == noErr,
                 @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()");
    }
    
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kAutoThresholdLowParam], true, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()"); 
    renderer->setAutoThresholdState(AUTO_THRESHOLD_LOW);
}



- (void) setTriggerThresholdSilent:(Float32)value {
    [super setTriggerThreshold:value];
}


- (void) setTriggerThreshold:(Float32)value {
    //NSLog(@"setting threshold...");
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kThresholdParam], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTriggerThresholdChanged:] AUParameterSet()");
    [super setTriggerThreshold:value];
}

- (void) setTimeRangeMin:(Float32)value {
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kMinTimeViewParam], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTimeRangeMinChanged:] AUParameterSet()");
    
    [super setTimeRangeMin:value];
}

- (void) setTimeRangeMax:(Float32)value {
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kMaxTimeViewParam], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaTimeRangeMaxChanged:] AUParameterSet()");
    
    [super setTimeRangeMax:value];
}

- (void) setAmplitudeRangeMin:(Float32)value {
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kMinAmplitudeViewParam], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaAmplitudeRangeMinChanged:] AUParameterSet()");
    
    [super setAmplitudeRangeMin:value];
}

- (void) setAmplitudeRangeMax:(Float32)value {
    NSAssert(	AUParameterSet(mParameterListener, self, &mParameter[kMaxAmplitudeViewParam], (Float32)value, 0) == noErr,
             @"[SpikeAudioUnit_CocoaView iaAmplitudeRangeMaxChanged:] AUParameterSet()");
    
    [super setAmplitudeRangeMax:value];
}



#pragma mark ____ NOTIFICATIONS ____

- (void)enterAdjustMode:(int)mode {

    AudioUnitEvent event;
    event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;

    //NSLog(@"enter adjust");
    if(mode == SP_TRIGGER_THRESHOLD_SELECT){
        
		event.mArgument.mParameter = mParameter[kThresholdParam];

    } else if(mode == SP_AMPLITUDE_MIN_SELECT){
		event.mArgument.mParameter = mParameter[kMinAmplitudeViewParam];
    } else if(mode == SP_AMPLITUDE_MAX_SELECT){
		event.mArgument.mParameter = mParameter[kMaxAmplitudeViewParam];
    } else if(mode == SP_TIME_MIN_SELECT){
		event.mArgument.mParameter = mParameter[kMinTimeViewParam];
    } else {
		event.mArgument.mParameter = mParameter[kMaxTimeViewParam];
    }
    
    AUEventListenerNotify (NULL, self, &event);
}

- (void)exitAdjustMode:(int)mode {
    
    //NSLog(@"exit adjust");
    
    AudioUnitEvent event;
    event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
    
   
    if(mode == SP_TRIGGER_THRESHOLD_SELECT){
		event.mArgument.mParameter = mParameter[kThresholdParam];
    } else if(mode == SP_AMPLITUDE_MIN_SELECT){
		event.mArgument.mParameter = mParameter[kMinAmplitudeViewParam];
    } else if(mode == SP_AMPLITUDE_MAX_SELECT){
		event.mArgument.mParameter = mParameter[kMaxAmplitudeViewParam];
    } else if(mode == SP_TIME_MIN_SELECT){
		event.mArgument.mParameter = mParameter[kMinTimeViewParam];
    } else if(mode == SP_AUTOTHRESH_UP_SELECT){
        [self autothresholdUp];
        event.mArgument.mParameter = mParameter[kAutoThresholdHighParam];
    } else if(mode == SP_AUTOTHRESH_DOWN_SELECT){
        [self autothresholdDown];
        event.mArgument.mParameter = mParameter[kAutoThresholdLowParam];
    } else {
		event.mArgument.mParameter = mParameter[kMaxTimeViewParam];
    }		
	
    AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
    
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
    
    //NSLog(@"notified of change: %d", *inParameter);
    
	switch (inParameter->mParameterID) {
		case kThresholdParam:
            [self setTriggerThresholdSilent:inValue];
            break;
        case kMinAmplitudeViewParam:
            renderer->setAmplitudeRangeMin(inValue);
            break;
        case kMaxAmplitudeViewParam:
            renderer->setAmplitudeRangeMax(inValue);
            break;
        case kMinTimeViewParam:
            renderer->setTimeRangeMin(inValue);
            break;
        case kMaxTimeViewParam:
            renderer->setTimeRangeMax(inValue);
            break;
        case kChannelIDParam:
            channel_id = inValue;
            [self connectToChannel:(int)inValue];
                       
            break;
        case kUnitsPerVoltParam:
            renderer->setUnitsPerVolt(inValue);
            break;
        case kGainParam:
            renderer->setAmplifierGain(inValue);
            break;
        case kAutoThresholdLowParam:
            if(inValue) renderer->setAutoThresholdState(AUTO_THRESHOLD_LOW);
            break;
        case kAutoThresholdHighParam:
            if(inValue) renderer->setAutoThresholdState(AUTO_THRESHOLD_HIGH);
            break;
	}
}

- (void)connectToChannel:(int)channel {
    
    if(message_socket != NULL){
        zmq_close(message_socket);
    }
    message_socket = zmq_socket(message_context, ZMQ_SUB);
    
    zmq_setsockopt(message_socket, ZMQ_SUBSCRIBE, "", 0);
    
    
    std::cerr << "Client changing to channel " << channel << std::endl;
    
    // construct the url
    ostringstream filename_stream, url_stream;


    #define HOST_ADDRESS "tcp://127.0.0.1" 
    #define SPIKE_BASE_PORT 8000

    url_stream << HOST_ADDRESS << ":" << SPIKE_BASE_PORT + channel;
    
    
    int rc = zmq_connect(message_socket, url_stream.str().c_str());
    
    if(rc != 0){
        std::cerr << "ZMQ (client): " << zmq_strerror(zmq_errno()) << std::endl;
    } else {
        std::cerr << "ZMQ client connected successfully to " << url_stream.str() << std::endl;
    }
    
}


@end
