< 설치 라이브러리 >  

Sketch > Include Library > Manage Libraries...

1. Arduino_TensorFlowLite

2.PDM (Nano 33 BLE Sense에서 마이크 입력용)

-----------------------------------------------------------------

< 파일 구조 >

my_project/
├── my_voice_project.ino          <-- 메인 스케치 파일
├── model.h                       <-- TFLite 모델의 C 배열 헤더
├── model.cc                      <-- TFLite 모델의 C 배열 소스
├── audio_provider.h / .cpp       <-- 마이크 입력 처리
├── feature_provider.h / .cpp     <-- 특징 추출 관리
├── model_functions.h / .cpp      <-- 모델 실행 함수들 (ModelInit, RunInference 등)
├── micro_features_micro_model_settings.h  <-- 특징 관련 설정
