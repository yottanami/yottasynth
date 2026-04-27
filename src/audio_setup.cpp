#include "audio_setup.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

/* // GUItool: begin automatically generated code */
/* AudioSynthWaveform       lead_waveform2;      //xy=67.5714340209961,63.14285659790039 */
/* AudioSynthWaveform       mid_waveform1;      //xy=68,155 */
/* AudioSynthWaveform       lead_waveform1;      //xy=68.57141876220703,24.857135772705078 */
/* AudioSynthWaveform       mid_waveform2;      //xy=70,197 */
/* AudioSynthWaveform       bass_waveform1;      //xy=76,289 */
/* AudioSynthWaveform       bass_waveform2;      //xy=77,327 */
/* AudioSynthNoisePink      mid_pink;          //xy=78.28571319580078,235.85714721679688 */
/* AudioSynthNoisePink      lead_pink;          //xy=80.71426773071289,103.14284896850586 */
/* AudioSynthNoisePink      bass_pink;          //xy=83,365.4285583496094 */
/* AudioSynthSimpleDrum     drum;          //xy=86,434 */
/* AudioMixer4              mid_mixer;         //xy=240.57142639160156,204.42857360839844 */
/* AudioMixer4              lead_mixer;         //xy=247.57142639160156,69.14284896850586 */
/* AudioMixer4              bass_mixer;         //xy=255.85714721679688,324.1428527832031 */
/* AudioFilterStateVariable drum_filter;        //xy=362,423 */
/* AudioFilterStateVariable mid_filter;        //xy=400,201 */
/* AudioFilterStateVariable bass_filter;        //xy=408,322 */
/* AudioFilterStateVariable lead_filter;        //xy=411.9999694824219,70.71428298950195 */
/* AudioMixer4              drum_mixer;         //xy=525,427 */
/* AudioMixer4              mid_filter_mixer;         //xy=560,207 */
/* AudioMixer4              bass_filter_mixer;         //xy=575,323 */
/* AudioMixer4              lead_filter_mixer;         //xy=587.4285278320312,76.14287185668945 */
/* AudioEffectEnvelope      drum_envelope;      //xy=714,427 */
/* AudioEffectEnvelope      mid_envelope;      //xy=749.4285888671875,209.28570556640625 */
/* AudioEffectEnvelope      bass_envelope;      //xy=756.9999389648438,323.4285888671875 */
/* AudioEffectEnvelope      lead_envelope;      //xy=772.5714111328125,76.0000114440918 */
/* AudioMixer4              global_mixer;         //xy=965,305.14288330078125 */
/* AudioFilterStateVariable global_filter;        //xy=1137,308 */
/* AudioMixer4              global_filter_mixer;         //xy=1325,314 */
/* AudioOutputI2S           i2s1;           //xy=1512.428466796875,315.28570556640625 */
/* AudioConnection          patchCord1(lead_waveform2, 0, lead_mixer, 1); */
/* AudioConnection          patchCord2(mid_waveform1, 0, mid_mixer, 0); */
/* AudioConnection          patchCord3(lead_waveform1, 0, lead_mixer, 0); */
/* AudioConnection          patchCord4(mid_waveform2, 0, mid_mixer, 1); */
/* AudioConnection          patchCord5(bass_waveform1, 0, bass_mixer, 0); */
/* AudioConnection          patchCord6(bass_waveform2, 0, bass_mixer, 1); */
/* AudioConnection          patchCord7(mid_pink, 0, mid_mixer, 2); */
/* AudioConnection          patchCord8(lead_pink, 0, lead_mixer, 2); */
/* AudioConnection          patchCord9(bass_pink, 0, bass_mixer, 2); */
/* AudioConnection          patchCord10(drum, 0, drum_filter, 0); */
/* AudioConnection          patchCord11(drum, 0, drum_filter, 1); */
/* AudioConnection          patchCord12(mid_mixer, 0, mid_filter, 0); */
/* AudioConnection          patchCord13(mid_mixer, 0, mid_filter, 1); */
/* AudioConnection          patchCord14(lead_mixer, 0, lead_filter, 0); */
/* AudioConnection          patchCord15(lead_mixer, 0, lead_filter, 1); */
/* AudioConnection          patchCord16(bass_mixer, 0, bass_filter, 0); */
/* AudioConnection          patchCord17(bass_mixer, 0, bass_filter, 1); */
/* AudioConnection          patchCord18(drum_filter, 0, drum_mixer, 0); */
/* AudioConnection          patchCord19(drum_filter, 1, drum_mixer, 1); */
/* AudioConnection          patchCord20(drum_filter, 2, drum_mixer, 2); */
/* AudioConnection          patchCord21(mid_filter, 0, mid_filter_mixer, 0); */
/* AudioConnection          patchCord22(mid_filter, 1, mid_filter_mixer, 1); */
/* AudioConnection          patchCord23(mid_filter, 2, mid_filter_mixer, 2); */
/* AudioConnection          patchCord24(bass_filter, 0, bass_filter_mixer, 0); */
/* AudioConnection          patchCord25(bass_filter, 1, bass_filter_mixer, 1); */
/* AudioConnection          patchCord26(bass_filter, 2, bass_filter_mixer, 2); */
/* AudioConnection          patchCord27(lead_filter, 0, lead_filter_mixer, 0); */
/* AudioConnection          patchCord28(lead_filter, 1, lead_filter_mixer, 1); */
/* AudioConnection          patchCord29(lead_filter, 2, lead_filter_mixer, 2); */
/* AudioConnection          patchCord30(drum_mixer, drum_envelope); */
/* AudioConnection          patchCord31(mid_filter_mixer, mid_envelope); */
/* AudioConnection          patchCord32(bass_filter_mixer, bass_envelope); */
/* AudioConnection          patchCord33(lead_filter_mixer, lead_envelope); */
/* AudioConnection          patchCord34(drum_envelope, 0, global_mixer, 3); */
/* AudioConnection          patchCord35(mid_envelope, 0, global_mixer, 1); */
/* AudioConnection          patchCord36(bass_envelope, 0, global_mixer, 2); */
/* AudioConnection          patchCord37(lead_envelope, 0, global_mixer, 0); */
/* AudioConnection          patchCord38(global_mixer, 0, global_filter, 0); */
/* AudioConnection          patchCord39(global_mixer, 0, global_filter, 1); */
/* AudioConnection          patchCord40(global_filter, 0, global_filter_mixer, 0); */
/* AudioConnection          patchCord41(global_filter, 1, global_filter_mixer, 1); */
/* AudioConnection          patchCord42(global_filter, 2, global_filter_mixer, 2); */
/* AudioConnection          patchCord43(global_filter_mixer, 0, i2s1, 0); */
/* AudioConnection          patchCord44(global_filter_mixer, 0, i2s1, 1); */
/* AudioControlSGTL5000     sgtl5000_1;     //xy=1409.5714111328125,425.1428527832031 */
/* // GUItool: end automatically generated code */


