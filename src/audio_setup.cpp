#include "audio_setup.h"

#include <Audio.h>
#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <string.h>

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
AudioMixer4           fx_echo_input_mixer;
AudioMixer4           fx_echo_feedback_mixer;
AudioMixer4           fx_reverb_input_mixer;
DMAMEM AudioEffectDelay      fx_delay;
DMAMEM AudioEffectFreeverb   fx_reverb;
AudioAmplifier        fx_drive_input;
AudioEffectWaveshaper fx_drive_shaper;
AudioEffectBitcrusher fx_drive_crusher;
AudioMixer4           final_left_mixer;
AudioMixer4           final_right_mixer;
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
AudioConnection          patchCord43(global_filter_mixer, 0, fx_echo_input_mixer, 0);
AudioConnection          patchCord44(global_filter_mixer, 0, fx_reverb_input_mixer, 0);
AudioConnection          patchCord45(global_filter_mixer, 0, fx_drive_input, 0);
AudioConnection          patchCord46(global_filter_mixer, 0, final_left_mixer, 0);
AudioConnection          patchCord47(global_filter_mixer, 0, final_right_mixer, 0);
AudioConnection          patchCord48(fx_echo_feedback_mixer, 0, fx_echo_input_mixer, 1);
AudioConnection          patchCord49(fx_echo_input_mixer, fx_delay);
AudioConnection          patchCord50(fx_delay, 0, final_left_mixer, 1);
AudioConnection          patchCord51(fx_delay, 1, final_right_mixer, 1);
AudioConnection          patchCord52(fx_delay, 0, fx_echo_feedback_mixer, 0);
AudioConnection          patchCord53(fx_delay, 1, fx_echo_feedback_mixer, 1);
AudioConnection          patchCord54(fx_delay, 2, fx_reverb_input_mixer, 1);
AudioConnection          patchCord55(fx_reverb_input_mixer, fx_reverb);
AudioConnection          patchCord56(fx_reverb, 0, final_left_mixer, 2);
AudioConnection          patchCord57(fx_reverb, 0, final_right_mixer, 2);
AudioConnection          patchCord58(fx_drive_input, fx_drive_shaper);
AudioConnection          patchCord59(fx_drive_shaper, fx_drive_crusher);
AudioConnection          patchCord60(fx_drive_crusher, 0, final_left_mixer, 3);
AudioConnection          patchCord61(fx_drive_crusher, 0, final_right_mixer, 3);
AudioConnection          patchCord62(final_left_mixer, 0, i2s1, 0);
AudioConnection          patchCord63(final_right_mixer, 0, i2s1, 1);


