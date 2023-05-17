//
// Created by shivaaz on 1/16/23.
//

#ifndef KALASOFT_MIDIFILE_H
#define KALASOFT_MIDIFILE_H

#include <KSIO/IKSStream.h>
#include <string>
#include "vector"
#include "MidiEvent.h"

//TODO get the desciptions right for EMidiFormat
namespace ks {
    enum EMidiFormat {
        SMT = 0, /*Single multi channel Track*/
        MT, /*one or multiple tracks (single sequence)*/
        MS,/*MultiSequence/clips? one or more sequentially independent single-track patterns //means a single track multi clip */
    };

    class MidiTrackFileInfo {

    public:

        double tickRate = 0.0;//ticks per quarter note.
        int32_t sizeBytes = 0;
        int64_t fileOffset = 0;
        int32_t tempo = 120;
        std::vector<ks::MidiEvent> events;

        void print() const;

    };

    class MidiFile {

    public:

        MidiFile(){}
        //TODO Always include filename with stream;
        MidiFile(const IKSStream *input);

        std::vector<MidiTrackFileInfo> trackInfos;
        double tickRate = 0.0;
    private:

        bool decode(IKSStream *input, std::string filename = "NotSet");

        void printFormat() const;

    private:

        std::string fileName;
        EMidiFormat midiFormat;
        bool bOpened = false;
        bool bUsingFPS = false;
        uint trackCnt = 0;


    };
}


#endif //KALASOFT_MIDIFILE_H
