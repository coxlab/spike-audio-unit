/*
 *  SpikeWave.h
 *  spike_visualization
 *
 *  Created by David Cox on 6/14/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include <OpenGL/gl.h>
#include <cstdlib>

namespace spike_visualization {
   
    // A class for representing a spike waveform for display purposes
    
    template <typename T>
    class SpikeWave {
        
    protected:    
        int length;
        T *data;
        T start_time;
        T data_interval;
        
    public:
        
        SpikeWave(int _length, T _start_time, T _data_interval){
            
            length = _length;
            start_time = _start_time;
            data_interval = _data_interval;
            data = new T[length];
        }
        
        SpikeWave(int _length, T _start_time, T _data_interval, T *_data){
        
            length = _length;
            start_time = _start_time;
            data_interval = _data_interval;
            data = new T[length];
            if(_data != NULL){
                memcpy(data, _data, length * sizeof(T));
            }
        }
        
        ~SpikeWave(){
            delete [] data;
        }
        
        T *getData(){ 
            return data; 
        }
        
        int getLength(){
            return length;
        }
        
        T getStartTime(){
            return start_time;
        }
        
        void setStartTime(T seconds){
            start_time = seconds;
        }
        
        T getDataInterval(){
            return data_interval;
        }
        
        void setDataInterval(T _data_interval){
            data_interval = _data_interval;
        }
        
        void scaleByFactor(double factor){
            for(int i = 0; i < length; i++){
                data[i] *= factor;
            }
        }
        
    };
    
    typedef SpikeWave<GLfloat> GLSpikeWave;
}