namespace {
bool g_audio_codec_ready = false;
bool g_fx_ready = false;
DMAMEM float g_drive_curve[257];
FxState g_last_fx;
bool g_has_last_fx = false;

float clampUnit(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

float msFromNormalized(float normalized, float min_ms, float max_ms) {
  return min_ms + (clampUnit(normalized) * (max_ms - min_ms));
}

void setMixerRow(AudioMixer4 &mixer, float g0, float g1, float g2, float g3) {
  mixer.gain(0, g0);
  mixer.gain(1, g1);
  mixer.gain(2, g2);
  mixer.gain(3, g3);
}

void buildDriveCurve() {
  for (int index = 0; index < 257; ++index) {
    const float x = (static_cast<float>(index) / 256.0f) * 2.0f - 1.0f;
    g_drive_curve[index] = tanhf(x * 2.8f) / tanhf(2.8f);
  }
}
}

bool setupAudio() {
  AudioMemory(220);

  g_audio_codec_ready = sgtl5000_1.enable();
  if (!g_audio_codec_ready) {
    return false;
  }

  sgtl5000_1.volume(0.50f);
  sgtl5000_1.dacVolume(1.0f);
  sgtl5000_1.lineOutLevel(29);

  buildDriveCurve();
  fx_drive_shaper.shape(g_drive_curve, 257);

  setMixerRow(global_filter_mixer, 0.75f, 0.18f, 0.07f, 0.0f);
  setMixerRow(fx_echo_input_mixer, 1.0f, 0.0f, 0.0f, 0.0f);
  setMixerRow(fx_echo_feedback_mixer, 0.0f, 0.0f, 0.0f, 0.0f);
  setMixerRow(fx_reverb_input_mixer, 1.0f, 0.0f, 0.0f, 0.0f);
  setMixerRow(final_left_mixer, 0.9f, 0.0f, 0.0f, 0.0f);
  setMixerRow(final_right_mixer, 0.9f, 0.0f, 0.0f, 0.0f);

  fx_delay.delay(0, 120.0f);
  fx_delay.delay(1, 180.0f);
  fx_delay.delay(2, 16.0f);
  for (uint8_t channel = 3; channel < 8; ++channel) {
    fx_delay.disable(channel);
  }

  fx_reverb.roomsize(0.55f);
  fx_reverb.damping(0.40f);
  fx_drive_input.gain(1.0f);
  fx_drive_crusher.bits(16);
  fx_drive_crusher.sampleRate(22050.0f);
  g_fx_ready = true;
  return true;
}

void setOutputVolume(float volume) {
  if (!g_audio_codec_ready) {
    return;
  }

  sgtl5000_1.volume(clampUnit(volume));
}

void applyFxState(const FxState &fx) {
  if (!g_fx_ready) {
    return;
  }

  if (g_has_last_fx && memcmp(&g_last_fx, &fx, sizeof(FxState)) == 0) {
    return;
  }
  g_last_fx = fx;
  g_has_last_fx = true;

  if (!fx.enabled) {
    setMixerRow(fx_echo_feedback_mixer, 0.0f, 0.0f, 0.0f, 0.0f);
    setMixerRow(fx_reverb_input_mixer, 1.0f, 0.0f, 0.0f, 0.0f);
    setMixerRow(final_left_mixer, 0.90f, 0.0f, 0.0f, 0.0f);
    setMixerRow(final_right_mixer, 0.90f, 0.0f, 0.0f, 0.0f);
    fx_drive_input.gain(1.0f);
    fx_drive_crusher.bits(16);
    fx_drive_crusher.sampleRate(22050.0f);
    return;
  }

  switch (fx.mode) {
    case FxMode::ECHO: {
      const float mix = clampUnit(fx.echo.mix);
      const float time_ms = msFromNormalized(fx.echo.time, 50.0f, 360.0f);
      const float ratio = 0.45f + (clampUnit(fx.echo.ratio) * 0.50f);
      const float smear = clampUnit(fx.echo.smear);
      const float feedback = clampUnit(fx.echo.feedback);

      fx_delay.delay(0, time_ms);
      fx_delay.delay(1, time_ms * ratio);
      fx_delay.delay(2, 12.0f + smear * 110.0f);

      setMixerRow(fx_echo_feedback_mixer, feedback * 0.38f, feedback * 0.24f, 0.0f, 0.0f);
      setMixerRow(fx_reverb_input_mixer, 1.0f, smear * 0.80f, 0.0f, 0.0f);
      fx_reverb.roomsize(0.28f + smear * 0.52f);
      fx_reverb.damping(0.25f + (1.0f - clampUnit(fx.echo.ratio)) * 0.55f);

      const float dry = 0.86f - mix * 0.18f;
      setMixerRow(final_left_mixer, dry, mix * 0.70f, mix * smear * 0.32f, 0.0f);
      setMixerRow(final_right_mixer, dry, mix * (0.45f + (1.0f - ratio) * 0.35f),
                  mix * smear * 0.36f, 0.0f);
      fx_drive_input.gain(1.0f);
      fx_drive_crusher.bits(16);
      fx_drive_crusher.sampleRate(22050.0f);
      break;
    }
    case FxMode::REVERB: {
      const float mix = clampUnit(fx.reverb.mix);
      const float size = clampUnit(fx.reverb.size);
      const float damping = clampUnit(fx.reverb.damping);
      const float predelay_ms = msFromNormalized(fx.reverb.predelay, 8.0f, 150.0f);
      const float tone = clampUnit(fx.reverb.tone);

      fx_delay.delay(2, predelay_ms);
      setMixerRow(fx_echo_feedback_mixer, 0.0f, 0.0f, 0.0f, 0.0f);
      setMixerRow(fx_reverb_input_mixer, 1.0f - tone * 0.45f, 0.75f + tone * 0.20f, 0.0f, 0.0f);
      fx_reverb.roomsize(0.22f + size * 0.74f);
      fx_reverb.damping(0.10f + damping * 0.85f);

      const float dry = 0.92f - mix * 0.30f;
      const float wet_left = mix * (0.68f + tone * 0.10f);
      const float wet_right = mix * (0.60f + tone * 0.18f);
      setMixerRow(final_left_mixer, dry, 0.0f, wet_left, 0.0f);
      setMixerRow(final_right_mixer, dry, 0.0f, wet_right, 0.0f);
      fx_drive_input.gain(1.0f);
      fx_drive_crusher.bits(16);
      fx_drive_crusher.sampleRate(22050.0f);
      break;
    }
    case FxMode::DRIVE: {
      const float mix = clampUnit(fx.drive.mix);
      const float drive = clampUnit(fx.drive.drive);
      const float tone = clampUnit(fx.drive.tone);
      const float crush = clampUnit(fx.drive.crush);
      const float level = clampUnit(fx.drive.level);

      setMixerRow(fx_echo_feedback_mixer, 0.0f, 0.0f, 0.0f, 0.0f);
      setMixerRow(fx_reverb_input_mixer, 1.0f, 0.0f, 0.0f, 0.0f);
      fx_drive_input.gain(1.0f + drive * 13.0f);
      fx_drive_crusher.bits(static_cast<uint8_t>(16 - roundf(crush * 10.0f)));
      fx_drive_crusher.sampleRate(4500.0f + tone * 18000.0f);

      const float dry = 0.92f - mix * 0.55f;
      const float wet = mix * (0.35f + level * 0.70f);
      setMixerRow(final_left_mixer, dry, 0.0f, 0.0f, wet);
      setMixerRow(final_right_mixer, dry, 0.0f, 0.0f, wet);
      break;
    }
    default:
      break;
  }
}