// Define audio objects
AudioSynthWaveform    lead_waveform1;
AudioSynthWaveform    lead_waveform2;
AudioSynthWaveform    mid_waveform1;
AudioSynthWaveform    mid_waveform2;
AudioSynthWaveform    bass_waveform1;
AudioSynthWaveform    bass_waveform2;
AudioSynthNoisePink   mid_pink;
AudioSynthNoisePink   lead_pink;
AudioSynthNoisePink   bass_pink;
AudioSynthSimpleDrum  drum;
AudioMixer4           mid_mixer;
AudioMixer4           lead_mixer;
AudioMixer4           bass_mixer;
AudioFilterStateVariable drum_filter;
AudioFilterStateVariable mid_filter;
AudioFilterStateVariable bass_filter;
AudioFilterStateVariable lead_filter;
AudioMixer4           drum_mixer;
AudioMixer4           mid_filter_mixer;
AudioMixer4           bass_filter_mixer;
AudioMixer4           lead_filter_mixer;
AudioEffectEnvelope   drum_envelope;
AudioEffectEnvelope   mid_envelope;
AudioEffectEnvelope   bass_envelope;
AudioEffectEnvelope   lead_envelope;
AudioMixer4           global_mixer;
AudioFilterStateVariable global_filter;
AudioMixer4           global_filter_mixer;
AudioOutputI2S        i2s1;
AudioControlSGTL5000  sgtl5000_1;


