//
// Created by shivaaz on 2/4/23.
//

#include <KSIO/IKSStream.h>
#include <Logger/KSLog.h>
#include "MidiEvent.h"
#include "Commons.h"
#define LOGTAG "MidiEvent"

//TODO all stream->readCheck should return requestn bytes so instead > 0 use exact count;
using namespace ks;
MidiEvent MidiEvent::getNextEventFromStream(IKSStream *stream, uint8_t runningStatus)
{
    MidiEvent event;

    /*
     * read deltaTime :
     * which is a variable length value(VLV).where all bytes except last one will have MSB set.
     * the largest possible value is of VLV is 0x0FFFFFFF so should fit into 32 signed int.
     */
    uint64_t dTime = 0;
    uint8_t vlvByte = 0 ;
    //TODO check can overread happen when reading byte(no) also while encoding midi need min two bytes for delta time
    do
    {
        if(stream->read(&vlvByte,sizeof(uint8_t)) > 0)
        {
            dTime = (dTime << 7 ) | (vlvByte & 0x7F);
        }
        else
        {
            KSLog::error(LOGTAG,"read delta time - stream error");//shouldn't happen in a good midifile
            break;//return  =0 ev
        }
    }while(vlvByte & 0x80);//shoudn't go more than 4/5 iterations in general.as max value is  0x0FFFFFFF
    event.deltaTime = dTime;//(UINT TO INT CONVERSION should be okay as dTime wont be that long also convert to int32 is okay)


    /*
     * Read  event : https://ccrma.stanford.edu/~craig/14q/midifile/MidiFileFormat.html
     * <event> = <MIDI event>| <sysex event> | <meta-event>
     *  event starts with a status byte followed by one/two data bytes.if no status byte means same as previous event status(running status)
     *  <MIDI event> = midi channel msgs such as note on and note off.
     */

    if(stream->read(&vlvByte,sizeof(uint8_t)) > 0)
    {
       // if highest bit is set its status byte else data byte
       if(vlvByte & 0X80)
       {
           event.status = vlvByte;
       }
       else
       {//data byte so use a(running status) same status as previous event.
           stream->seek(-1  ,SEEK_CUR);//read all data bytes below so unread this in stream.
           event.status = runningStatus;
       }
    }
    else
    {
        KSLog::error(LOGTAG,"stream error status");
        //return event without msg?
    }

    //different event have different parsing.so check event type from  the  status byte and parse acccordigly
    bool bRead = false;
    switch (event.getType())
    {
        case MIDIEVENT_SYS_EX:
           // KSLog::debug(LOGTAG,"readSYSEX");
            bRead = event.readSysEX(stream);
            break;
        case MIDIEVENT_META:
          //  KSLog::debug(LOGTAG,"read meta");
            bRead = event.readMeta(stream);
            break;
        default:
           // KSLog::debug(LOGTAG,"read default");
            bRead = event.readMIDI(stream);
    }
    assert(bRead);

    return event;
}

EMidiEventType MidiEvent::getType()
{
    //system ex event F0 < status < FF
    if(status > 0xEF && status < 0XFF)
    {
        return MIDIEVENT_SYS_EX;
    }
    if(status == 0XFF)
    {
        return MIDIEVENT_META;
    }
   /* if (status <  0x80)
    {
    //handled below
        return MIDIEVENT_UNKNOWN;
    }*/

    //status byte ssssnnnn - ssss contains status ,nnnn contains channel number for MIDI

    uint8_t MSN = status >> 4;//ssss;

    //TODO Convert below to ENUM MIDIMESSAGE TYPE:?
    switch(MSN)
    {
        case 0x8:return MIDIEVENT_NOTE_OFF;//ssss = 1000
        case 0X9:return MIDIEVENT_NOTE_ON;//ssss = 1001
        case 0xA:return MIDIEVENT_AFTERTOUCH;//1010
        case 0xB:return MIDIEVENT_CONTROL_CHANGE;//1011
        case 0xC:return MIDIEVENT_PROGRAM_CHANGE;//1100
        case 0xD:return MIDIEVENT_CHANNEL_PRESSURE;//1101
        case 0xE:return MIDIEVENT_PITCH_WHEEL;//1110
        default:return MIDIEVENT_UNKNOWN;//???
    }

}

bool MidiEvent::readMIDI(IKSStream *stream)
{
    //at most 2 bytes for midi channel msgs.
    d1 = d2 =  0;
    switch (getType())
    {
        case MIDIEVENT_NOTE_OFF:
        case MIDIEVENT_NOTE_ON:
        case MIDIEVENT_AFTERTOUCH:
        case MIDIEVENT_CONTROL_CHANGE:
        case MIDIEVENT_PITCH_WHEEL:
            //2 data bytes for above events.
            if(!(stream->read(&d1,sizeof(uint8_t)) > 0 && stream->read(&d2,sizeof(uint8_t)) > 0))return false;
            break;

        case MIDIEVENT_CHANNEL_PRESSURE:
        case MIDIEVENT_PROGRAM_CHANGE:
            //1 byte for above events;
            if(!(stream->read(&d1,sizeof(uint8_t) > 0)))return false;
            break;
        default:
            KSLog::error(LOGTAG,"readMIDI type unregistered");
            return false;

    }
    return true;
}

