/*
 *  spike_visualization.h
 *  spike_visualization
 *
 *  Created by David Cox on 6/14/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef SpikeRenderer_
#define SpikeRenderer_

#include "SpikeWave.h"
#include "GLStringRenderer.h"
#include "CocoaGLStringRenderer.h"
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <list>
#include <iostream>
#include <math.h>

#define SP_TRIGGER_THRESHOLD_SELECT 1
#define SP_AMPLITUDE_MAX_SELECT 2
#define SP_AMPLITUDE_MIN_SELECT 3
#define SP_TIME_MAX_SELECT 4
#define SP_TIME_MIN_SELECT 5
#define SP_AUTOTHRESH_UP_SELECT 6
#define SP_AUTOTHRESH_DOWN_SELECT 7


namespace spike_visualization {
    
    // Note: we use GLfloat's everywhere here since the contained classes are 
    // all concerned with display and visualization. Data sent to disk
    // should be encoded in native (e.g. short) format

    using namespace boost;
    using namespace std;

    typedef struct SpikeWaveSelectionActionStruct {

        int action_type;
        int subject;
        int subsubject;
            
    } SpikeWaveSelectionAction;


    enum AutoThresholdState {
        AUTO_THRESHOLD_OFF,
        AUTO_THRESHOLD_HIGH,
        AUTO_THRESHOLD_LOW
    };

    class HitTestRegion {
        
        protected:
            SpikeWaveSelectionAction action;
        
        public:
        
            HitTestRegion(int action_type, int subject, int subsubject){
                action.action_type = action_type;
                action.subject = subject;
                action.subsubject = subsubject;
            }
            
            SpikeWaveSelectionAction getAction(){ return action; }
            
            virtual bool hitTest(GLfloat _x, GLfloat _y) = 0;
    };
        

    class RectangularHitTestRegion : public HitTestRegion {

        GLfloat x,y,w,h;

        public:
        
            RectangularHitTestRegion(GLfloat _x, GLfloat _y, GLfloat _w, GLfloat _h, int action_type, int subject, int subsubject) : HitTestRegion(action_type, subject, subsubject){
                x = _x;
                y = _y;
                w = _w;
                h = _h;
            }
            
            void setRegion(GLfloat _x, GLfloat _y, GLfloat _w, GLfloat _h){
                x = _x;  y = _y;  w = _w;  h = _h;
            }
            
            bool hitTest(GLfloat _x, GLfloat _y){
                if(_x >= x && _x < x+w &&
                   _y >= y && _y < y+h){
                   
                   return true;
                }
                
                return false;
            }
    };
        
        
        
    class SpikeRenderer {
        
        protected:  

            list< shared_ptr<GLSpikeWave> > spike_list;
            
            //vector< shared_ptr<DiscriminatorWindowSet> > window_sets;
            
            // unit translation
            float units_per_volt;
            
            // amplifier gain factor
            float amplifier_gain;
            
            // view dimensions in pixels
            GLfloat view_width;
            GLfloat view_height;
        
            // bounds for drawing plots in pixels
            GLfloat data_viewport_x, data_viewport_y, data_viewport_width, data_viewport_height;
            
            GLfloat horizontal_range_frame_x, horizontal_range_frame_y, horizontal_range_frame_width, horizontal_range_frame_height;
            GLfloat vertical_range_frame_x, vertical_range_frame_y, vertical_range_frame_width, vertical_range_frame_height;
            
            GLfloat vertical_range_frame_offset, horizontal_range_frame_offset;
            
            GLfloat range_frame_major_tick_length;
            GLfloat top_padding, left_padding, right_padding, bottom_padding;
            
            
            GLfloat auto_threshold_buttons_edge_offset;
            GLfloat auto_threshold_buttons_width;
            GLfloat auto_threshold_buttons_height;
            GLfloat auto_threshold_buttons_separation;
        
            // bounds for the plotting area (natural units)
            GLfloat amplitude_range_max_volts;
            GLfloat amplitude_range_min_volts;
            GLfloat time_range_max_seconds;
            GLfloat time_range_min_seconds;
            
            float range_frame_font_size;
            float label_scale_factor;
            
            // The trigger threshold (in V)
            GLfloat threshold;
            
            // Autothresholding state
            AutoThresholdState auto_thresholding;
            
            // time of the first spike wave data point
            GLfloat spike_wave_time_offset;
            
            
            bool grid_on;
            bool hold_on;
            GLfloat persistence_time;

            // window discriminator parameters
            int n_window_sets;
            int n_windows_per_set;
        
            int max_spikes_to_show;
        
            // a utility helper to render strings to GL textures
            shared_ptr<GLStringRenderer> string_renderer;
            
            // hit test regions
            vector< shared_ptr<HitTestRegion> > hit_test_regions;
            shared_ptr< RectangularHitTestRegion > threshold_knob_region; // knob to move the threshold
            shared_ptr< RectangularHitTestRegion > amplitude_max_region; // knob to move amplitude scale
            shared_ptr< RectangularHitTestRegion > amplitude_min_region; // knob to move amplitude scale
            shared_ptr< RectangularHitTestRegion > time_max_region; // knob to move amplitude scale
            shared_ptr< RectangularHitTestRegion > time_min_region; // knob to move amplitude scale
            shared_ptr< RectangularHitTestRegion > auto_threshold_up_region;
            shared_ptr< RectangularHitTestRegion > auto_threshold_down_region;
        
        public:

            SpikeRenderer(GLfloat _width, 
                          GLfloat _height, 
                          int _n_window_sets, 
                          int _n_windows_per_set, 
                          GLfloat _min_ampl,
                          GLfloat _max_ampl,
                          GLfloat _min_time,
                          GLfloat _max_time){
                
                auto_thresholding = AUTO_THRESHOLD_OFF;
                                              
                units_per_volt = 1;
                n_window_sets = _n_window_sets;
                n_windows_per_set = _n_windows_per_set;
                
                time_range_min_seconds = _min_time;
                time_range_max_seconds = _max_time;
                amplitude_range_min_volts = _min_ampl;
                amplitude_range_max_volts = _max_ampl;
                
                vertical_range_frame_offset = 10;
                horizontal_range_frame_offset = 15;
                vertical_range_frame_width = 30;
                horizontal_range_frame_height = 30;
                top_padding = 20;
                left_padding = 20;
                right_padding = 20;
                bottom_padding = 10;
                
                auto_threshold_buttons_edge_offset = 5;
                auto_threshold_buttons_width = 7;
                auto_threshold_buttons_height = 7;
                auto_threshold_buttons_separation = 3;
                
                range_frame_major_tick_length = 5;
                range_frame_font_size = 6.0;
                label_scale_factor = 0.85;
                
                max_spikes_to_show = 15;
                
                setViewDimensions(_width, _height);
                
                string_renderer = shared_ptr<GLStringRenderer>(new CocoaGLStringRenderer());
                
                
                // define parts with which the user can interact
                threshold_knob_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_TRIGGER_THRESHOLD_SELECT, 0, 0));
                hit_test_regions.push_back(threshold_knob_region);

                amplitude_max_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_AMPLITUDE_MAX_SELECT, 0, 0));
                hit_test_regions.push_back(amplitude_max_region);

                amplitude_min_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_AMPLITUDE_MIN_SELECT, 0, 0));
                hit_test_regions.push_back(amplitude_min_region);

                time_max_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_TIME_MAX_SELECT, 0, 0));
                hit_test_regions.push_back(time_max_region);

                time_min_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_TIME_MIN_SELECT, 0, 0));
                hit_test_regions.push_back(time_min_region);

                auto_threshold_up_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_AUTOTHRESH_UP_SELECT, 0, 0));
                hit_test_regions.push_back(auto_threshold_up_region); 

                auto_threshold_down_region = shared_ptr< RectangularHitTestRegion >(new RectangularHitTestRegion(0.0,0.0,0.0,0.0, SP_AUTOTHRESH_DOWN_SELECT, 0, 0));
                hit_test_regions.push_back(auto_threshold_down_region); 

                
            }
            
            float getUnitsPerVolt() { return units_per_volt; }
            void setUnitsPerVolt(float _units_per_volt) { units_per_volt = _units_per_volt; } 
            
            float getAmplifierGain(){ return amplifier_gain; }
            void setAmplifierGain(float _gain) { amplifier_gain = _gain; }
            
            GLfloat getAmplitudeRangeMax(){ return amplitude_range_max_volts; }
            GLfloat getAmplitudeRangeMin(){ return amplitude_range_min_volts; }
        
            GLfloat getTimeRangeMax(){ return time_range_max_seconds; }
            GLfloat getTimeRangeMin(){ return time_range_min_seconds; }
        
            void setAmplitudeRangeMax(GLfloat new_max){ amplitude_range_max_volts = new_max; }
            void setAmplitudeRangeMin(GLfloat new_min){ amplitude_range_min_volts = new_min; }
            
            void setTimeRangeMax(GLfloat new_max){ time_range_max_seconds = new_max; }
            void setTimeRangeMin(GLfloat new_min){ time_range_min_seconds = new_min; }
            
            int getMaxSpikesToShow(){ return max_spikes_to_show; }
        
            bool hitTest(GLfloat view_x, GLfloat view_y, SpikeWaveSelectionAction *action){
                
                vector< shared_ptr<HitTestRegion> >::iterator i;
                
                for(i = hit_test_regions.begin(); i != hit_test_regions.end(); ++i){
                    shared_ptr<HitTestRegion> region = *i;
                    if(region->hitTest(view_x, view_y)){
                        *action = region->getAction();
                        return true;
                    }
                }
                
                return false;
            }
            
            
            void setAutoThresholdState(AutoThresholdState _state){
                auto_thresholding = _state;
            }
            
            AutoThresholdState getAutoThresholdState(){
                return auto_thresholding;
            }
            
            void pushSpikeWave(shared_ptr<GLSpikeWave> spike_wave){
                                
                spike_list.push_back(spike_wave);
                
                if(spike_list.size() > (unsigned int)max_spikes_to_show){
                    spike_list.pop_front();
                }
                
            }
            
            
            void setViewDimensions(GLfloat _width, GLfloat _height){
                view_width = _width;
                view_height = _height;
                
                data_viewport_width = view_width - left_padding - right_padding - vertical_range_frame_width - vertical_range_frame_offset;
                data_viewport_height = view_height - top_padding - bottom_padding - horizontal_range_frame_height - horizontal_range_frame_offset;
                data_viewport_x = left_padding + vertical_range_frame_width + vertical_range_frame_offset;
                data_viewport_y = bottom_padding + horizontal_range_frame_height + horizontal_range_frame_offset;
                
                vertical_range_frame_x = left_padding;
                vertical_range_frame_y = data_viewport_y;
                vertical_range_frame_height = data_viewport_height;
                
                horizontal_range_frame_x = data_viewport_y;
                horizontal_range_frame_y = bottom_padding;
                horizontal_range_frame_width = data_viewport_width;
                
                //cerr << "view_width="  << _width << " view_height=" << _height << endl;
                //cerr << "data viewport: x=" << data_viewport_x << " y=" << data_viewport_y << " width=" << data_viewport_width << " height=" << data_viewport_height << endl;
           
                //cerr << "vertical range frame: x=" << vertical_range_frame_x << " y=" << vertical_range_frame_y << " width=" << vertical_range_frame_width << " height=" << vertical_range_frame_height << endl;
                
            }
            
            // reset all the viewport and view transformations (e.g. after size update)
            void updateViewingWindow(){ 
            
            }
            
            // reset / rebuild window discriminators
            void resetWindowDiscriminators(){ }
            
            
            // convert view coordinates to data viewport coordinates
            void convertViewToDataCoordinates(GLfloat view_x, GLfloat view_y, GLfloat *data_x, GLfloat *data_y){ 
                GLfloat tmp_x = view_x - data_viewport_x;
                GLfloat tmp_y = view_y - data_viewport_y;
                
                *data_x = time_range_min_seconds + tmp_x * (time_range_max_seconds - time_range_min_seconds) / (data_viewport_width);
                *data_y = amplitude_range_min_volts + tmp_y * (amplitude_range_max_volts - amplitude_range_min_volts) / (data_viewport_height);
            }
            
            void convertDataToViewCoordinates(GLfloat data_x, GLfloat data_y, GLfloat *view_x, GLfloat *view_y){
                
                *view_x = data_viewport_x + (data_x - time_range_min_seconds) * data_viewport_width / (time_range_max_seconds - time_range_min_seconds);
                *view_y = data_viewport_y + (data_y - amplitude_range_min_volts) * data_viewport_height / (amplitude_range_max_volts - amplitude_range_min_volts);
            }
            
                
            void convertViewToDataSize(GLfloat view_x, GLfloat view_y, GLfloat *data_x, GLfloat *data_y){ 
                GLfloat tmp_x = view_x;
                GLfloat tmp_y = view_y ;
                
                *data_x = tmp_x * (time_range_max_seconds - time_range_min_seconds) / (data_viewport_width);
                *data_y = tmp_y * (amplitude_range_max_volts - amplitude_range_min_volts) / (data_viewport_height);
            }
            
            void convertDataToViewSize(GLfloat data_x, GLfloat data_y, GLfloat *view_x, GLfloat *view_y){
                
                *view_x = data_x * data_viewport_width / (time_range_max_seconds - time_range_min_seconds);
                *view_y = data_y * data_viewport_height / (amplitude_range_max_volts - amplitude_range_min_volts);
            }
            
            float convertVoltsToUnits(float inval){
                return inval * units_per_volt * amplifier_gain;
            }
            
            float convertUnitsToVolts(float inval){
                return inval / (units_per_volt * amplifier_gain);
            }
            
            // trigger threshold
            void setTriggerThreshold(GLfloat new_threshold){
                threshold = new_threshold;
            }
            
            // window discriminators
            void setDiscriminatorWindowMax(int set_id, int window_id, GLfloat new_max){ }
            void setDiscriminatorWindowMin(int set_id, int window_id, GLfloat new_min){ }
            void setDiscriminatorWindowTime(int set_id, int window_id, GLfloat new_time_offset){ }
            
            
            // Render Functions
            
            void render(){
            
                glEnable(GL_SCISSOR_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glPushMatrix();
                selectFullViewport();
                renderVerticalRangeFrame();
                renderHorizontalRangeFrame();
                renderAutoThresholdButtons();
                renderThresholdKnob();
                glPopMatrix();
            
               
                
                glPushMatrix();
                selectDataViewport();
                renderWaves();
                renderThresholdLine();
                glPopMatrix();
            
                glFlush();
            }
            
            
            void selectFullViewport(){
                glViewport(0, 0, view_width, view_height);
                glScissor(0, 0, view_width,view_height);
                glOrtho(0, view_width, 0, view_height, -1.0, 1.0);  
                
                glClearColor(0.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            
            void selectDataViewport(){
                glViewport(data_viewport_x, data_viewport_y, data_viewport_width, data_viewport_height);
                
                glScissor(data_viewport_x, data_viewport_y, data_viewport_width, data_viewport_height);
                glOrtho(time_range_min_seconds, time_range_max_seconds, amplitude_range_min_volts, amplitude_range_max_volts, -1.0, 1.0);
                
                glClearColor(0.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            
            void renderThresholdKnob(){
            
                glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
                
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
                GLfloat view_x, view_y;
                convertDataToViewCoordinates(0.0, threshold, &view_x, &view_y);
                
                
                GLfloat marker_width = 8;
                
                glColor3f(1.0, 1.0, 0.0);
                glBegin(GL_LINE_STRIP);
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width - marker_width, view_y+marker_width/2.0);
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width, view_y); // point
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width - marker_width, view_y-marker_width/2.0);
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width - marker_width, view_y+marker_width/2.0);
                glEnd();
                
                glBegin(GL_TRIANGLES);
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width - marker_width, view_y+marker_width/2.0);
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width, view_y); // point
                glVertex2f(vertical_range_frame_x + vertical_range_frame_width - marker_width, view_y-marker_width/2.0);
                glEnd();
                
                glPopAttrib();
                
                // edit the hit region to match what we just drew
                threshold_knob_region->setRegion(vertical_range_frame_x + vertical_range_frame_width - marker_width,
                                                 view_y-marker_width/2.0,
                                                 marker_width,
                                                 marker_width);
            }
            
            void renderThresholdLine(){
            
                glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
                
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH, GL_NICEST);
                
                glLineWidth(2.0);
                
                glColor4f(1.0, 1.0, 0.0, 0.5);
                
                glBegin(GL_LINE_STRIP);
                glVertex2f(time_range_min_seconds, threshold);
                glVertex2f(time_range_max_seconds, threshold);
                glEnd();
                            
                glPopAttrib();
            }
            
            // draw the wave; assumes we're in the data viewport
            void renderWaves(){
            
                glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
                
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH, GL_NICEST);
                
                glLineWidth(2.0);
                
                
                list< shared_ptr<GLSpikeWave> >::iterator i;
                
                float alpha = 1.0;
                
                for(i = spike_list.begin(); i != spike_list.end(); ++i){
                    shared_ptr<GLSpikeWave> wave = *i;
                    GLfloat *thedata = wave->getData();
                       
                    GLfloat time_offset = wave->getStartTime();
                    GLfloat data_interval = wave->getDataInterval();
                    int length = wave->getLength();
                    //std::cerr << "Start time: " << time_offset << " Interval: " << data_interval << std::endl;
                    
                    glColor4f(1.0, 0.0, 0.0, 1.0 - alpha);
                    glBegin(GL_LINE_STRIP);
                    for(int j = 0; j < length; j++){
                        
                        glVertex2f(time_offset + j*data_interval, convertUnitsToVolts(thedata[j])); 
                    }
                    glEnd();
                    
                    alpha -= 1.0 / max_spikes_to_show;
                }
            
                glPopAttrib();
            }
            
            
            void renderAutoThresholdButtons(){
            
                if(auto_thresholding == AUTO_THRESHOLD_HIGH){
                    glColor3f(0.6, 0.0, 0.0);
                } else {
                    glColor3f(0.4, 0.4, 0.4);
                }
                
                // the "up" button
                GLfloat left = view_width - 2. * auto_threshold_buttons_width - 
                                                auto_threshold_buttons_edge_offset - 
                                                auto_threshold_buttons_separation;
                                                
                GLfloat right = left + auto_threshold_buttons_width;
                GLfloat horizontal_center = (left + right)/ 2.0;
                GLfloat top = view_height - auto_threshold_buttons_edge_offset;
                GLfloat bottom = top - auto_threshold_buttons_height; 
                
                glBegin(GL_TRIANGLES);
                glVertex2f( left, bottom );
                glVertex2f( right, bottom );
                glVertex2f( horizontal_center, top );
                glEnd();
                
                auto_threshold_up_region->setRegion(left, bottom,
                                                    auto_threshold_buttons_width,
                                                    auto_threshold_buttons_height);
                
                
                
                if(auto_thresholding == AUTO_THRESHOLD_LOW){
                    glColor3f(0.6, 0.0, 0.0);
                } else {
                    glColor3f(0.4, 0.4, 0.4);
                }
                
                // the "down" button
                left = right + auto_threshold_buttons_separation;
                right = left + auto_threshold_buttons_width;
                horizontal_center = (left + right)/ 2.0;

                glBegin(GL_TRIANGLES);
                glVertex2f( right, top );
                glVertex2f( horizontal_center, bottom );
                glVertex2f( left, top );
                glEnd();
                
                auto_threshold_down_region->setRegion(left, bottom,
                                                    auto_threshold_buttons_width,
                                                    auto_threshold_buttons_height);
                
            }
            
            // draw the range bars; assumes we're in a whole-view viewport
            void renderVerticalRangeFrame(){
            
                // draw a "]" shape, starting at the bottom left
                
                glColor3f(1.0, 1.0, 1.0);
                
                glBegin(GL_LINE_STRIP);
                GLfloat right_edge_x = vertical_range_frame_x + vertical_range_frame_width;
                glVertex2f( right_edge_x - range_frame_major_tick_length, vertical_range_frame_y);
                glVertex2f( right_edge_x, vertical_range_frame_y);
                glVertex2f( right_edge_x, vertical_range_frame_y+vertical_range_frame_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length, vertical_range_frame_y+vertical_range_frame_height);
                glEnd();
                
                // determine appropriate units
                string unit_string;
                float unit_multiplier = 1.0;
                float test_case = max(fabs(amplitude_range_max_volts), fabs(amplitude_range_min_volts));
                if(test_case < 0.1 && test_case >= 0.001){
                    // MILLIVOLTS
                    unit_string = "mV";
                    unit_multiplier *= 1000.0;
                } else if(test_case < 0.001 && test_case > 0.000001){
                    // MICROVOLTS
                    unit_string = "uV";
                    unit_multiplier *= 1000000.0;
                } else {
                    // VOLTS
                    unit_string = "V";
                }
                
                // render some labels
                float label_width, label_height;
                GLfloat pad = 4;
                
                glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
                glDisable (GL_DEPTH_TEST); // ensure text is not remove by depth buffer test.
                glEnable (GL_BLEND); // for text fading
                glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ditto
                glEnable (GL_TEXTURE_RECTANGLE_EXT);	
                
                
                string high_string = (boost::format("%.1f") % (amplitude_range_max_volts*unit_multiplier)).str();
                GLuint high_label = string_renderer->stringToTexture(high_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  high_label);
                
                GLfloat lf = label_scale_factor;
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,0.0); 
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - lf*label_width,   vertical_range_frame_y + vertical_range_frame_height + lf*label_height/2.0);
                glTexCoord2d(label_width,0.0);           
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   vertical_range_frame_y + vertical_range_frame_height  + lf*label_height/2.0);
                glTexCoord2d(label_width,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   vertical_range_frame_y + vertical_range_frame_height - lf*label_height/2.0);
                glTexCoord2d(0.0,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - lf*label_width,   vertical_range_frame_y + vertical_range_frame_height - lf*label_height/2.0);
                glEnd();
                
                amplitude_max_region->setRegion(right_edge_x - range_frame_major_tick_length - pad - lf*label_width,
                                           vertical_range_frame_y + vertical_range_frame_height - lf*label_height/2.0,
                                           label_width, label_height);
                
                string low_string = (boost::format("%.1f") % (amplitude_range_min_volts*unit_multiplier)).str();
                GLuint low_label = string_renderer->stringToTexture(low_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  low_label);
                
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,0.0); 
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - lf*label_width,   vertical_range_frame_y + lf*label_height/2.0);
                glTexCoord2d(label_width,0.0);           
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   vertical_range_frame_y + lf*label_height/2.0);
                glTexCoord2d(label_width,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   vertical_range_frame_y - lf*label_height/2.0);
                glTexCoord2d(0.0,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - lf*label_width,   vertical_range_frame_y - lf*label_height/2.0);
                glEnd();
                
                amplitude_min_region->setRegion(right_edge_x - range_frame_major_tick_length - pad - lf*label_width,
                                                vertical_range_frame_y - lf*label_height/2.0,
                                                label_width, label_height);
                
                GLuint unit_label = string_renderer->stringToTexture(unit_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  unit_label);
                
                GLfloat zero_height = vertical_range_frame_y + vertical_range_frame_height / 2.0;
                
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,0.0); 
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - label_width,   zero_height + label_height/2.0);
                glTexCoord2d(label_width,0.0);           
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   zero_height + label_height/2.0);
                glTexCoord2d(label_width,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad,   zero_height - label_height/2.0);
                glTexCoord2d(0.0,label_height);
                glVertex2f( right_edge_x - range_frame_major_tick_length - pad - label_width,   zero_height - label_height/2.0);
                glEnd();
                
                
                glPopAttrib();
                
                glDeleteTextures(1, &high_label);
                glDeleteTextures(1, &low_label);
                glDeleteTextures(1, &unit_label);
            }

            // draw the range bars; assumes we're in a whole-view viewport
            void renderHorizontalRangeFrame(){
                
                glColor3f(1.0, 1.0, 1.0);
                
                // draw a sideways "[" shape, starting at the bottom left
                glBegin(GL_LINE_STRIP);
                glVertex2f(horizontal_range_frame_x, horizontal_range_frame_y + horizontal_range_frame_height - range_frame_major_tick_length);
                glVertex2f(horizontal_range_frame_x, horizontal_range_frame_y + horizontal_range_frame_height);
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width, horizontal_range_frame_y + horizontal_range_frame_height);
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width, horizontal_range_frame_y + horizontal_range_frame_height - range_frame_major_tick_length);
                glEnd();
                
                
                // determine appropriate units
                string unit_string;
                float unit_multiplier = 1;
                float test_case = max(fabs(time_range_max_seconds), fabs(time_range_min_seconds));
                if(test_case < 0.1 && test_case >= 0.001){
                    // MILLIVOLTS
                    unit_string = "ms";
                    unit_multiplier = 1000.0;
                } else if(test_case < 0.001 && test_case > 0.000001){
                    // MICROVOLTS
                    unit_string = "us";
                    unit_multiplier = 1000000.0;
                } else {
                    // VOLTS
                    unit_string = "s";
                }
                
                // render some labels
                float label_width, label_height;
                GLfloat pad = 4;
                GLfloat lf = label_scale_factor;
                
                glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
                glDisable (GL_DEPTH_TEST); // ensure text is not remove by depth buffer test.
                glEnable (GL_BLEND); // for text fading
                glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ditto
                glEnable (GL_TEXTURE_RECTANGLE_EXT);	
                
                
                string high_string = (boost::format("%.1f") % (time_range_max_seconds*unit_multiplier)).str();
                GLuint high_label = string_renderer->stringToTexture(high_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  high_label);
                
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,label_height); 
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width - lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad);
                glTexCoord2d(label_width,label_height);           
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width + lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad);
                glTexCoord2d(label_width,0.0);
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width + lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glTexCoord2d(0.0,0.0);
                glVertex2f(horizontal_range_frame_x + horizontal_range_frame_width - lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glEnd();
                
                time_max_region->setRegion(horizontal_range_frame_x + horizontal_range_frame_width - lf*label_width/2.0,
                                           horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad,
                                                label_width, label_height);
                
                
                
                string low_string = (boost::format("%.1f") % (time_range_min_seconds*unit_multiplier)).str();
                GLuint low_label = string_renderer->stringToTexture(low_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  low_label);
                
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,label_height); 
                glVertex2f(horizontal_range_frame_x - lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad);
                glTexCoord2d(label_width,label_height);           
                glVertex2f(horizontal_range_frame_x + lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad);
                glTexCoord2d(label_width,0.0);
                glVertex2f(horizontal_range_frame_x + lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glTexCoord2d(0.0,0.0);
                glVertex2f(horizontal_range_frame_x - lf*label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glEnd();
                
                time_min_region->setRegion(horizontal_range_frame_x - lf*label_width/2.0,
                                           horizontal_range_frame_y + horizontal_range_frame_height - lf*label_height - pad,
                                           label_width, label_height);
                
                
                GLuint unit_label = string_renderer->stringToTexture(unit_string, range_frame_font_size, &label_width, &label_height);
                
                glBindTexture (GL_TEXTURE_RECTANGLE_EXT,  unit_label);
                
                GLfloat zero_pos = horizontal_range_frame_x + horizontal_range_frame_width / 2.0;
                
                glColor4f(1.0,1.0,1.0,1.0);
                glBegin(GL_QUADS);
                glTexCoord2d(0.0,label_height); 
                glVertex2f(zero_pos - label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - label_height - pad);
                glTexCoord2d(label_width, label_height);           
                glVertex2f(zero_pos + label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - label_height - pad);
                glTexCoord2d(label_width,0.0);
                glVertex2f(zero_pos + label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glTexCoord2d(0.0,0.0);
                glVertex2f(zero_pos - label_width/2.0, horizontal_range_frame_y + horizontal_range_frame_height - pad);
                glEnd();
                
                
                glPopAttrib();
                
                glDeleteTextures(1, &high_label);
                glDeleteTextures(1, &low_label);
                glDeleteTextures(1, &unit_label);
                
            }
            
            
    };
        
        
}

#endif
