/*
*	File:		SpikeAudioUnit.h
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

#include "AUEffectBase.h"
#include "SpikeAudioUnitVersion.h"
#include "CARingBuffer.h"
#include "CAThreadSafeList.h"
#include "CABufferList.h"
#include <boost/shared_ptr.hpp>
#include "SharedTypes.h"
#include "MIDIEndpoint.h"
#include "LocklessQueue.h"

using namespace boost;

#define DEFAULT_BUFFER_SIZE     44100


#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif


#ifndef __SpikeAudioUnit_h__
#define __SpikeAudioUnit_h__


#pragma mark ____SpikeAudioUnit Parameters

// parameters
static const float kDefaultValue_ThresholdParam = -0.000035;
static const float kDefaultValue_MinAmplitudeViewParam = -0.000100;
static const float kDefaultValue_MaxAmplitudeViewParam = 0.000100;
static const float kDefaultValue_MinTimeViewParam = -0.00125;
static const float kDefaultValue_MaxTimeViewParam = 0.00125;

static CFStringRef kThresholdParamName = CFSTR("Trigger Threshold");
static CFStringRef kMinAmplitudeViewParamName = CFSTR("Min Amplitude");
static CFStringRef kMaxAmplitudeViewParamName = CFSTR("Max Amplitude");
static CFStringRef kMinTimeViewParamName = CFSTR("Min Time");
static CFStringRef kMaxTimeViewParamName = CFSTR("Max Time");

enum {
    kThresholdParam =0,
    kMinAmplitudeViewParam = 1,
    kMaxAmplitudeViewParam = 2,
    kMinTimeViewParam = 3,
    kMaxTimeViewParam = 4,
    kNumberOfParameters=5
};



typedef LocklessQueue<AUSpikeContainer> SpikeContainerQueue;

#pragma mark ____SpikeAudioUnit
class SpikeAudioUnit : public AUEffectBase
{
public:
	SpikeAudioUnit(AudioUnit component);
#if AU_DEBUG_DISPATCHER
	virtual ~SpikeAudioUnit () { delete mDebugDispatcher; }
#endif
	
	virtual AUKernelBase *		NewKernel() { return new SpikeAudioUnitKernel(this); }
	
	virtual	OSStatus			GetParameterValueStrings(AudioUnitScope			inScope,
														 AudioUnitParameterID		inParameterID,
														 CFArrayRef *			outStrings);
    
	virtual	OSStatus			GetParameterInfo(AudioUnitScope			inScope,
												 AudioUnitParameterID	inParameterID,
												 AudioUnitParameterInfo	&outParameterInfo);
    
	virtual OSStatus			GetPropertyInfo(AudioUnitPropertyID		inID,
												AudioUnitScope			inScope,
												AudioUnitElement		inElement,
												UInt32 &			outDataSize,
												Boolean	&			outWritable );
	
	virtual OSStatus			GetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 		inElement,
											void *			outData);
	
 	virtual	bool				SupportsTail () { return false; }
	
	/*! @method Version */
	virtual OSStatus		Version() { return kSpikeAudioUnitVersion; }
	
    


    class SpikeAudioUnitKernel : public AUKernelBase		// most real work happens here
	{  
        public:
            SpikeAudioUnitKernel(AUEffectBase *inAudioUnit ): AUKernelBase(inAudioUnit){ 
                
                midi_endpoint = shared_ptr<MIDIEndpoint>(new MIDIEndpoint("midi_spikes", "default_port", "spike_source"));
                
                capture_buffer.Allocate(1, sizeof(Float32), DEFAULT_BUFFER_SIZE);//2048);
                
                CAStreamBasicDescription	bufClientDesc;		
                bufClientDesc.SetCanonical(1, false);
                bufClientDesc.mSampleRate = 44100;
                                
                
                //temp_buffer.mNumberChannels = 1;
//                temp_buffer.mDataByteSize = sizeof(Float32);
//                temp_buffer.mData = (void *)malloc(sizeof(Float32));
//                temp_buffer_list.mNumberBuffers = 1;
//                temp_buffer_list.mBuffers[0] = temp_buffer;
                input_buffer_list = CABufferList::New("input intermediate buffer", bufClientDesc);
                input_buffer_list->AllocateBuffers(10 * (PRE_TRIGGER + POST_TRIGGER) & sizeof(Float32));
                
                pending_trigger = -1; // armed
                refractory_count = 0;
                
                frame_number = 0;
                n_triggers = 0;
                
                last_sample = 0.0;
                
                capture_buffer_list = CABufferList::New("capture buffer", bufClientDesc );
                capture_buffer_list->AllocateBuffers(DEFAULT_BUFFER_SIZE * sizeof(Float32));//10 * (PRE_TRIGGER + POST_TRIGGER) * sizeof(Float32));
            
                
            }
		
            // *Required* overides for the process method for this effect
            // processes one channel of interleaved samples
            virtual void 		Process(	const Float32 	*inSourceP,
                                            Float32		 	*inDestP,
                                            UInt32 			inFramesToProcess,
                                            UInt32			inNumChannels,
                                            bool			&ioSilence);
            
            virtual void		Reset();
		
            
            void getTriggeredSpikes(TriggeredSpikes *spikes); 
                        
        protected:
        
            int n_triggers;
            
            Float32 last_sample;
            
            SInt64 frame_number;
            
            int pending_trigger; // a count-down until a buffer is full and ready to dispatch
                                 // -1: no pending trigger
                                 // +n: n samples until trigger is tripped
                                 // 0: trigger now! (copy from ring buffer to spike queue)
            
            
            int refractory_count;
            
            
            SpikeContainerQueue spike_display_queue;  // a thread-safe place to drop triggered waveforms
            
            CARingBuffer capture_buffer;
            
            AudioBufferList temp_buffer_list;
            AudioBuffer temp_buffer;
            
            CABufferList *input_buffer_list;
            CABufferList *capture_buffer_list;
            
            AUSpikeContainer getFreshSpikeContainer();  
            
            shared_ptr<MIDIEndpoint> midi_endpoint; 
               
	};
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif