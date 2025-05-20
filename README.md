< 설치 라이브러리 >  

Sketch > Include Library > Manage Libraries...

1. Arduino_TensorFlowLite

2.PDM (Nano 33 BLE Sense에서 마이크 입력용)

-----------------------------------------------------------------

< 파일 구조 >

my_project/ micro_speech.ino             <-- 메인 스케치 파일.
          /model.h                       <-- TFLite 모델의 C 배열 
          /audio_provider.h / .cpp       <-- 마이크 입력 처리
          /feature_provider.h / .cpp     <-- 특징 추출 관리
          /feature_generator.h / .cpp          
          /micro_features_micro_model_settings.h / .cpp  <-- 모델 특징 세팅 관련 설정.
