#include <Arduino.h>
#include <MicroTFLite.h>

#include "model.h"
#include "audio_provider.h"
#include "feature_provider.h"
#include "micro_features_micro_model_settings.h"

// Tensor arena
constexpr int tensorArenaSize = 10 * 1024;
alignas(16) byte tensorArena[tensorArenaSize];

// Classification settings
constexpr int kNumClasses = 2;
constexpr float kThreshold = 0.5f;

// Globals
FeatureProvider* feature_provider = nullptr;
int32_t previous_time = 0;
int8_t feature_buffer[kFeatureElementCount];

bool is_active = false;  // 음성 인식 상태 (토글)
bool last_enter_pressed = false;  // 중복 입력 방지용

void turnOffAllLEDs() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}

void setup() {
  Serial.begin(4800);
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  turnOffAllLEDs();

  // 초기 대기 상태에서 파란색 LED 점등
  digitalWrite(LEDB, LOW);

  if (!ModelInit(model, tensorArena, tensorArenaSize)) {
    Serial.println("ModelInit failed!");
    while (1);
  }

  ModelPrintMetadata();
  ModelPrintTensorQuantizationParams();
  ModelPrintTensorInfo();

  static FeatureProvider fp(kFeatureElementCount, feature_buffer);
  feature_provider = &fp;

  if (InitAudioRecording() != kTfLiteOk) {
    Serial.println("Audio init failed!");
    while (1);
  }

  Serial.println("Setup complete");
  Serial.println("Press ENTER to toggle recognition");
}

void loop() {
  // 엔터 입력 감지
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (!last_enter_pressed) {
        is_active = !is_active;  // 상태 토글
        last_enter_pressed = true;

        if (is_active) {
          Serial.println("Voice recognition STARTED.");
          turnOffAllLEDs();  // 녹색 또는 빨강 LED를 사용할 준비
        } else {
          Serial.println("Voice recognition STOPPED. (Waiting mode)");
          turnOffAllLEDs();
          digitalWrite(LEDB, LOW);  // 파란 LED ON
        }
      }
    }
  } else {
    last_enter_pressed = false;
  }

  if (!is_active) {
    delay(100);
    return;
  }

  // 음성 인식 루틴
  int32_t current_time = LatestAudioTimestamp();
  int how_many_new_slices = 10;

  if (feature_provider->PopulateFeatureData(
        previous_time, current_time, &how_many_new_slices) != kTfLiteOk) {
    Serial.println("Feature generation failed!");
    return;
  }

  previous_time += how_many_new_slices * kFeatureSliceStrideMs;
  if (how_many_new_slices == 0) return;

  for (int i = 0; i < kFeatureElementCount; ++i) {
    ModelSetInput(feature_buffer[i], i);
  }

  if (!ModelRunInference()) {
    Serial.println("Inference failed!");
    return;
  }

  float yes_score = ModelGetOutput(1);
  float no_score  = ModelGetOutput(0);

  Serial.print("OUT_score [yes]: "); Serial.print(yes_score);
  Serial.print(" / [no]: "); Serial.println(no_score);

  if (yes_score > no_score && yes_score > kThreshold) {
    Serial.println("Detected: yes");
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, LOW);  // 초록 ON
    digitalWrite(LEDB, HIGH);
    delay(800);
  } else if (no_score > yes_score && no_score > kThreshold) {
    Serial.println("Detected: no");
    digitalWrite(LEDR, LOW);  // 빨강 ON
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    delay(800);
  }

  // 대기 없이 LED 꺼주기
  turnOffAllLEDs();
}
