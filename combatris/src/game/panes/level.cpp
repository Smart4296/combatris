#include "game/panes/level.h"

namespace {

// Gravity is expressed in unit G, where 1G = 1 cell per frame, and 0.1G = 1 cell per 10 frames
struct LevelData {
  LevelData(double gravity, double lock_delay) : gravity_(gravity), lock_delay_(lock_delay) {}

  double gravity_;
  double lock_delay_;
};

const std::vector<LevelData> kLevelData = {
  LevelData(0.01667, 0.5),
  LevelData(0.021017, 0.5),
  LevelData(0.026977, 0.5),
  LevelData(0.035256, 0.5),
  LevelData(0.04693, 0.5),
  LevelData(0.06361, 0.5),
  LevelData(0.0879, 0.5),
  LevelData(0.1236, 0.5),
  LevelData(0.1775, 0.5),
  LevelData(0.2598, 0.5),
  LevelData(0.388, 0.5),
  LevelData(0.59, 0.5),
  LevelData(0.92, 0.5),
  LevelData(1.46, 0.5),
  LevelData(2.36, 0.5)
};

} // namespace

void Level::SetThresholds() {
  if (level_ + 1 > static_cast<int>(kLevelData.size())) {
    events_.Push(Event::Type::LastLevelCompleted);
  }
  auto index = std::min(level_ + 1, static_cast<int>(kLevelData.size()));

  wait_time_ = (1.0 / kLevelData.at(index).gravity_) / 60.0;
  lock_delay_ = kLevelData.at(index).lock_delay_;
  std::cout << "Current level index " << index << std::endl;
}

bool Level::WaitForMoveDown(double time_delta) {
  time_ += time_delta;
  if (time_ >= wait_time_) {
    time_ = 0.0;
    return true;
  }
  return false;
}

bool Level::WaitForLockDelay(double time_delta) {
  time_ += time_delta;
  if  (time_ >= lock_delay_) {
    return true;
  }
  return false;
}

void Level::Update(const Event& event) {
  if (!event.Is(Event::Type::LinesCleared)) {
    return;
  }
  lines_this_level_ += event.value_;

  if (lines_this_level_ >= lines_for_next_level_) {
    lines_this_level_ = 0;
    level_++;
    SetThresholds();
    SetCenteredText(level());
    events_.Push(Event::Type::LevelUp, level_ + 1);
    std::cout << "lines for next level " << lines_for_next_level_ << std::endl;
  }
}
