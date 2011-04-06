#!/bin/bash
protoc --proto_path=./ --python_out=./ ../spike_wave.proto
