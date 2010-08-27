/*
 *  SharedTypes.h
 *  SpikeAudioUnit
 *
 *  Created by David Cox on 6/16/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include <vector>


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
        buffer = (Float32 *)calloc(PRE_TRIGGER + POST_TRIGGER, sizeof(Float32));
    }
    
    // constructor for recycling an existing buffer
    AUSpikeContainer(Float32* _buffer){
        buffer = _buffer;
    }
    
    // only call when the program is finished; otherwise, we'll
    // recycle the buffer
    void dispose(){
        delete buffer;
    }
    
    bool operator==(const AUSpikeContainer& container){
        return (buffer == container.buffer && timestamp == container.timestamp); 
    }
    
    AUSpikeContainer *deep_copy(){
        
        AUSpikeContainer *return_val = new AUSpikeContainer();
        memcpy(return_val->buffer, buffer, (PRE_TRIGGER + POST_TRIGGER)*sizeof(Float32));
        
        return return_val;
    }
};



struct TriggeredSpikes {
    
    int n_spikes;
    std::vector<AUSpikeContainer> spike_vector;
    
};
typedef struct TriggeredSpikes TriggeredSpikes;

