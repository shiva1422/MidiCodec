//
// Created by shivaaz on 2/4/23.
//

#ifndef DIGIBANDPIANO_MIDIEVENT_H
#define DIGIBANDPIANO_MIDIEVENT_H

#include <Commons.h>
#include "string"
#include "../Piano/PianoCore.h"
namespace ks{

    //TODO move enums to MidiCore
    enum EMidiEventType{
        MIDIEVENT_UNKNOWN = -1,
        MIDIEVENT_STANDARD_MIDI,
        MIDIEVENT_SYS_EX,MIDIEVENT_META,

        MIDIEVENT_NOTE_OFF,
        MIDIEVENT_NOTE_ON,
        MIDIEVENT_AFTERTOUCH /*POLYPHONIC KEY PRESSURE*/,
        MIDIEVENT_CONTROL_CHANGE,
        MIDIEVENT_PROGRAM_CHANGE,
        MIDIEVENT_CHANNEL_PRESSURE/*AferTouch*/,
        MIDIEVENT_PITCH_WHEEL,
    };


    enum EMidiMetaEventType{
        //https://www.mixagesoftware.com/en/midikit/help/HTML/meta_events.html

        MIDIMETAEVENT_SEQ_NUM = 0X00,
        MIDIMETAEVENT_TEXT = 0X01,
        MIDIMETAEVENT_COPYRIGHT = 0X02/*occurs at delta time 0 in the first track*/,
        MIDIMETAEVENT_TRACK_NAME = 0x03/* seq/track name*  occurs delta time 0*/,
        MIDIMETAEVENT_INSTRUMENT_NAME = 0X04,
        MIDIMETAEVENT_LYRIC = 0X05 ,
        MIDIMETAEVENT_MARKER = 0X06 ,
        MIDIMETAEVENT_CUE_POINT = 0X07 ,
        MIDIMETAEVENT_PROGRAM_NAME = 0X08,
        MIDIMETAEVENT_DEVICE_NAME = 0X09/*PORT NAME */,

        //below two seems not used anymore TODO checkk
        MIDIMETAEVENT_CHANNEL_PREFIX = 0X20/* TODO CHECK NOT FOUND IN SOME?LATEST*/,
        MIDIMETAEVENT_MIDI_PORT = 0X21/*OPTIONAL*/,

        MIDIMETAEVENT_END_OF_TRACK = 0X2F/*mandatory at the end of each track*/,
        MIDIMETAEVENT_SET_TEMPO = 0X51/* US/quarter note can occur any where but generally in the first track*/,
        MIDIMETAEVENT_SMPTE_OFFSET = 0X54/*at the beginning of a track and in the first track of files with MIDI format type 1*/,
        MIDIMETAEVENT_TIME_SIGNATURE = 0X58,
        MIDIMETAEVENT_KEY_SIGNATURE = 0X59,
        MIDIMETAEVENT_SEQUENCER_SPECIFIC= 0X7F,
        MIDIMETAEVENT_UNKNOWN = 0XFF/*TODO can also be reset when comes from midiport but unkown meta in midi file only?*/,

        //NON STANDARD CAN OCCUR AT TIME 0 FIRST TRACK IN THE SEQ
        MIDIMETAEVENT_META_TAG = 0X4B,

    };


    class MidiEvent{

    public:

        /**
         *@brief Read  next  MidiTrack Event from input stream ::
         *
         * @brief #MTrkEvent = #delta-time + #event
         * where #delta-time is the amount of time before the #event
         * where #event = #MIDI event | #sysex event | #meta-event
         *
         * @param stream   :   input stream open and ready(current position after track head chunk) to read events.
         * @param runningStatus  : status of  the previous event , many consecutive msgs might have same(running)
         * status in such cases status might not be present in the #stream for some msgs,so it should be known from the previous event.
         */
        static MidiEvent getNextEventFromStream(IKSStream *stream, uint8_t runningStatus);

        /**
         *
         * @return event type based on the status byte of the midi event.make sure #status is set before this call;
         */
        EMidiEventType getType();

        /**
         * @return return  if end of track event based on Event type is meta and  Metatype is EOT;
         */
        KSFORCEINLINE bool isEndOfTrack(){return getType()== MIDIEVENT_META && metaType == MIDIMETAEVENT_END_OF_TRACK;};

        /**
         *get status byte of the event;
         */

        KSFORCEINLINE uint8_t getStatus(){return status;}


        KeyStateInfo getKeyState();

        KSFORCEINLINE int32_t getEventDeltaTicks(){return deltaTime;}//TODO retruning int32_t below change to int32;

        KSFORCEINLINE bool isTempoEvent(){ return getType()== MIDIEVENT_META && metaType == MIDIMETAEVENT_SET_TEMPO; }

        KSFORCEINLINE int32_t getTempo(){return tempo;}
    private:

        bool readMeta(IKSStream *stream);
        bool readSysEX(IKSStream *stream);
        bool readMIDI(IKSStream *stream);

    private:

        //Time to wait(numTicks) for processing this event after last event .in TickRate.
        int64_t deltaTime;//TODO deltaTick Would be accurate name and also 32 bit should be enough why increase mem if use 32 needed just do it no need any checks.

        uint8_t status;
        //midi msg has  status followd by 1||2 data bytes
        uint8_t d1;
        uint8_t d2;

        uint8_t metaType;
        std::string metaData ;

        //https://www.midi.org/forum/4452-calculate-absolute-time-from-ppq-and-ticks
        /*
        * To convert beats per minute to a Tempo value, take the quotient from dividing 60,000,000 by the beats per minute.
        */
        uint32_t tempo;// = 500000;//microsecs per(beat) quarterNote?//default 500000 = 120 bpm



    };

}



#endif //DIGIBANDPIANO_MIDIEVENT_H