// Define audio connections
AudioConnection          patchCord1(lead_waveform2, 0, lead_mixer, 1);
AudioConnection          patchCord2(mid_waveform1, 0, mid_mixer, 0);
AudioConnection          patchCord3(lead_waveform1, 0, lead_mixer, 0);
AudioConnection          patchCord4(mid_waveform2, 0, mid_mixer, 1);
AudioConnection          patchCord5(bass_waveform1, 0, bass_mixer, 0);
AudioConnection          patchCord6(bass_waveform2, 0, bass_mixer, 1);
AudioConnection          patchCord7(mid_pink, 0, mid_mixer, 2);
AudioConnection          patchCord8(lead_pink, 0, lead_mixer, 2);
AudioConnection          patchCord9(bass_pink, 0, bass_mixer, 2);
AudioConnection          patchCord10(drum, 0, drum_filter, 0);
AudioConnection          patchCord11(drum, 0, drum_filter, 1);
AudioConnection          patchCord12(mid_mixer, 0, mid_filter, 0);
AudioConnection          patchCord13(mid_mixer, 0, mid_filter, 1);
AudioConnection          patchCord14(lead_mixer, 0, lead_filter, 0);
AudioConnection          patchCord15(lead_mixer, 0, lead_filter, 1);
AudioConnection          patchCord16(bass_mixer, 0, bass_filter, 0);
AudioConnection          patchCord17(bass_mixer, 0, bass_filter, 1);
AudioConnection          patchCord18(drum_filter, 0, drum_mixer, 0);
AudioConnection          patchCord19(drum_filter, 1, drum_mixer, 1);
AudioConnection          patchCord20(drum_filter, 2, drum_mixer, 2);
AudioConnection          patchCord21(mid_filter, 0, mid_filter_mixer, 0);
AudioConnection          patchCord22(mid_filter, 1, mid_filter_mixer, 1);
AudioConnection          patchCord23(mid_filter, 2, mid_filter_mixer, 2);
AudioConnection          patchCord24(bass_filter, 0, bass_filter_mixer, 0);
AudioConnection          patchCord25(bass_filter, 1, bass_filter_mixer, 1);
AudioConnection          patchCord26(bass_filter, 2, bass_filter_mixer, 2);
AudioConnection          patchCord27(lead_filter, 0, lead_filter_mixer, 0);
AudioConnection          patchCord28(lead_filter, 1, lead_filter_mixer, 1);
AudioConnection          patchCord29(lead_filter, 2, lead_filter_mixer, 2);
AudioConnection          patchCord30(drum_mixer, drum_envelope);
AudioConnection          patchCord31(mid_filter_mixer, mid_envelope);
AudioConnection          patchCord32(bass_filter_mixer, bass_envelope);
AudioConnection          patchCord33(lead_filter_mixer, lead_envelope);
AudioConnection          patchCord34(drum_envelope, 0, global_mixer, 3);
AudioConnection          patchCord35(mid_envelope, 0, global_mixer, 1);
AudioConnection          patchCord36(bass_envelope, 0, global_mixer, 2);
AudioConnection          patchCord37(lead_envelope, 0, global_mixer, 0);
AudioConnection          patchCord38(global_mixer, 0, global_filter, 0);
AudioConnection          patchCord39(global_mixer, 0, global_filter, 1);
AudioConnection          patchCord40(global_filter, 0, global_filter_mixer, 0);
AudioConnection          patchCord41(global_filter, 1, global_filter_mixer, 1);
AudioConnection          patchCord42(global_filter, 2, global_filter_mixer, 2);
AudioConnection          patchCord43(global_filter_mixer, 0, i2s1, 0);
AudioConnection          patchCord44(global_filter_mixer, 0, i2s1, 1);


bool setupAudio() {
  AudioMemory(30);

  const bool enabled = sgtl5000_1.enable();
  if (!enabled) {
    return false;
  }

  sgtl5000_1.volume(0.70f);
  sgtl5000_1.dacVolume(1.0f);
  sgtl5000_1.lineOutLevel(29);
  return true;
}
