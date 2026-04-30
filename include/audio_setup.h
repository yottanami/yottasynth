#ifndef AUDIO_SETUP_H
#define AUDIO_SETUP_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "app_state.h"

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
/* AudioMixer4              fx_echo_input_mixer;         //xy=1325,160 */
/* AudioMixer4              fx_echo_feedback_mixer;         //xy=1327,230 */
/* AudioMixer4              fx_reverb_input_mixer;         //xy=1328,400 */
/* AudioEffectDelay         fx_delay;         //xy=1492,193 */
/* AudioEffectFreeverb      fx_reverb;         //xy=1498,407 */
/* AudioAmplifier           fx_drive_input;         //xy=1500,493 */
/* AudioEffectWaveshaper    fx_drive_shaper;         //xy=1658,492 */
/* AudioEffectBitcrusher    fx_drive_crusher;         //xy=1816,494 */
/* AudioMixer4              final_left_mixer;         //xy=2001,231 */
/* AudioMixer4              final_right_mixer;         //xy=2002,355 */
/* AudioOutputI2S           i2s1;           //xy=2165.428466796875,294.28570556640625 */
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
/* AudioConnection          patchCord43(global_filter_mixer, 0, fx_echo_input_mixer, 0); */
/* AudioConnection          patchCord44(global_filter_mixer, 0, fx_reverb_input_mixer, 0); */
/* AudioConnection          patchCord45(global_filter_mixer, 0, fx_drive_input, 0); */
/* AudioConnection          patchCord46(global_filter_mixer, 0, final_left_mixer, 0); */
/* AudioConnection          patchCord47(global_filter_mixer, 0, final_right_mixer, 0); */
/* AudioConnection          patchCord48(fx_echo_feedback_mixer, 0, fx_echo_input_mixer, 1); */
/* AudioConnection          patchCord49(fx_echo_input_mixer, fx_delay); */
/* AudioConnection          patchCord50(fx_delay, 0, final_left_mixer, 1); */
/* AudioConnection          patchCord51(fx_delay, 1, final_right_mixer, 1); */
/* AudioConnection          patchCord52(fx_delay, 0, fx_echo_feedback_mixer, 0); */
/* AudioConnection          patchCord53(fx_delay, 1, fx_echo_feedback_mixer, 1); */
/* AudioConnection          patchCord54(fx_delay, 2, fx_reverb_input_mixer, 1); */
/* AudioConnection          patchCord55(fx_reverb_input_mixer, fx_reverb); */
/* AudioConnection          patchCord56(fx_reverb, 0, final_left_mixer, 2); */
/* AudioConnection          patchCord57(fx_reverb, 0, final_right_mixer, 2); */
/* AudioConnection          patchCord58(fx_drive_input, fx_drive_shaper); */
/* AudioConnection          patchCord59(fx_drive_shaper, fx_drive_crusher); */
/* AudioConnection          patchCord60(fx_drive_crusher, 0, final_left_mixer, 3); */
/* AudioConnection          patchCord61(fx_drive_crusher, 0, final_right_mixer, 3); */
/* AudioConnection          patchCord62(final_left_mixer, 0, i2s1, 0); */
/* AudioConnection          patchCord63(final_right_mixer, 0, i2s1, 1); */
/* AudioControlSGTL5000     sgtl5000_1;     //xy=1409.5714111328125,425.1428527832031 */
/* // GUItool: end automatically generated code */


