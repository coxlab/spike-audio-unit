/*
 *  MIDIEndpoint.h
 *  SpikeAudioUnit
 *
 *  Created by David Cox on 6/15/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef MIDI_ENDPOINT_H_
#define MIDI_ENDPOINT_H_

#include <CoreMIDI/MIDIServices.h>
#include <string>

using namespace std;

class MIDIEndpoint {

    protected:
    
    
        MIDIClientRef midi_client;    
        MIDIPortRef midi_port;
        MIDIEndpointRef *midi_source;

        string client_name, client_port, source_name;
    
    public:
    
        MIDIEndpoint(string _client_name, string _client_port, string _source_name){
    
    
            client_name = _client_name;
            client_port = _client_port;
            source_name = _source_name;
            
            midi_source = (MIDIEndpointRef *)malloc(sizeof(MIDIEndpointRef));
            
            OSStatus err;
            
            CFStringRef client_name_cf = CFStringCreateWithCString(NULL, client_name.c_str(), kCFStringEncodingUTF8);
            
            err = MIDIClientCreate(client_name_cf, NULL, NULL, &midi_client);
            if (err != noErr) {
                fprintf(stderr, "MIDIClientCreate had problems\n");
                exit(1);
            }
            
            
            CFStringRef port_name_cf = CFStringCreateWithCString(NULL, client_port.c_str(), kCFStringEncodingUTF8);
            
            err = MIDIOutputPortCreate(midi_client, port_name_cf, &midi_port);
            if (err != noErr) {
                fprintf(stderr, "MIDIOutputPortCreate had problems\n");
                exit(1);
            }
            
            CFStringRef source_name_cf = CFStringCreateWithCString(NULL, source_name.c_str(), kCFStringEncodingUTF8);
            
            err = MIDISourceCreate(midi_client, source_name_cf, midi_source);
            if (err != noErr) {
                fprintf(stderr, "MIDISourceCreate had problems\n");
                exit(1);
            }
            
        }


        void sendPip(){
            return sendMessage(0x90, 0x00, 0x7F);
        }

        void sendMessage(unsigned char command, unsigned char data1, unsigned char data2){
    
            MIDIPacketList pktlist;
            
            MIDIPacket p;
            
            Byte data[3];
            
            MIDIPacket *p_head = MIDIPacketListInit(&pktlist);
            
            data[0] = command; // Control change
            data[1] = data1;   // Modulation
            data[2] = data2; // Value
            
            MIDIPacketListAdd( &pktlist, sizeof(p), p_head, 0, 3, data);
            
            MIDIReceived(*midi_source, &pktlist);
            
        }
};

#endif
