/*
 *  LocklessQueue.h
 *  SpikeAudioUnit
 *
 *  Created by David Cox on 8/27/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */
#ifndef LOCK_FREE_QUEUE_H_
#define LOCK_FREE_QUEUE_H_
 
#include <boost/atomic.hpp>

using namespace boost;

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        Node( T val ) : value(val), next(NULL) { }
        T value;
        Node* next;
    };
    Node* first;			   // for producer only
    atomic<Node*> divider, last;         // shared

public:
    LockFreeQueue() {
        first = divider = last =
        new Node( T() );           // add dummy separator
    }
    ~LockFreeQueue() {
        while( first != NULL ) {   // release the list
            Node* tmp = first;
            first = tmp->next;
            delete tmp;
        }
    }
    
    void Produce( const T& t ) {
        last->next = new Node(t);  	// add the new item
        last = last->next;		// publish it
        while( first != divider ) {	// trim unused nodes
            Node* tmp = first;
            first = first->next;
            delete tmp;
        }
    }
    
    bool Consume( T& result ) {
        if( divider != last ) {      	// if queue is nonempty
            result = divider->next->value; 	// C: copy it back
            divider = divider->next; 	// D: publish that we took it
            return true;          	// and report success
        }
        return false;           	// else report empty
    }
};

#endif
