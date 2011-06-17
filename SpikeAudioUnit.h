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
#include <AudioToolbox/AudioUnitUtilities.h>

#include <zmq.hpp>
#include <iostream>
#include <sstream>

#include "spike_wave.pb.h"
#include "ctl_message.pb.h"


using namespace boost;

#define DEFAULT_BUFFER_SIZE     44100
#define DONT_RECYCLE_SPIKES


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
static const float kDefaultValue_GainParam = 1.0;
static const float kDefaultValue_UnitsPerVoltParam = 1.0;
static const float kDefaultValue_AutoThresholdHighParam = 0.0;
static const float kDefaultValue_AutoThresholdLowParam = 0.0;
static const float kDefaultValue_AutoThresholdFactorParam = 5.0;

static CFStringRef kThresholdParamName = CFSTR("Trigger Threshold");
static CFStringRef kMinAmplitudeViewParamName = CFSTR("Min Amplitude");
static CFStringRef kMaxAmplitudeViewParamName = CFSTR("Max Amplitude");
static CFStringRef kMinTimeViewParamName = CFSTR("Min Time");
static CFStringRef kMaxTimeViewParamName = CFSTR("Max Time");
static CFStringRef kChannelIDParamName = CFSTR("Channel ID");
static CFStringRef kGainParamName = CFSTR("Preamplification Gain");
static CFStringRef kUnitsPerVoltParamName = CFSTR("CoreAudio units / volt");
static CFStringRef kAutoThresholdHighParamName = CFSTR("Auto-threshold (HIGH)");
static CFStringRef kAutoThresholdLowParamName = CFSTR("Auto-threshold (LOW)");
static CFStringRef kAutoThresholdFactorParamName = CFSTR("Auto-threshold factor (stdevs)");



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
	
    
    static int channel_count;

    class SpikeAudioUnitKernel : public AUKernelBase		// most real work happens here
	{  
        
        private:
            int frames_since_last_update;
            float crest_factor; 
            float sample_sum;
            float sample_sumsq;
            int autothresholding_count;
            bool autothresholding_armed;
            int n_autothreshold_samples;
            
            int broadcast_update_counter;
            
        public:
            SpikeAudioUnitKernel(AUEffectBase *inAudioUnit ): AUKernelBase(inAudioUnit),
                                                              message_ctx(1){ 
                
                frames_since_last_update = 0;
                crest_factor = 0.0; 
                sample_sum = 0.0;
                sample_sumsq = 0.0;
                autothresholding_count = 0;
                autothresholding_armed = false;
                n_autothreshold_samples = 0;
                broadcast_update_counter = 0;
                                                                  
                
                capture_buffer.Allocate(1, sizeof(Float32), DEFAULT_BUFFER_SIZE);//2048);
                
                CAStreamBasicDescription	bufClientDesc;		
                bufClientDesc.SetCanonical(1, false);
                bufClientDesc.mSampleRate = 44100;
                                
                
                input_buffer_list = CABufferList::New("input intermediate buffer", bufClientDesc);
                input_buffer_list->AllocateBuffers(10 * (PRE_TRIGGER + POST_TRIGGER) & sizeof(Float32));
                
                pending_trigger = -1; // armed
                refractory_count = 0;
                
                frame_number = 0;
                n_triggers = 0;
                
                last_sample = 0.0;
                
                capture_buffer_list = CABufferList::New("capture buffer", bufClientDesc );
                capture_buffer_list->AllocateBuffers(DEFAULT_BUFFER_SIZE * sizeof(Float32));//10 * (PRE_TRIGGER + POST_TRIGGER) * sizeof(Float32));
                
                channel_id = (int)GetParameter( kChannelIDParam );
                                                                  
                
                connectChannelSocket(channel_id);
                connectControlSockets(channel_id);
                
            }
		
            // *Required* overides for the process method for this effect
            // processes one channel of interleaved samples
            virtual void 		Process(	const Float32 	*inSourceP,
                                            Float32		 	*inDestP,
                                            UInt32 			inFramesToProcess,
                                            UInt32			inNumChannels,
                                            bool			&ioSilence);
            
            virtual void		Reset();
		
                
            void setGlobalParameter(AudioUnitParameterID param_id, AudioUnitParameterValue val, bool silent = false){
                mAudioUnit->SetParameter(param_id, val);
                
                AudioUnitEvent myEvent;
                
                myEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
                myEvent.mArgument.mParameter.mAudioUnit = mAudioUnit->GetComponentInstance();
                myEvent.mArgument.mParameter.mParameterID = param_id;
                myEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
                myEvent.mArgument.mParameter.mElement = 0;
                
                AUEventListenerNotify(NULL, NULL, &myEvent);
                
                if (! silent ){
                    sendCtlMessage(0, param_id, (double)val);
                }
            }
            
            void announceState(long frame_number){
                
                #define ANNOUNCE_PARAM(P)   sendCtlMessage(frame_number, P, GetParameter(P))
                ANNOUNCE_PARAM(kThresholdParam);
                ANNOUNCE_PARAM(kMaxAmplitudeViewParam);
                ANNOUNCE_PARAM(kMinAmplitudeViewParam);
                ANNOUNCE_PARAM(kMinTimeViewParam);
                ANNOUNCE_PARAM(kMaxTimeViewParam);
            
            }
            
            void sendCtlMessage(long frame_number, int param, double value){
                
                CtlMessage ctl_msg;
                ctl_msg.set_channel_id(channel_id);
                ctl_msg.set_time_stamp(frame_number);

                switch(param){
                    case kThresholdParam:
                        ctl_msg.set_message_type(CtlMessage::THRESHOLD);
                        break;
                    case kMaxAmplitudeViewParam:
                        ctl_msg.set_message_type(CtlMessage::AMPLITUDE_MAX);
                        break;
                    case kMinAmplitudeViewParam:
                        ctl_msg.set_message_type(CtlMessage::AMPLITUDE_MIN);
                        break;
                    case kMinTimeViewParam:
                        ctl_msg.set_message_type(CtlMessage::TIME_MIN);
                        break;
                    case kMaxTimeViewParam:
                        ctl_msg.set_message_type(CtlMessage::TIME_MAX);
                        break;
                    default:
                        return;
                }
                ctl_msg.set_value(value);
                
                string serialized;

                ctl_msg.SerializeToString(&serialized);
                zmq::message_t msg(serialized.length());
                memcpy(msg.data(), serialized.c_str(), serialized.length());
                bool rc = ctl_send_socket->send(msg);
            }
            
            
            void checkCtlMessages(){
            
                // read from incoming ctl sockets
                bool msg_received;
                zmq::message_t msg;
                
                
                // Receive a message 
                msg_received = ctl_receive_socket->recv(&msg, ZMQ_NOBLOCK);
                
                
                while(msg_received){

                    string data((const char *)msg.data(), msg.size());
                    CtlMessage ctl_msg;
                    ctl_msg.ParseFromString(data);
                    
                    // use setGlobalParameter as needed to update the global AU state
                    switch(ctl_msg.message_type()){
                    
                        case CtlMessage::THRESHOLD:
                            double new_thresh = ctl_msg.value();
                            setGlobalParameter(kThresholdParam, new_thresh, true);
                            break;
                    
                    }

                    
                    msg_received = ctl_receive_socket->recv(&msg, ZMQ_NOBLOCK);
                }
            }
            
                        
        protected:
        
            
        
            int n_triggers;
            
            Float32 last_sample;
            
            SInt64 frame_number;
            
            int pending_trigger; // a count-down until a buffer is full and ready to dispatch
                                 // -1: no pending trigger
                                 // +n: n samples until trigger is tripped
                                 // 0: trigger now! (copy from ring buffer to spike queue)
            
            
            int refractory_count;
            
            
            CARingBuffer capture_buffer;
            
            AudioBufferList temp_buffer_list;
            AudioBuffer temp_buffer;
            
            CABufferList *input_buffer_list;
            CABufferList *capture_buffer_list;
            
            //shared_ptr<MIDIEndpoint> midi_endpoint; 
            
            zmq::context_t message_ctx;
            shared_ptr<zmq::socket_t> message_socket;
            
            shared_ptr<zmq::socket_t> ctl_receive_socket;
            shared_ptr<zmq::socket_t> ctl_send_socket;
        
            int channel_id;
            
            
            
            #define HOST_ADDRESS "tcp://127.0.0.1" 
            #define SPIKE_BASE_PORT 8000
            #define CTL_IN_BASE_PORT 9000
            #define CTL_OUT_BASE_PORT 10000

        
            void connectChannelSocket(int channel_id){
                
                std::cerr << "changing to channel " << channel_id << std::endl;
                
                // generate a fresh socket
                message_socket = shared_ptr<zmq::socket_t>(new zmq::socket_t(message_ctx, ZMQ_PUB));
                
                uint64_t hwm = 1000;
                message_socket->setsockopt(ZMQ_HWM, &hwm, sizeof(uint64_t));
                
                // construct the url
                ostringstream filename_stream, url_stream;
                
//                // hacky filesystem manipulation
//                filename_stream << "/tmp/spike_channels";
//                
//                string mkdir_command("mkdir -p ");
//                mkdir_command.append(filename_stream.str());
//                system(mkdir_command.c_str());
//                
//                filename_stream << "/" << channel_id;
//                string touch_command("touch ");
//                touch_command.append(filename_stream.str());
//                system(touch_command.c_str());
//                
//                url_stream << "ipc://" << filename_stream.str();

                url_stream << HOST_ADDRESS << ":" << SPIKE_BASE_PORT + channel_id;

                try {
                    message_socket->bind(url_stream.str().c_str());
                    //std::cerr << "ZMQ server bound successfully to " << url_stream.str() << std::endl;
                } catch (zmq::error_t& e) {
                    std::cerr << "ZMQ: " << e.what() << std::endl;
                }
            }
            
            
            void connectControlSockets(int channel_id){
                
                // Create send and receive sockets
                ctl_receive_socket = shared_ptr<zmq::socket_t>(new zmq::socket_t(message_ctx, ZMQ_SUB));
                ctl_send_socket = shared_ptr<zmq::socket_t>(new zmq::socket_t(message_ctx, ZMQ_PUB));
                
                uint64_t hwm = 1000;
                ctl_receive_socket->setsockopt(ZMQ_HWM, &hwm, sizeof(uint64_t));
                ctl_send_socket->setsockopt(ZMQ_HWM, &hwm, sizeof(uint64_t));                
                
                // subscribe to all messages on receive
                ctl_receive_socket->setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
                
                // construct the url
                ostringstream receive_filename_stream, send_filename_stream, receive_url_stream, send_url_stream;
                
                                
                // connect receive socket
                receive_url_stream << HOST_ADDRESS << ":" << CTL_IN_BASE_PORT + channel_id;
                try {
                    ctl_receive_socket->connect(receive_url_stream.str().c_str());
                    //std::cerr << "ZMQ ctl client bound successfully to " << receive_url_stream.str() << std::endl;
                } catch (zmq::error_t& e) {
                    std::cerr << "ZMQ (ctl receive): " << receive_url_stream.str() << e.what() << std::endl;
                }
                
                // bind send socket
                send_url_stream << HOST_ADDRESS << ":" << CTL_OUT_BASE_PORT + channel_id;
                try {
                    ctl_send_socket->bind(send_url_stream.str().c_str());
                    //std::cerr << "ZMQ ctl server bound successfully to " << send_url_stream.str() << std::endl;
                } catch (zmq::error_t& e) {
                    std::cerr << "ZMQ (ctl send): " << "[" << send_url_stream.str() << "] " << e.what() << std::endl;
                }

                
            }
        
        
               
	};
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif