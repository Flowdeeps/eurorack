// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Calibration settings.

#include "clouds/cv_scaler.h"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"

#include "clouds/resources.h"

namespace clouds {

using namespace std;

/* static */
CvTransformation CvScaler::transformations_[ADC_CHANNEL_LAST] = {
  // ADC_POSITION_POTENTIOMETER_CV,
  { true, false, 0.05f },
  // ADC_DENSITY_POTENTIOMETER_CV,
  { true, false, 0.01f },
  // ADC_SIZE_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_SIZE_CV,
  { false, true, 0.1f },
  // ADC_PITCH_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_V_OCT_CV,
  { false, false, 1.00f },
  // ADC_WET_POTENTIOMETER,
  { false, false, 0.05f },
  // ADC_WET_CV,
  { false, true, 0.2f },
  //ADC_STEREO_POTENTIOMETER_CV,
  { true, false, 0.01f },
  //ADC_FEEDBACK_POTENTIOMETER_CV,
  { true, false, 0.01f },
  //ADC_REVERB_POTENTIOMETER_CV,
  { true, false, 0.01f },
  // ADC_TEXTURE_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_TEXTURE_CV,
  { false, true, 0.01f }
};

void CvScaler::Init(CalibrationData* calibration_data) {
  adc_.Init();
  gate_input_.Init();
  calibration_data_ = calibration_data;
  fill(&smoothed_adc_value_[0], &smoothed_adc_value_[ADC_CHANNEL_LAST], 0.0f);
  note_ = 0.0f;
  
  fill(&previous_trigger_[0], &previous_trigger_[kAdcLatency], false);
  fill(&previous_gate_[0], &previous_gate_[kAdcLatency], false);
}

void CvScaler::Read(Parameters* parameters) {

  for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
    const CvTransformation& transformation = transformations_[i];
    
    float value = adc_.float_value(i);
    if (transformation.flip) {
      value = 1.0f - value;
    }
    if (transformation.remove_offset) {
      value -= calibration_data_->offset[i];
    }

    smoothed_adc_value_[i] += transformation.filter_coefficient * \
        (value - smoothed_adc_value_[i]);
  }
  
  parameters->position = smoothed_adc_value_[ADC_POSITION_POTENTIOMETER_CV];
  
  float texture = smoothed_adc_value_[ADC_TEXTURE_POTENTIOMETER];
  texture -= smoothed_adc_value_[ADC_TEXTURE_CV] * 2.0f;
  CONSTRAIN(texture, 0.0f, 1.0f);
  parameters->texture = texture;

  parameters->kammerl.slice_selection = smoothed_adc_value_[ADC_TEXTURE_CV];
  CONSTRAIN(parameters->kammerl.slice_selection, 0.0f, 1.0f);
  parameters->kammerl.slice_modulation = smoothed_adc_value_[ADC_TEXTURE_POTENTIOMETER];
  CONSTRAIN(parameters->kammerl.slice_modulation, 0.0f, 1.0f);

  float density = smoothed_adc_value_[ADC_DENSITY_POTENTIOMETER_CV];
  CONSTRAIN(density, 0.0f, 1.0f);
  parameters->density = density;

  parameters->kammerl.size_modulation = density;
  CONSTRAIN(parameters->kammerl.size_modulation, 0.0f, 1.0f);

  parameters->size = smoothed_adc_value_[ADC_SIZE_POTENTIOMETER];
  parameters->size -= smoothed_adc_value_[ADC_SIZE_CV];
  CONSTRAIN(parameters->size, 0.0f, 1.0f);
  
  // reworked for uBurst expanded

  parameters->dry_wet = smoothed_adc_value_[ADC_WET_POTENTIOMETER];
  parameters->dry_wet -= smoothed_adc_value_[ADC_WET_CV];
  CONSTRAIN(parameters->dry_wet, 0.0f, 1.0f);

  float reverb = smoothed_adc_value_[ADC_REVERB_POTENTIOMETER_CV];
  CONSTRAIN(reverb, 0.0f, 1.0f);
  parameters->reverb = reverb;

  float feedback = smoothed_adc_value_[ADC_FEEDBACK_POTENTIOMETER_CV];
  CONSTRAIN(feedback, 0.0f, 1.0f);
  parameters->feedback = feedback;

  float stereo = smoothed_adc_value_[ADC_STEREO_POTENTIOMETER_CV];
  CONSTRAIN(stereo, 0.0f, 1.0f);
  parameters->stereo = stereo;

  parameters->kammerl.probability = parameters->dry_wet;
  parameters->kammerl.clock_divider = stereo;
  parameters->kammerl.pitch_mode = feedback;
  parameters->kammerl.distortion = reverb;

  parameters->pitch = stmlib::Interpolate(
      lut_quantized_pitch,
      smoothed_adc_value_[ADC_PITCH_POTENTIOMETER],
      1024.0f);
  
  float note = calibration_data_->pitch_offset;
  note += smoothed_adc_value_[ADC_V_OCT_CV] * calibration_data_->pitch_scale;
  if (fabs(note - note_) > 0.5f) {
    note_ = note;
  } else {
    ONE_POLE(note_, note, 0.2f)
  }
  
  parameters->pitch += note_;
  CONSTRAIN(parameters->pitch, -48.0f, 48.0f);

  parameters->kammerl.pitch = smoothed_adc_value_[ADC_PITCH_POTENTIOMETER];
  parameters->kammerl.pitch += smoothed_adc_value_[ADC_V_OCT_CV] - 0.5f;
  CONSTRAIN(parameters->kammerl.pitch, 0.0f, 1.0f);
  
  gate_input_.Read();
  if (gate_input_.freeze_rising_edge()) {
    parameters->freeze = true;
  } else if (gate_input_.freeze_falling_edge()) {
    parameters->freeze = false;
  }
  
  parameters->trigger = previous_trigger_[0];
  parameters->gate = previous_gate_[0];
  for (int i = 0; i < kAdcLatency - 1; ++i) {
    previous_trigger_[i] = previous_trigger_[i + 1];
    previous_gate_[i] = previous_gate_[i + 1];
  }
  previous_trigger_[kAdcLatency - 1] = gate_input_.trigger_rising_edge();
  previous_gate_[kAdcLatency - 1] = gate_input_.gate();
  
  adc_.Convert();
}

}  // namespace clouds
