/***************************************************************************
 * Copyright (C) 2008-2011 by Cameron Wong                                 *
 * name in passport: HUANG GUANNENG                                        *
 * email: hgneng at yahoo.com.cn                                           *
 * website: http://e-guidedog.sf.net                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the Creative Commons GNU GPL.                     *
 *                                                                         *
 * To get a Human-Readable description of this license,                    *
 * please refer to http://creativecommons.org/licenses/GPL/2.0/            *
 *                                                                         *
 * To get Commons Deed Lawyer-Readable description of this license,        *
 * please refer to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html   *
 *                                                                         *
 **************************************************************************/
#ifndef EKHO_H
#define EKHO_H

#include <queue>
#include <pthread.h>
#include <sndfile.h>
#include "config.h"
#include "ekho_dict.h"
#include "sonic.h"

#if HAVE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

#ifdef ENABLE_SOUNDTOUCH
#include "SoundTouch.h"
using namespace soundtouch;
#endif

using namespace std;

namespace ekho {

  typedef struct {
    string text;
    void (*pCallback)(void*);
    void* pCallbackArgs;
  } SpeechOrder;

  enum Command {
    SPEAK,
    SAVEMP3,
    SAVEOGG,
    GETPHONSYMBOLS
  };

  class Ekho {
    public:
      const static int BUFFER_SIZE = 8192;
      const static int MAX_CLIENTS = 100; 
      Dict mDict;
      int mPort;

      static void debug(bool flag = true) {
        mDebug = flag;
        Dict::mDebug = flag;
      };

      Ekho(void);
      Ekho(string voice);

      /* Destructor.
      */
      ~Ekho(void);

      /* Set voice 
       * voice is the name of voice, which is a directory name under
       * ekho-data and should be begun with jyutping-, pinyin- or 
       * hangul-. Cantonese, Mandarin, Korean are alias to jyutping, 
       * pinyin, hangul.
       */
      int setVoice(string voice);
      /* Get current voice */
      string getVoice(void);

      /* Speak text
       * text should be in UTF-8 format
       * it will launch a new thread and return immediately
       */
      int speak(string text,
          void (*pCallback)(void*) = NULL,
          void* pCallbackArgs = NULL);

      typedef int (SynthCallback)(short *pcm, int frames, void *arg);
      /* Synth speech
       * callback will be called time from time when buffer is ready
       */
      int synth(string text, SynthCallback *callback, void *userdata = 0);

      /* no pause is allowed
       * it will return after all sound is played
       */
      int blockSpeak(string text);

      /* play audio file with external player */
      int play(string file);

      /* output text to WAVE file */
      int saveWav(string text, string filename);

      /* output to OGG file */
      int saveOgg(string text, string filename);

      /* output to MP3 file */
      int saveMp3(string text, string filename);

      /* Pause speaking */
      int pause(void);

      /* Resume speaking */
      int resume(void);

      /* Stop speaking */
      int stop(void);

      /* start TTS server */
      int startServer(int port);

      /* request wave from Ekho TTS server */
      int request(string ip, int port, Command cmd, string text, string outfile);

      /* Set tempo delta
       * Parameter:
       *    tempo_delta (-50 .. 100, in percent)
       *    If input out of range, tempo_delta will restore to 0
       */
      void setSpeed(int tempo_delta);
      int getSpeed(void);

      /* Set pitch delta
       * Parameter:
       *    pitch_delta (-100 .. 100, in percent)
       *    If input out of range, pitch_delta will restore to 0
       */
      void setPitch(int pitch_delta);
      int getPitch(void);

      /* Set volume delta
       * Parameter:
       *    volume_delta (-100 .. 100, in percent)
       *    If input out of range, volume_delta will restore to 0
       */
      void setVolume(int volume_delta);
      int getVolume(void);

      /* Set rate delta
       * Parameter:
       *    rate_delta (-50 .. 100, in percent)
       *    If input out of range, rate_delta will restore to 0
       * Return: 0
       */
      void setRate(int rate_delta);
      int getRate(void);

      void setPcmCache(bool b) { mPcmCache = b; }

      string genTempFilename(void);

      sonicStream mSonicStream;
#ifdef ENABLE_SOUNDTOUCH
      SoundTouch pSoundtouch;
#endif

      static void* speechDaemon(void *args);
      static int speakPcm(short *pcm, int frames, void* arg);
      static int writePcm(short *pcm, int frames, void* arg);
      /*
         static int changeSamplerate(const short *source_data,
         long source_len, // len in 8 bits
         int source_rate,
         short *target_data,
         int target_rate);
         */

      /* get PCM, internal use only */
      const char* getPcmFromFestival(string text, int& size);

    private:
      int init(void);
      int initPcm(void);
      int initSound(void);
      int initStream(void);
      int initFestival(void);
      void closeStream(void);
      int outputSpeech(string text);

      static bool mDebug;
      bool mPcmCache;
      int tempoDelta; // -50 .. 100 (%)
      int pitchDelta; // -100 .. 100 (%)
      int volumeDelta; // -100 .. 100 (%)
      int rateDelta; // -50 .. 100 (%)

      bool isRecording;
      bool isPaused;
      bool isStopped;
      bool isEnded;
      bool isSoundInited;
      string player; // "ogg123", "mplayer" or "play"
      SNDFILE *mSndFile;
      queue<SpeechOrder> speechQueue;

      bool isSpeechThreadInited;
      pthread_t speechThread;
#ifdef HAVE_PULSEAUDIO
      pa_simple *stream;
#endif
  };
}
#endif
