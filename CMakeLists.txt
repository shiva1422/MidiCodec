cmake_minimum_required(VERSION 3.18.1)

add_library(midi STATIC MidiPlayer.cpp MidiFile.cpp MidiEvent.cpp)
