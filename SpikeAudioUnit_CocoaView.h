/*
*	File:		CocoaView.h
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

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <zmq.h>

#import "SharedTypes.h"
#import "SpikeWaveView.h"


/************************************************************************************************************/
/* NOTE: It is important to rename ALL ui classes when using the XCode Audio Unit with Cocoa View template	*/
/*		 Cocoa has a flat namespace, and if you use the default filenames, it is possible that you will		*/
/*		 get a namespace collision with classes from the cocoa view of a previously loaded audio unit.		*/
/*		 We recommend that you use a unique prefix that includes the manufacturer name and unit name on		*/
/*		 all objective-C source files. You may use an underscore in your name, but please refrain from		*/
/*		 starting your class name with an undescore as these names are reserved for Apple.					*/
/*  Example  : AppleDemoFilter_UIView AppleDemoFilter_ViewFactory											*/
/************************************************************************************************************/

@interface SpikeAudioUnit_GestureSlider : NSSlider {}
@end

@interface SpikeAudioUnit_CocoaView : SpikeWaveView
{
    // IB Members
    IBOutlet SpikeAudioUnit_GestureSlider * uiParam1Slider;
    IBOutlet NSTextField *			uiParam1TextField;
	
    // Other Members
    AudioUnit 				mAU;
	AudioUnitParameter		mParameter[6];
    AUParameterListenerRef	mParameterListener;
    
    NSTimer * capture_timer;
    
    void *message_context;
    void *message_socket;
    
    int channel_id;
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;


#pragma mark ____ TIMER FUNCTIONS ____
- (NSTimer *) timer;
- (void) setTimer: (NSTimer *) value;


#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaParam1Changed:(id)sender;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)_synchronizeUIWithParameterValues;
- (void)_addListeners;
- (void)_removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue;

@end
