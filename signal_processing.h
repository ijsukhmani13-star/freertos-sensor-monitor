#pragma once
// Hardware-independent signal processing used by the processing task.
// Kept free of Arduino/FreeRTOS headers so it can be unit-tested on a PC.

#include <stddef.h>
#include <stdint.h>

namespace sp {

// Fixed-size moving average (no dynamic allocation, O(1) per sample).
template <size_t N>
class MovingAverage {
 public:
  static_assert(N > 0, "window must be > 0");

  void reset() {
    sum_ = 0.0f;
    count_ = 0;
    index_ = 0;
    for (size_t i = 0; i < N; ++i) buf_[i] = 0.0f;
  }

  float add(float sample) {
    if (count_ < N) {
      ++count_;
    } else {
      sum_ -= buf_[index_];
    }
    buf_[index_] = sample;
    sum_ += sample;
    index_ = (index_ + 1) % N;
    return value();
  }

  float value() const { return count_ ? sum_ / static_cast<float>(count_) : 0.0f; }
  size_t count() const { return count_; }
  bool full() const { return count_ == N; }

 private:
  float buf_[N] = {};
  float sum_ = 0.0f;
  size_t count_ = 0;
  size_t index_ = 0;
};

// Threshold with hysteresis: turns ON at >= setPoint,
// turns OFF only when value drops to <= setPoint - hysteresis.
// Prevents alarm "chatter" when a value hovers near the limit.
class HysteresisThreshold {
 public:
  HysteresisThreshold(float setPoint, float hysteresis)
      : setPoint_(setPoint), hysteresis_(hysteresis < 0 ? -hysteresis : hysteresis) {}

  bool update(float value) {
    if (!active_ && value >= setPoint_) {
      active_ = true;
    } else if (active_ && value <= setPoint_ - hysteresis_) {
      active_ = false;
    }
    return active_;
  }

  bool active() const { return active_; }
  void reset() { active_ = false; }

 private:
  float setPoint_;
  float hysteresis_;
  bool active_ = false;
};

// Convert a raw ADC reading to engineering units (e.g. amps), clamped to range.
inline float adcToUnits(int32_t raw, int32_t adcMax, float fullScale) {
  if (adcMax <= 0) return 0.0f;
  if (raw < 0) raw = 0;
  if (raw > adcMax) raw = adcMax;
  return (static_cast<float>(raw) / static_cast<float>(adcMax)) * fullScale;
}

}  // namespace sp
