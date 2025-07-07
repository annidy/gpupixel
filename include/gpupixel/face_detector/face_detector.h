/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#pragma once

#include <vector>
#include "gpupixel/gpupixel_define.h"

namespace mars_vision {
class MarsFaceLandmarker;
}

namespace gpupixel {

class GPUPIXEL_API FaceDetector {
 public:
  static std::shared_ptr<FaceDetector> Create();
  std::vector<float> Detect(const uint8_t* data,
                            int width,
                            int height,
                            int stride,
                            GPUPIXEL_MODE_FMT fmt,
                            GPUPIXEL_FRAME_TYPE type);
  ~FaceDetector();

 private:
  FaceDetector();
#ifdef GPUPIXEL_ENABLE_VNN
  uint32_t vnn_handle_;
  int use_278pts = 0;
#else
  std::shared_ptr<mars_vision::MarsFaceLandmarker> mars_face_detector_;
#endif
};
}  // namespace gpupixel
