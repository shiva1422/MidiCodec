//
// Created by shivaaz on 11/14/22.
//

/*
_________________________________________________________________________________________________________________________________________________________________
|   (POWER)      - OCTAVE +           - TEMPO +                                                                                                                 |
|_______________________________________________________________________________________________________________________________________________________________|                                                                   |
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|******|   |   |***|   |   |******|******|   |   |***|   |   |***|   |   |******|
|******|___|___|***|___|___|******|******|___|___|***|___|___|***|___|___|******|******|___|___|***|___|___|******|******|___|___|***|___|___|***|___|___|******|
|**********|***********|**********|**********|***********|***********|**********|**********|***********|**********|**********|***********|***********|**********|
|**********|***********|**********|**********|***********|***********|**********|**********|***********|**********|**********|***********|***********|**********|
|**********|***********|**********|**********|***********|***********|**********|**********|***********|**********|**********|***********|***********|**********|
|**********|***********|**********|**********|***********|***********|**********|**********|***********|**********|**********|***********|***********|**********|
``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````

 */

#ifndef DIGIBANDPIANO_PIANOCORE_H
#define DIGIBANDPIANO_PIANOCORE_H

#include "Commons.h"
#include <limits.h>
#include "Logger/KSLog.h"

//if this changes all code using this must be modified accordingly
#define PIANO_MAX_KEY_COUNT 88

//TODO into namespace KS


enum EKeyState{

    KEYSTATE_OFF = 0,KEYSTATE_ON
};

enum EKeyType{

    BASE/*white Key*/ , SHARP , FLAT
};

//88 keys that map in order from left-right on piano from 0 - 87(A0 - C8) //TODO EKeyName to ENoteName/Index/Number
enum EKeyName{

    A0 = 0 , As0 , Bb0 = As0 , B0 ,
    C1 , Cs1 , Db1 = Cs1, D1 , Ds1 , Eb1 = Ds1 , E1 , F1 , Fs1 , Gb1 = Fs1 , G1, Gs1 , Ab1 = Gs1 , A1 , As1 , Bb1 = As1 , B1 ,
    C2 , Cs2 , Db2 = Cs2, D2 , Ds2 , Eb2 = Ds2 , E2 , F2 , Fs2 , Gb2 = Fs2 , G2, Gs2 , Ab2 = Gs2 , A2 , As2 , Bb2 = As2 , B2 ,
    C3 , Cs3 , Db3 = Cs3, D3 , Ds3 , Eb3 = Ds3 , E3 , F3 , Fs3 , Gb3 = Fs3 , G3, Gs3 , Ab3 = Gs3 , A3 , As3 , Bb3 = As3 , B3 ,
    C4 , Cs4 , Db4 = Cs4, D4 , Ds4 , Eb4 = Ds4 , E4 , F4 , Fs4 , Gb4 = Fs4 , G4, Gs4 , Ab4 = Gs4 , A4 , As4 , Bb4 = As4 , B4 ,
    C5 , Cs5 , Db5 = Cs5, D5 , Ds5 , Eb5 = Ds5 , E5 , F5 , Fs5 , Gb5 = Fs5 , G5, Gs5 , Ab5 = Gs5 , A5 , As5 , Bb5 = As5 , B5 ,
    C6 , Cs6 , Db6 = Cs6, D6 , Ds6 , Eb6 = Ds6 , E6 , F6 , Fs6 , Gb6 = Fs6 , G6, Gs6 , Ab6 = Gs6 , A6 , As6 , Bb6 = As6 , B6 ,
    C7 , Cs7 , Db7 = Cs7, D7 , Ds7 , Eb7 = Ds7 , E7 , F7 , Fs7 , Gb7 = Fs7 , G7, Gs7 , Ab7 = Gs7 , A7 , As7 , Bb7 = As7 , B7 ,
    C8 , INVALID = INT_MIN
};

//KeyNames base white keys
enum EKeyNameBase{

    A = 0, B , C , D , E , F , G
};

struct KeyStateInfo{

public:

    KeyStateInfo(EKeyName name,EKeyState state)
    {
        keyName = name;
        state = keyState;
    }

   KSFORCEINLINE void set(const EKeyName &name,const EKeyState &state)
    {
        this->keyName = name;
        this->keyState = state;
    }

    EKeyName keyName;
    EKeyState keyState;

};

KSFORCEINLINE EKeyName getKeyNameFromBase(EKeyNameBase baseName)
{
    switch (baseName)
    {
        case EKeyNameBase::A : return EKeyName::A1;

        case EKeyNameBase::B : return EKeyName::B1;

        case EKeyNameBase::C : return EKeyName::C1;

        case EKeyNameBase::D : return EKeyName::D1;

        case EKeyNameBase::E : return EKeyName::E1;

        case EKeyNameBase::F : return EKeyName::F1;

        case EKeyNameBase::G : return EKeyName::G1;

        default:
            KSLOGE("PianoCore","getKeyNameFromBase invalid basename");
            return EKeyName::INVALID;
    }
}


KSFORCEINLINE EKeyName getKeyName(EKeyNameBase baseName,int octaveNum,EKeyType keyType)
{
   int name = getKeyNameFromBase(baseName);
   name += ((octaveNum-1)*12);//since above return octave 1 not 0 and 12 keys per octave;

   if(keyType == EKeyType::FLAT)
       name--;
   else if(keyType == EKeyType::SHARP)
       name++;

   return static_cast<EKeyName>(name);
}


/*
 const float keyFreqs[] =
        {
                  //  25.95654,
                    27.50000 ,29.13524 ,30.86771 ,32.70320 ,34.64783 ,36.70810 ,38.89087 ,41.20344 ,43.65353 ,46.24930 ,48.99943 ,51.91309 ,
                    55.00000 ,58.27047 ,61.73541 ,65.40639 ,69.29566 ,73.41619 ,77.78175 ,82.40689 ,87.30706 ,92.49861 ,97.99886 ,103.8262 ,
                    110.0000 ,116.5409 ,123.4708 ,130.8128 ,138.5913 ,146.8324 ,155.5635 ,164.8138 ,174.6141 ,184.9972 ,195.9977 ,207.6523 ,
                    220.0000 ,233.0819 ,246.9417 ,261.6256 ,277.1826 ,293.6648 ,311.1270 ,329.6276 ,349.2282 ,369.9944 ,391.9954 ,415.3047 ,
                    440.0000 ,466.1638 ,493.8833 ,523.2511 ,554.3653 ,587.3295 ,622.2540 ,659.2551 ,698.4565 ,739.9888 ,783.9909 ,830.6094 ,
                    880.0000 ,932.3275 ,987.7666 ,1046.502 ,1108.731 ,1174.659 ,1244.508 ,1318.510 ,1396.913 ,1479.978 ,1567.982 ,1661.219 ,
                    11760760.000 ,1864.655 ,1975.533 ,2093.005 ,2217.461 ,2349.318 ,2489.016 ,2637.020 ,2793.826 ,2959.955 ,3135.963 ,3322.438 ,
                    3520.000 ,3729.310 ,3951.066 ,4186.009,
        };

        *
        */




#endif //DIGIBANDPIANO_PIANOCORE_H
