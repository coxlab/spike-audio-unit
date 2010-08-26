/*

TESTING
*	File:		SpikeAudioUnit.cpp
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

#include "SpikeAudioUnit.h"
#include <iostream>

#define EMIT_MIDI

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(SpikeAudioUnit)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::SpikeAudioUnit
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SpikeAudioUnit::SpikeAudioUnit(AudioUnit component)
	: AUEffectBase(component)
{
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParameters);
	SetParameter(kThresholdParam, kDefaultValue_ThresholdParam );
    SetParameter(kMinAmplitudeViewParam, kDefaultValue_MinAmplitudeViewParam );
    SetParameter(kMaxAmplitudeViewParam, kDefaultValue_MaxAmplitudeViewParam );
    SetParameter(kMinTimeViewParam, kDefaultValue_MinTimeViewParam );
    SetParameter(kMaxTimeViewParam, kDefaultValue_MaxTimeViewParam );
    
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
	
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SpikeAudioUnit::GetParameterValueStrings(AudioUnitScope		inScope,
                                                                AudioUnitParameterID	inParameterID,
                                                                CFArrayRef *		outStrings)
{
        
    return kAudioUnitErr_InvalidProperty;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SpikeAudioUnit::GetParameterInfo(AudioUnitScope		inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo )
{
	OSStatus result = noErr;

	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						|		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        switch(inParameterID)
        {
            case kThresholdParam:
                AUBase::FillInParameterName (outParameterInfo, kThresholdParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_ThresholdParam;
                break;
            
            case kMinAmplitudeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMinAmplitudeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MinAmplitudeViewParam;
                break;
                
            case kMaxAmplitudeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMaxAmplitudeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MaxAmplitudeViewParam;
                break;
                
            case kMinTimeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMinTimeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MinTimeViewParam;
                break;
            
            case kMaxTimeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMaxTimeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MaxTimeViewParam;
                break;
                
            default:
                result = kAudioUnitErr_InvalidParameter;
                break;
            }
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }
    


	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SpikeAudioUnit::GetPropertyInfo (AudioUnitPropertyID	inID,
                                                        AudioUnitScope		inScope,
                                                        AudioUnitElement	inElement,
                                                        UInt32 &		outDataSize,
                                                        Boolean &		outWritable)
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
				outWritable = false;
				outDataSize = sizeof (AudioUnitCocoaViewInfo);
				return noErr;
            
            case kAudioUnitProperty_TriggeredSpikes:
				outWritable = true;
				outDataSize = sizeof(TriggeredSpikes);
				return noErr;
		}
	}

	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SpikeAudioUnit::GetProperty(	AudioUnitPropertyID inID,
															AudioUnitScope 		inScope,
															AudioUnitElement 	inElement,
															void *				outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.audiounit.SpikeAudioUnit") );
				
				if (bundle == NULL) return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
                    CFSTR("SpikeAudioUnit_CocoaViewFactory"), 
                    CFSTR("bundle"), 
                    NULL);
                
                if (bundleURL == NULL) return fnfErr;

				AudioUnitCocoaViewInfo cocoaInfo;
				cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
				cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "SpikeAudioUnit_CocoaViewFactory", kCFStringEncodingUTF8);
				
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
				
				return noErr;
			}
            break;
                
            
            case kAudioUnitProperty_TriggeredSpikes:
            {
                TriggeredSpikes *overview = (TriggeredSpikes *)outData;
                if(mKernelList.size() > 0){
                    ((SpikeAudioUnitKernel *)mKernelList[0])->getTriggeredSpikes(overview);
                } else {
                    overview->n_spikes = 0;
                    overview->spike_containers = NULL;
                }
                
                return noErr;
            }
            
		}
	}

	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}


void SpikeAudioUnit::SpikeAudioUnitKernel::getTriggeredSpikes(TriggeredSpikes *spikes){

    // run through once quickly to see how many there are
    int n_spikes = 0;
    SpikeContainerList::iterator i = spike_display_queue.begin();
    while(i != spike_display_queue.end()){
        i++;
        n_spikes++;
    }
    
    spikes->spike_containers = (AUSpikeContainer **)calloc(n_spikes, sizeof(AUSpikeContainer *));
    
    i = spike_display_queue.begin();
    int s = 0;
    while(s < n_spikes && i != spike_display_queue.end()){
        
        spikes->spike_containers[s] = (*i).deep_copy();
        spike_display_queue.deferred_remove(*i);
        spike_recycle_queue.deferred_add(*i);
        i++;
        s++;
    }
    
    
    spikes->n_spikes = s;

}


#pragma mark ____SpikeAudioUnitEffectKernel


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::SpikeAudioUnitKernel::Reset()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void		SpikeAudioUnit::SpikeAudioUnitKernel::Reset()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SpikeAudioUnit::SpikeAudioUnitKernel::Process
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SpikeAudioUnit::SpikeAudioUnitKernel::Process(	const Float32 	*inSourceP,
                                                    Float32		 	*inDestP,
                                                    UInt32 			inFramesToProcess,
                                                    UInt32			inNumChannels, // for version 2 AudioUnits inNumChannels is always 1
                                                    bool			&ioSilence )
{

	//This code will pass-thru the audio data.
	//This is where you want to process data to produce an effect.
	
    Assert( inNumChannels == 1, "Unable to process more than one channel at a time");
    
	UInt32 nSampleFrames = inFramesToProcess;
	
    const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
    Float32 threshold = GetParameter( kThresholdParam );
    
//    // always put the data in the ring buffer
    input_buffer_list->SetBytes( inFramesToProcess * sizeof(Float32), (void *)sourceP);
    AudioBufferList abl;
    capture_buffer.Store(input_buffer_list->ToAudioBufferList(&abl), inFramesToProcess, frame_number);
//    Float32 *buffer = (Float32 *)(temp_buffer_list.mBuffers[0].mData);
//    buffer[0] = inputSample;
//    capture_buffer.Store(&temp_buffer_list, 1, frame_number);
    
    
    spike_display_queue.update();
    spike_recycle_queue.update();
    
	while (nSampleFrames-- > 0) {
        
		Float32 inputSample = *sourceP;
		
		//The current (version 2) AudioUnit specification *requires* 
	    //non-interleaved format for all inputs and outputs. Therefore inNumChannels is always 1
		
		sourceP += inNumChannels;	// advance to next frame (e.g. if stereo, we're advancing 2 samples);
									// we're only processing one of an arbitrary number of interleaved channels

        // here's where you do your DSP work
        Float32 outputSample = inputSample;
        
        // Simply pass on the data to the rest of CoreAudio
		*destP = outputSample;
		destP += inNumChannels;
        
        if(pending_trigger > 0){  // trigger is pending
            pending_trigger--;
            
        } else if(pending_trigger == 0){
            
            n_triggers++;
            //if(n_triggers % 100 == 0){
            //  std::cerr << "ntrigger: " << n_triggers << std::endl;
            //}
            
            AudioBufferList *buffer_list = &capture_buffer_list->GetModifiableBufferList();
            // TRIGGER THAT SHIT
                        
            capture_buffer.Fetch(buffer_list,   PRE_TRIGGER + POST_TRIGGER , frame_number - (PRE_TRIGGER + POST_TRIGGER), false); 
            
            AUSpikeContainer container = getFreshSpikeContainer();
            memcpy(container.buffer, buffer_list->mBuffers[0].mData, (PRE_TRIGGER + POST_TRIGGER) * sizeof(Float32));
            
            spike_display_queue.deferred_add(container);
            spike_display_queue.update();
            
            pending_trigger = -1; // rearm
            refractory_count = 5;
        
        } else if(pending_trigger < 0){ // trigger is active
            
            if(refractory_count){
                refractory_count--;
            } else if(last_sample > threshold && inputSample < threshold){
                pending_trigger += POST_TRIGGER ; // send out this waveform when it is fully captured
                
                #ifdef EMIT_MIDI
                    midi_enpoint->sendMessage(0x90, 0x00, 0x7F);
                #endif
            } 
        }
        
        frame_number++;
        last_sample = inputSample;
    }
    
    
}


AUSpikeContainer SpikeAudioUnit::SpikeAudioUnitKernel::getFreshSpikeContainer(){
  

  
  SpikeContainerList::iterator i = spike_recycle_queue.begin();
  if(i != spike_recycle_queue.end()){
      AUSpikeContainer recycled_container = *i;
      spike_recycle_queue.deferred_remove(*i);
      spike_recycle_queue.update();
      return recycled_container;
  } else {
      // grab it from the display queue
      SpikeContainerList::iterator j = spike_display_queue.begin();
      if(j == spike_display_queue.end()){
          // Deep shit here
          std::cerr << "deep shit" << std::endl;
          AUSpikeContainer brand_new;
          return brand_new;
      }
      
      SpikeContainerList::iterator j_last = j;
      while(j != spike_display_queue.end()){
          j_last = j;
          j++;
      }
      
      AUSpikeContainer recycled_from_display = *j_last;
      spike_display_queue.deferred_remove(*j_last);
      spike_display_queue.update();
      
      return recycled_from_display;
  }
}
                              
