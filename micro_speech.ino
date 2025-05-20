#include <Arduino.h>
#include <MicroTFLite.h>

#include "model.h"  // 모델 데이터 포함 (g_model[])
// Contains the model data as a C array (e.g., g_model[])

#include "audio_provider.h"  // 마이크 입력을 처리하는 오디오 입력 관련 함수
// Handles audio input from the microphone

#include "feature_provider.h"  // 오디오에서 특징(MFCC 등)을 추출
// Extracts features (e.g., MFCCs) from audio input

#include "micro_features_micro_model_settings.h"  // 모델 입력 설정 (슬라이스 수, 특성 크기 등)
// Defines model input settings (slice count, feature length, etc.)


//-----------------------------------------------------------------------------------

// 텐서 아레나 크기 설정 / Tensor arena setup
// 모델 실행에 필요한 메모리 공간을 확보
constexpr int tensorArenaSize = 10 * 1024;
alignas(16) byte tensorArena[tensorArenaSize];

// 클래스 수 및 임계값 / Number of classes & detection threshold
// 'yes'와 'no' 두 가지 명령어 분류 / Only two classes
constexpr int kNumClasses = 2;
// 감지로 간주할 최소 점수 / Minimum confidence score to count as detected
constexpr float kThreshold = 0.5f;

// 전역 변수 / Global variables
FeatureProvider* feature_provider = nullptr;
int32_t previous_time = 0;  // 이전 오디오 슬라이스 처리 시간 / Timestamp of last processed slice
int8_t feature_buffer[kFeatureElementCount];  // 특징 버퍼 / Buffer to hold extracted features (int8 quantized)

// ------------------------------------------------------------------------------------------------
// 초기 설정 / setup()
// ------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(4800);
  while (!Serial);  // 시리얼 연결 대기 / Wait for serial port to connect

  // 내장 및 RGB LED 초기화 / Initialize built-in and RGB LEDs
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);  // 빨간색 LED
  pinMode(LEDG, OUTPUT);  // 초록색 LED
  pinMode(LEDB, OUTPUT);  // 파란색 LED

  // LED 꺼짐 상태로 초기화 / Turn off all LEDs (active LOW)
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  // 모델 초기화 / Initialize the TFLite Micro model
  if (!ModelInit(model, tensorArena, tensorArenaSize)) {
    Serial.println("ModelInit failed!");  // 모델 불러오기 실패 시 멈춤
    while (1);
  }

  // 모델 정보 출력 (디버깅용) / Print input/output tensor info for debug
  ModelPrintMetadata();
  ModelPrintTensorQuantizationParams();
  ModelPrintTensorInfo();

  // 특징 추출기 초기화 / Create feature extractor with buffer
  static FeatureProvider fp(kFeatureElementCount, feature_buffer);
  feature_provider = &fp;

  // 오디오 입력 초기화 / Start microphone recording
  if (InitAudioRecording() != kTfLiteOk) {
    Serial.println("Audio init failed!");  // 마이크 문제 시 멈춤
    while (1);
  }

  Serial.println("Setup complete");  // 초기화 완료
}

// ------------------------------------------------------------------------------------------------
// 루프 / loop()
// ------------------------------------------------------------------------------------------------
void loop() {
  // 현재 시간 기준 오디오 슬라이스 계산 / Check how many new audio slices are available
  int32_t current_time = LatestAudioTimestamp();
  int how_many_new_slices = 10;

  // 특징 추출 / Extract MFCC or other features from audio
  if (feature_provider->PopulateFeatureData(
        previous_time, current_time, &how_many_new_slices) != kTfLiteOk) {
    Serial.println("Feature generation failed!");  // 특성 생성 실패 시 중단
    return;
  }

  // 새 슬라이스가 없으면 추론 생략 / Skip if there's no new data
  previous_time += how_many_new_slices * kFeatureSliceStrideMs;
  if (how_many_new_slices == 0) return;

  // 모델 입력 설정 / Copy feature data into model input tensor
  for (int i = 0; i < kFeatureElementCount; ++i) {
    ModelSetInput(feature_buffer[i], i);
  }

  // 추론 실행 / Run inference on the input tensor
  if (!ModelRunInference()) {
    Serial.println("Inference failed!");
    return;
  }

  // 결과 출력: 모델 출력은 두 클래스 (index 1 = "yes", index 0 = "no")
  float yes_score = ModelGetOutput(1);
  float no_score  = ModelGetOutput(0);

  Serial.print("OUT_score [yes_score (1)]: "); Serial.print(yes_score);
  Serial.print(" / OUT_score [no_score (0)]: "); Serial.println(no_score);

  // 결과 판단 및 LED 제어 / Decision logic and LED indication
  if (yes_score > no_score && yes_score > kThreshold) {
    Serial.println("Detected: yes");

    // 초록색 LED ON / Turn on green LED
    digitalWrite(LEDR, HIGH);  // 빨간 OFF
    digitalWrite(LEDG, LOW);   // 초록 ON
    digitalWrite(LEDB, HIGH);  // 파랑 OFF
    delay(800);
  } 
  else if (no_score > yes_score && no_score > kThreshold) {
    Serial.println("Detected: no");

    // 빨간색 LED ON / Turn on red LED
    digitalWrite(LEDR, LOW);   // 빨간 ON
    digitalWrite(LEDG, HIGH);  // 초록 OFF
    digitalWrite(LEDB, HIGH);  // 파랑 OFF
    delay(800);
  } 
  /*else {
    // 아무 명령도 감지되지 않음 / No valid detection
    Serial.println("No command detected.");
  }*/

  // 모든 LED 끄기 / Turn off all LEDs
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}