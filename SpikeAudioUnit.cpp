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
#include <math.h>

//#define EMIT_MIDI

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
    SetParameter(kGainParam, kDefaultValue_GainParam );
    SetParameter(kUnitsPerVoltParam, kDefaultValue_UnitsPerVoltParam );
    SetParameter(kChannelIDParam, SpikeAudioUnit::channel_count++);
    SetParameter(kAutoThresholdHighParam, kDefaultValue_AutoThresholdHighParam );
    SetParameter(kAutoThresholdLowParam, kDefaultValue_AutoThresholdLowParam );
    SetParameter(kAutoThresholdFactorParam, kDefaultValue_AutoThresholdFactorParam );
    
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
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_ThresholdParam;
                break;
            
            case kMinAmplitudeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMinAmplitudeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MinAmplitudeViewParam;
                break;
                
            case kMaxAmplitudeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMaxAmplitudeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MaxAmplitudeViewParam;
                break;
                
            case kMinTimeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMinTimeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MinTimeViewParam;
                break;
            
            
                
            case kMaxTimeViewParam:
                AUBase::FillInParameterName (outParameterInfo, kMaxTimeViewParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1;
                outParameterInfo.defaultValue = kDefaultValue_MaxTimeViewParam;
                break;
            
            case kChannelIDParam:
                AUBase::FillInParameterName (outParameterInfo, kChannelIDParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 48.0;
                outParameterInfo.defaultValue = 1.0;
                break;    
            
            case kGainParam:
                AUBase::FillInParameterName (outParameterInfo, kGainParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 100000.0;
                outParameterInfo.defaultValue = kDefaultValue_GainParam;
                //outParameterInfo.flags = kAudioUnitParameterFlag_DisplayLogarithmic ;
                break;
            
            case kUnitsPerVoltParam:
                AUBase::FillInParameterName (outParameterInfo, kUnitsPerVoltParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Ratio;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 20.0;
                outParameterInfo.defaultValue = kDefaultValue_UnitsPerVoltParam;
                break;
            
            
            case kAutoThresholdHighParam:
                AUBase::FillInParameterName (outParameterInfo, kAutoThresholdHighParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 1.0;
                outParameterInfo.defaultValue = kDefaultValue_AutoThresholdHighParam;
                break;


            case kAutoThresholdLowParam:
                AUBase::FillInParameterName (outParameterInfo, kAutoThresholdLowParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 1.0;
                outParameterInfo.defaultValue = kDefaultValue_AutoThresholdLowParam;
                break;
            
            case kAutoThresholdFactorParam:
                AUBase::FillInParameterName (outParameterInfo, kAutoThresholdFactorParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = 1.0;
                outParameterInfo.maxValue = 5.0;
                outParameterInfo.defaultValue = kDefaultValue_AutoThresholdFactorParam;
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
                
		}
	}

	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
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
    Float32 threshold = GetParameter( kThresholdParam ) * (GetParameter(kUnitsPerVoltParam) * GetParameter(kGainParam));
    int current_channel_id = GetParameter( kChannelIDParam );
    if(current_channel_id != channel_id){
        channel_id = current_channel_id;
        connectChannelSocket(channel_id); 
        connectControlSockets(channel_id); 
    }
    
    // always put the data in the ring buffer
    input_buffer_list->SetBytes( inFramesToProcess * sizeof(Float32), (void *)sourceP);
    AudioBufferList abl;
    capture_buffer.Store(input_buffer_list->ToAudioBufferList(&abl), inFramesToProcess, frame_number);

    
    int fresh_spikes = 0;
        
    bool autothreshold_high = GetParameter(kAutoThresholdHighParam);
    bool autothreshold_low = GetParameter(kAutoThresholdLowParam);
    
    
    #define AUTOTHRESHOLD_NSAMPLES  44100
    
    // arm the autothresholding mechanism
    if(!autothresholding_armed && (autothreshold_high || autothreshold_low)){
        autothresholding_armed = true;
        autothresholding_count = AUTOTHRESHOLD_NSAMPLES;
        n_autothreshold_samples = 0;
        crest_factor = GetParameter(kAutoThresholdFactorParam);
        sample_sum = 0.0;
        sample_sumsq = 0.0;
        
        if(autothreshold_low){
            crest_factor *= -1;
        }
        
    }
        
    #define MIN_FRAMES_BETWEEN_UPDATES  4000
    
    broadcast_update_counter--;
    if(broadcast_update_counter <= 0){
        announceState(frame_number);
        broadcast_update_counter = 5;
    }
    
    checkCtlMessages();
    
	while (nSampleFrames-- > 0) {
        
		Float32 inputSample = *sourceP;
		
		//The current (version 2) AudioUnit specification *requires* 
	    //non-interleaved format for all inputs and outputs. Therefore inNumChannels is always 1
		
		sourceP += inNumChannels;	// advance to next frame (e.g. if stereo, we're advancing 2 samples);
									// we're only processing one of an arbitrary number of interleaved channels

        // here's we would do DSP work, if we were doing any
        Float32 outputSample = inputSample;
        
        // Simply pass on the data to the rest of CoreAudio
		*destP = outputSample;
		destP += inNumChannels;
        
        
        
        
        if(autothresholding_armed && autothresholding_count > 0){
            sample_sum += inputSample;
            sample_sumsq += inputSample*inputSample;
            
            autothresholding_count--;
            n_autothreshold_samples++;
        }
        
        if(autothresholding_armed && autothresholding_count <= 0){
            autothresholding_armed = false;
            float sample_std = sqrt((sample_sumsq / n_autothreshold_samples) - 
                                    (sample_sum/n_autothreshold_samples) * (sample_sum/n_autothreshold_samples)) ;
            
            float new_thresh = (crest_factor * sample_std) / (GetParameter(kUnitsPerVoltParam) * GetParameter(kGainParam));
            
            setGlobalParameter(kThresholdParam, new_thresh);
                        
            
            float dir = 1.0;
            if(new_thresh < 0.0){
                dir = -1.0;
            }
            setGlobalParameter(kMaxAmplitudeViewParam, dir * new_thresh * 2.);
            setGlobalParameter(kMinAmplitudeViewParam, dir * new_thresh * -2.);
        }
        
        if(pending_trigger > 0){  // trigger is pending
            pending_trigger--;
            
        } else if(pending_trigger == 0){
            
            n_triggers++;
            
            AudioBufferList *buffer_list = &capture_buffer_list->GetModifiableBufferList();
            
            // TRIGGER 
            capture_buffer.Fetch(buffer_list,   PRE_TRIGGER + POST_TRIGGER , frame_number - (PRE_TRIGGER + POST_TRIGGER), false); 

            
            // copy the spike wave into a protocol buffer object
            SpikeWaveBuffer wave;
            wave.set_channel_id(channel_id);
            wave.set_time_stamp(frame_number);
            
            if(buffer_list->mBuffers[0].mData == NULL){
                std::cerr << "Bad CA Buffer" << std::endl;
            }
            
            Float32 *float_buffer = (Float32 *)(buffer_list->mBuffers[0].mData);
            for(int i = 0; i < (PRE_TRIGGER + POST_TRIGGER); i++){
                wave.add_wave_sample(*(float_buffer + i));
            }
                    
            string serialized;
            wave.SerializeToString(&serialized);
            zmq::message_t msg(serialized.length());
            memcpy(msg.data(), serialized.c_str(), serialized.length());
            bool rc = message_socket->send(msg);
            
        
            
            if(!rc){
                std::cerr << "ZMQ: " << zmq_strerror(zmq_errno()) << std::endl;
            }             
            
            
            fresh_spikes++;
                                    
            pending_trigger = -1; // rearm
            refractory_count = 5;
        
        } else if(pending_trigger < 0){ // trigger is active
            
            if(refractory_count){
                refractory_count--;
            } else if(last_sample > threshold && inputSample < threshold){
                pending_trigger += POST_TRIGGER ; // send out this waveform when it is fully captured
                
                
            } 
        }
        
        
        frame_number++;
        frames_since_last_update++;
        last_sample = inputSample;
    }
    
}

int SpikeAudioUnit::channel_count = 0;


                              
