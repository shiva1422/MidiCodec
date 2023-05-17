//
// Created by shivaaz on 1/16/23.
//

#include <Logger/KSLog.h>
#include "MidiFile.h"
#include "Commons.h"
#include "MidiEvent.h"
#define LOGTAG "MidiFile"

using namespace ks;
MidiFile::MidiFile(const IKSStream *input)
{
    bOpened = decode(const_cast<IKSStream *>(input));
    assert(bOpened);
}

//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
bool MidiFile::decode(IKSStream *input,std::string filename)
{
    //TODO clear previous if exists

    bool bLittleEndian = isLittleEndian();//TODO can be known at compile time has a macro #ifdef __LITTLE_ENDIAN__
    KSLog::debug(LOGTAG,"isLitteIndian %d",bLittleEndian);
    bOpened  = false;
    if(!input)
    {
        KSLog::error(LOGTAG,"input file not open");
        return bOpened;
    }
    //Midi file  consists of chucks each chunk with type of 4 chars and size(4 bytes) of chunk(exclude type and size bytes (8)) ;
    //chunks can be header chunks or track chunks.headerChunks have type "MThd",track chunks have type "MTrk"
    uint8_t chunkId[4];
    uint8_t chunkSize[4];
    //TODO is >0 fine? depends on what IKStream.read() should return later verify
    if(input->read(chunkId, 4) > 0 && input->read(chunkSize, 4) > 0)
    {
        /*
         * Header chunk :
         * type("MThd") 4 bytes ,data size(bigEndian) 4 bytes, where data size = 6,
         * data (size bytes) = midi format(0,1,2) 2 bytes,track count 2 bytes ,division 2 bytes
         *
         */
        if(isLittleEndian())//TODO use macro
            swapEndian32(chunkSize);

        if(chunkId[0] == 'M' && chunkId[1] == 'T' && chunkId[2] == 'h' && chunkId[3] == 'd' && (*(int32_t *)chunkSize) == 6)//maybe >6 also ok>
        {
            KSLog::debug(LOGTAG, "header read");

            //get EMidiFormat from chunkData
            int16_t headData;
            input->read(&headData, 2);
            if (bLittleEndian)
            {
                swapEndian16((uint8_t *) (&headData));
            }
            if (!(headData < 0 || headData > 2))
            {
                midiFormat = static_cast<EMidiFormat>(headData);
            }
            else
            {
                KSLog::error(LOGTAG, "invalid Midi format");
                return false;
            }

            //read trackCount
            input->read(&headData,2);
            if (bLittleEndian)
            {
                swapEndian16((uint8_t *) (&headData));
            }
            trackCnt = headData;
            if(midiFormat == EMidiFormat::SMT && trackCnt != 1)
            {
                KSLog::error(LOGTAG,"error 0 format can have only one track");
                return false;
            }
            //TODO resize trackCnt if holding  trackdata hear like MIDITrack?

            //Read the beatDivision(ticks per quarter note)//MSB  0 -> ticks per Quarternote else negative SMPTE format
            input->read(&headData,2);
            if (bLittleEndian)
            {
                swapEndian16((uint8_t *) (&headData));
            }
            if(headData & 0x8000)
            {
                //negative SMPTE format
                bUsingFPS = true;
                //TODO check -ve parsing
                int8_t fps = -((headData & 0xFF00)>>8);//bits 14-8 give -ve fps (starting count from 0 th bit)
                tickRate = (double )fps;
                if(tickRate == 29.0){tickRate = 29.97;}//does it still valid?
                tickRate *= (headData & 0x00FF);//0-7 give ticks per frame;so tickRate = fps*ticksPerFrame;

            }
            else
            {
                bUsingFPS = false;//using tick per(beat) quarter note
                tickRate = headData;
            }

            printFormat();

        }
        else
        {
            KSLog::error(LOGTAG,"%s might not be midifile",filename.c_str());
            return false;
        }

        //Read TrackData

         /*
          * Track Chunk> = <chunk type><length><MTrk event><MTrk envts>.....all events
          * where TrackEvent = <delta-time><event>
          * <event> = <MIDI event> | <sysex event> | <meta-event>
          */
         trackInfos.resize(trackCnt);
        for(uint trkNo = 0 ; trkNo < trackCnt ; ++trkNo)
        {
            KSLog::verbose(LOGTAG,"reading midi track %u",trkNo);
           if( input->read(chunkId,4) == 4 )//TODO
           {
               if(chunkId[0] == 'M' && chunkId[1] == 'T' && chunkId[2] == 'r' && chunkId[3] == 'k' )
               {
                   input->read(chunkSize,4);
                   if(bLittleEndian){swapEndian32(chunkSize);}
                   //chunkSize may not be accurate some times.but track end can be determined with a meta event
                   trackInfos[trkNo].sizeBytes = *((int32_t *)chunkSize);//is this correct No?
                   trackInfos[trkNo].fileOffset = input->getCurrentPosition();
                   trackInfos[trkNo].print();



                   //Read Midi Events

                   //since events contain delta time (amount of time to wait before this event after previous event,we shall)
                   //we shall use absoulute ticks(seqTime) which starts from 0 and increments.
                   int64_t seqInTime = 0;//still is @tickRate

                   uint8_t lastStatus = 0;
                   ks::MidiEvent event;
                   int64_t cnt = 0;
                   do {
                       event  = ks::MidiEvent::getNextEventFromStream(input,lastStatus);
                       lastStatus = event.getStatus();
                       cnt++;
                       trackInfos[trkNo].events.push_back(event);//do a better way.
                       if(event.isTempoEvent())
                       {
                           KSLog::debug(LOGTAG,"tempo %d",event.getTempo());
                           trackInfos[trkNo].tempo = event.getTempo();
                       }
                   } while (!event.isEndOfTrack());


                   KSLog::debug(LOGTAG,"MidiEvent count %ld",cnt);

               }
               else
               {
                   assert(false);
                   //TODO not valid track data,check possible errors and handle
               }
           }
            else
            {
                KSLog::error(LOGTAG,"error reading track");
                return false;
            }

        }
        bOpened = true;

    }
    else
    {
        KSLog::error(LOGTAG,"Midi file read error");
    }


    printFormat();
    this->fileName = filename;
    return bOpened;
}

void MidiFile::printFormat() const
{
    KSLog::verbose(LOGTAG,"format %d trackCnt %u tick format %s - tickRate %lf",midiFormat,trackCnt,tickRate,bUsingFPS ? "FFPS":"tickPerQuarterNote",tickRate);
}

void MidiTrackFileInfo::print() const
{
    KSLog::verbose(LOGTAG,"track byte size %d ,track file offset %ld",sizeBytes,fileOffset);
}