bool MidiEvent::readSysEX(IKSStream *stream)
{
    //The sysEx consists of status byte ,read already followed by length(VLV) of bytes that follow the length itself
    //ends with F7

    uint64_t length = 0;
    uint8_t vlvByte = 0 ;
    //TODO check can overread happen when reading byte(no) also while encoding midi need min two bytes for delta time
    do
    {
        if(stream->read(&vlvByte,sizeof(uint8_t)) > 0)
        {
            length = (length << 7 ) | (vlvByte & 0x7F);
        }
        else
        {
            KSLog::error(LOGTAG,"read delta time - stream error");//shouldn't happen in a good midifile
            return false;
        }
    }while(vlvByte & 0x80);


    //TODO store this metal information for later if required to save;for now ignoring
    char data[length + 1];//TODO dynamic pass
    data[length] = '\0';
    if(!(stream->read(data,length) > 0))
    {
        KSLog::error(LOGTAG,"read sysex stream failed");
        return false;
    }
    //F7 inside the buffer?

    KSLog::debug(LOGTAG,"the sysex data %s",data);
    return true;
}

bool MidiEvent::readMeta(IKSStream *stream)
{
    //
    //FF(this->status) <metaType> <length> <bytes>

    if(!(stream->read(&metaType,sizeof(uint8_t)) == 1))
    {
        KSLog::error(LOGTAG,"stream readMeta failed");
        return false;
    }

    uint64_t length = 0;
    uint8_t vlvByte = 0 ;
    do
    {
        if(stream->read(&vlvByte,sizeof(uint8_t)) > 0)
        {
            length = (length << 7 ) | (vlvByte & 0x7F);
        }
        else
        {
            KSLog::error(LOGTAG,"read meta length- stream error");//shouldn't happen in a good midifile
            return false;
        }

    }while(vlvByte & 0x80);

    char *data = new char[length +1];
    data[length] = '\0';

    if(!(stream->read(data,length) == length))
    {
        KSLog::error(LOGTAG,"read metaBuffer failed lenght %lu",length);
        delete[] data;
        return false;
    }

    KSLog::debug(LOGTAG,"meta data %s",data);
    switch(metaType)
    {
        case MIDIMETAEVENT_TEXT:
        case MIDIMETAEVENT_COPYRIGHT:
        case MIDIMETAEVENT_TRACK_NAME:
        case MIDIMETAEVENT_INSTRUMENT_NAME:
        case MIDIMETAEVENT_LYRIC:
        case MIDIMETAEVENT_MARKER:
        case MIDIMETAEVENT_CUE_POINT:
        case MIDIMETAEVENT_PROGRAM_NAME:
        case MIDIMETAEVENT_DEVICE_NAME:
            metaData = std::string(data);
            break;
        case  MIDIMETAEVENT_SET_TEMPO:
        { //stored in 3 bytes;microseconds per quarter note
            if (length < 3) {
                KSLog::error(LOGTAG, "midi parse error set tempo");
                delete[] data;
                return false;
            }
            // tempo = 0;
            uint32_t b0 = static_cast<uint8_t>(data[0]);
            uint32_t b1 = static_cast<uint8_t>(data[1]);
            uint32_t b2 = static_cast<uint8_t>(data[2]);
            tempo = (b0 << 16) | (b1 << 8) | b2;//TODO verify
        }
            break;

        //TODO store below events if need to saveback
        case MIDIMETAEVENT_SEQ_NUM:
        case MIDIMETAEVENT_CHANNEL_PREFIX:
        case MIDIMETAEVENT_MIDI_PORT:
        case MIDIMETAEVENT_SMPTE_OFFSET:
        case MIDIMETAEVENT_END_OF_TRACK:
        case MIDIMETAEVENT_TIME_SIGNATURE:
        case MIDIMETAEVENT_KEY_SIGNATURE:
        case MIDIMETAEVENT_SEQUENCER_SPECIFIC:
        case MIDIMETAEVENT_UNKNOWN://TODO read desciption at declaratiion
        case MIDIMETAEVENT_META_TAG:
            break;
        default:
            KSLog::error(LOGTAG,"midi meta data unregistered");
    }

    delete[] data;
    return true;
}

KeyStateInfo MidiEvent::getKeyState()
{
    EMidiEventType event = getType();
    KeyStateInfo keyState(EKeyName::INVALID,KEYSTATE_OFF);
    if((event == MIDIEVENT_NOTE_ON) || (event == MIDIEVENT_NOTE_OFF))
    {
        //midi d1 = 0-127 i.e c(-1) to g#(9)
        int k = ((int)d1 - 21);//cuz TODO TODO because we now start EKEYNAME startWith A0 =0 which is 21 in midi
        if(k>-1)
        {
            keyState.set((EKeyName)k,event == MIDIEVENT_NOTE_ON ? KEYSTATE_ON : KEYSTATE_OFF);
        }
        else
        {
            KSLog::error(LOGTAG,"Please implement : the current midi PianoCore key name doesn't support below a0 :");
        }
    }
    return keyState;
}



