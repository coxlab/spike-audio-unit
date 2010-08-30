/*
 *  SharedTypes.h
 *  SpikeAudioUnit
 *
 *  Created by David Cox on 6/16/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef SHARED_TYPES_H_
#define SHARED_TYPES_H_

#include <zmq.hpp>


#define PRE_TRIGGER     33
#define POST_TRIGGER    33

enum
{
	kAudioUnitProperty_TriggeredSpikes = 65536,
    //	kAudioUnitProperty_SampleTimeStamp = 65537,
};


class AUSpikeContainer {

    
public:
    Float32 *buffer;
    SInt64 timestamp;
    
    // constructor for building a fresh waveform buffer
    AUSpikeContainer(){
        buffer = new Float32[PRE_TRIGGER + POST_TRIGGER];
    }
    
    // constructor for recycling an existing buffer
    AUSpikeContainer(Float32* _buffer){
        buffer = _buffer;
    }
    
    
    
    // only call when the program is finished; otherwise, we'll
    // recycle the buffer
    void dispose(){
        if(buffer != NULL){
            delete [] buffer;
            buffer = NULL;
        }
    }
    
    
    bool operator==(const AUSpikeContainer& container){
        return (buffer == container.buffer && timestamp == container.timestamp); 
    }
    
//    AUSpikeContainer *deep_copy(){
//        
//        AUSpikeContainer *return_val = new AUSpikeContainer();
//        memcpy(return_val->buffer, buffer, (PRE_TRIGGER + POST_TRIGGER)*sizeof(Float32));
//        
//        return return_val;
//    }
//    
//    AUSpikeContainer *shallow_copy(){
//        
//        AUSpikeContainer *return_val = new AUSpikeContainer(buffer);        
//        return return_val;
//    }
    

    
};



struct TriggeredSpikes {
    
    int n_spikes;
    AUSpikeContainer **spike_containers;
    
};
typedef struct TriggeredSpikes TriggeredSpikes;

#endif