// Declare audio objects
extern AudioSynthWaveform      lead_waveform1;
extern AudioSynthWaveform      lead_waveform2;
extern AudioSynthWaveform      mid_waveform1;
extern AudioSynthWaveform      mid_waveform2;
extern AudioSynthWaveform      bass_waveform1;
extern AudioSynthWaveform      bass_waveform2;
extern AudioSynthNoisePink     mid_pink;
extern AudioSynthNoisePink     lead_pink;
extern AudioSynthNoisePink     bass_pink;
extern AudioSynthSimpleDrum    drum;
extern AudioMixer4             mid_mixer;
extern AudioMixer4             lead_mixer;
extern AudioMixer4             bass_mixer;
extern AudioFilterStateVariable drum_filter;
extern AudioFilterStateVariable mid_filter;
extern AudioFilterStateVariable bass_filter;
extern AudioFilterStateVariable lead_filter;
extern AudioMixer4             drum_mixer;
extern AudioMixer4             mid_filter_mixer;
extern AudioMixer4             bass_filter_mixer;
extern AudioMixer4             lead_filter_mixer;
extern AudioEffectEnvelope     drum_envelope;
extern AudioEffectEnvelope     mid_envelope;
extern AudioEffectEnvelope     bass_envelope;
extern AudioEffectEnvelope     lead_envelope;
extern AudioMixer4             global_mixer;
extern AudioFilterStateVariable global_filter;
extern AudioMixer4             global_filter_mixer;
extern AudioMixer4             fx_echo_input_mixer;
extern AudioMixer4             fx_echo_feedback_mixer;
extern AudioMixer4             fx_reverb_input_mixer;
extern AudioEffectDelay        fx_delay;
extern AudioEffectFreeverb     fx_reverb;
extern AudioAmplifier          fx_drive_input;
extern AudioEffectWaveshaper   fx_drive_shaper;
extern AudioEffectBitcrusher   fx_drive_crusher;
extern AudioMixer4             final_left_mixer;
extern AudioMixer4             final_right_mixer;
extern AudioOutputI2S          i2s1;
extern AudioControlSGTL5000    sgtl5000_1;


// Declare audio connections
extern AudioConnection          patchCord1;
extern AudioConnection          patchCord2;
extern AudioConnection          patchCord3;
extern AudioConnection          patchCord4;
extern AudioConnection          patchCord5;
extern AudioConnection          patchCord6;
extern AudioConnection          patchCord7;
extern AudioConnection          patchCord8;
extern AudioConnection          patchCord9;
extern AudioConnection          patchCord10;
extern AudioConnection          patchCord11;
extern AudioConnection          patchCord12;
extern AudioConnection          patchCord13;
extern AudioConnection          patchCord14;
extern AudioConnection          patchCord15;
extern AudioConnection          patchCord16;
extern AudioConnection          patchCord17;
extern AudioConnection          patchCord18;
extern AudioConnection          patchCord19;
extern AudioConnection          patchCord20;
extern AudioConnection          patchCord21;
extern AudioConnection          patchCord22;
extern AudioConnection          patchCord23;
extern AudioConnection          patchCord24;
extern AudioConnection          patchCord25;
extern AudioConnection          patchCord26;
extern AudioConnection          patchCord27;
extern AudioConnection          patchCord28;
extern AudioConnection          patchCord29;
extern AudioConnection          patchCord30;
extern AudioConnection          patchCord31;
extern AudioConnection          patchCord32;
extern AudioConnection          patchCord33;
extern AudioConnection          patchCord34;
extern AudioConnection          patchCord35;
extern AudioConnection          patchCord36;
extern AudioConnection          patchCord37;
extern AudioConnection          patchCord38;
extern AudioConnection          patchCord39;
extern AudioConnection          patchCord40;
extern AudioConnection          patchCord41;
extern AudioConnection          patchCord42;
extern AudioConnection          patchCord43;
extern AudioConnection          patchCord44;
extern AudioConnection          patchCord45;
extern AudioConnection          patchCord46;
extern AudioConnection          patchCord47;
extern AudioConnection          patchCord48;
extern AudioConnection          patchCord49;
extern AudioConnection          patchCord50;
extern AudioConnection          patchCord51;
extern AudioConnection          patchCord52;
extern AudioConnection          patchCord53;
extern AudioConnection          patchCord54;
extern AudioConnection          patchCord55;
extern AudioConnection          patchCord56;
extern AudioConnection          patchCord57;
extern AudioConnection          patchCord58;
extern AudioConnection          patchCord59;
extern AudioConnection          patchCord60;
extern AudioConnection          patchCord61;
extern AudioConnection          patchCord62;
extern AudioConnection          patchCord63;

bool setupAudio();
void setOutputVolume(float volume);
void applyFxState(const FxState &fx);

#endif
