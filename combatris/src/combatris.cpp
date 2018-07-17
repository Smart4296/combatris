#include "utility/timer.h"
#include "game/tetrion.h"

#include <set>
#include <functional>
#include <unordered_map>

namespace {

// DAS settings
int64_t kAutoRepeatInitialDelay = 200; // milliseconds
int64_t kAutoRepeatSubsequentDelay = 50; // milliseconds

constexpr int HatValueToButtonValue(Uint8 value) { return (0xFF << 8) | value; }

const std::unordered_map<std::string, const std::unordered_map<int, Tetrion::Controls>> kJoystickMappings = {
  {"Logitech Dual Action", {
      { HatValueToButtonValue(8), Tetrion::Controls::Left },
      { HatValueToButtonValue(2), Tetrion::Controls::Right },
      { HatValueToButtonValue(4), Tetrion::Controls::SoftDrop },
      { HatValueToButtonValue(1), Tetrion::Controls::RotateClockwise },
      { 0, Tetrion::Controls::RotateCounterClockwise },
      { 2, Tetrion::Controls::RotateClockwise },
      { 3, Tetrion::Controls::Hold },
      { 1, Tetrion::Controls::HardDrop },
      { 9, Tetrion::Controls::Start },
      { 8, Tetrion::Controls::Pause }
    }
  },
  {"PLAYSTATION(R)3 Controller", {
      { 7, Tetrion::Controls::Left },
      { 5, Tetrion::Controls::Right },
      { 6, Tetrion::Controls::SoftDrop },
      { 4, Tetrion::Controls::RotateClockwise },
      { 15, Tetrion::Controls::RotateCounterClockwise },
      { 13, Tetrion::Controls::RotateClockwise },
      { 12, Tetrion::Controls::Hold },
      { 14, Tetrion::Controls::HardDrop },
      { 3, Tetrion::Controls::Start },
      { 0, Tetrion::Controls::Pause }
    }
  },
  {"8Bitdo NES30 GamePad", {
      { 6, Tetrion::Controls::Left },
      { 7, Tetrion::Controls::Right },
      { -1, Tetrion::Controls::SoftDrop },
      { -1, Tetrion::Controls::RotateClockwise },
      { 4, Tetrion::Controls::RotateCounterClockwise },
      { 0, Tetrion::Controls::RotateClockwise },
      { 3, Tetrion::Controls::Hold },
      { 1, Tetrion::Controls::HardDrop },
      { 11, Tetrion::Controls::Start },
      { 10, Tetrion::Controls::Pause }
    }
  }
};

// use_axismotion, joystick mapping
const std::unordered_map<std::string, std::pair<bool, std::string>> kTranslateJoystickName = {
  { "Logitech Dual Action", { false, "Logitech Dual Action" } },
  { "Logitech Logitech Dual Action", { false, "Logitech Dual Action" } },
  { "PLAYSTATION(R)3 Controller", { false, "PLAYSTATION(R)3 Controller" } },
  { "8Bitdo NES30 GamePad", { true, "8Bitdo NES30 GamePad"} },
  { "NES30 Joystick",{ true, "8Bitdo NES30 GamePad" } }
};

const std::set<Tetrion::Controls> kAutoRepeatControls = { Tetrion::Controls::SoftDrop, Tetrion::Controls::Left, Tetrion::Controls::Right };

} // namespace

class Combatris {
 public:
  enum class ButtonType { HatButton, JoyButton };
  using RepeatFunc = std::function<void()>;

  Combatris() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
      std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
      exit(-1);
    }
    if (TTF_Init() != 0) {
      std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
      exit(-1);
    }
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  }

  ~Combatris() {
    tetrion_.reset();
    DetachJoystick(joystick_index_);
    SDL_Quit();
    TTF_Quit();
  }

  void AttachJoystick(int index) {
    if (nullptr != joystick_) {
      return;
    }
    joystick_ = SDL_JoystickOpen(index);
    if (nullptr == joystick_) {
      std::cout << "Warning: Unable to open game controller! SDL Error: " << SDL_GetError() << std::endl;
      exit(-1);
    }
    joystick_index_ = index;
    joystick_name_ = SDL_JoystickName(joystick_);

    if (kTranslateJoystickName.count(joystick_name_) == 0) {
      std::cout << "Non supported joystick found: \"" <<  joystick_name_ << "\" it will be DISABLED" << std::endl;
      DetachJoystick(joystick_index_);
    } else {
      std::tie(use_axismotion_ , joystick_name_) = kTranslateJoystickName.at(joystick_name_);
      std::cout << "Joystick found: " <<  joystick_name_ << std::endl;
    }
  }

  void DetachJoystick(int index) {
    if (nullptr == joystick_ || index != joystick_index_) {
      return;
    }
    SDL_JoystickClose(joystick_);
    joystick_ = nullptr;
    joystick_index_ = -1;
    joystick_name_ = "";
  }

  Tetrion::Controls TranslateKeyboardCommands(const SDL_Event& event, bool button_pressed) const {
    auto current_control = Tetrion::Controls::None;

    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
      current_control = Tetrion::Controls::Left;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
      current_control = Tetrion::Controls::Right;
    }  if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
      current_control = Tetrion::Controls::SoftDrop;
    } else if (!button_pressed && event.key.keysym.scancode == SDL_SCANCODE_Z) {
      current_control = Tetrion::Controls::RotateCounterClockwise;
    } else if (!button_pressed && (event.key.keysym.scancode == SDL_SCANCODE_UP || event.key.keysym.scancode == SDL_SCANCODE_X)) {
      current_control = Tetrion::Controls::RotateClockwise;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_C) {
      current_control = Tetrion::Controls::Hold;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
      current_control = Tetrion::Controls::HardDrop;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_N) {
      current_control = Tetrion::Controls::Start;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_P) {
      current_control = Tetrion::Controls::Pause;
    } else if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
      current_control = Tetrion::Controls::Quit;
    } else if (event.key.keysym.scancode >=  SDL_SCANCODE_1 && event.key.keysym.scancode <=  SDL_SCANCODE_9) {
      current_control = Tetrion::Controls::DebugSendLine;
    }

    return current_control;
  }

  Tetrion::Controls TranslateJoystickCommands(SDL_JoystickID index, ButtonType type, Uint8 button) const {
    if (index != joystick_index_) {
      return Tetrion::Controls::None;
    }
    const auto& mapping = kJoystickMappings.at(joystick_name_);
    int v = (type == ButtonType::JoyButton) ? button : HatValueToButtonValue(button);

    if (!mapping.count(v)) {
      return Tetrion::Controls::None;
    }
    return mapping.at(v);
  }

  Tetrion::Controls TranslateJoystickAxisMotion(SDL_JoystickID index, Uint8 axis, Sint16 value) const {
    if (!use_axismotion_ || index != joystick_index_) {
      return Tetrion::Controls::None;
    }
    if (0 == axis) {
      return (-32768 == value) ? Tetrion::Controls::Left : Tetrion::Controls::Right;
    }
    return (-32768 == value) ? Tetrion::Controls::Up : Tetrion::Controls::Down;
  }

  template <Tetrion::Controls control>
  void Repeatable(Tetrion::Controls& previous_control, RepeatFunc& func, int& repeat_count, int64_t& time_since_last_auto_repeat) {
    if (control == previous_control) {
      return;
    }
    previous_control = control;
    func = [this]() { tetrion_->GameControl(control); };
    repeat_count = 0;
    time_since_last_auto_repeat = kAutoRepeatInitialDelay;
  }

  void Play() {
    bool quit = false;
    DeltaTimer delta_timer;
    int repeat_count = 0;
    bool button_pressed = false;
    int64_t time_since_last_auto_repeat = 0;
    int64_t auto_repeat_threshold = kAutoRepeatInitialDelay;
    std::function<void()> function_to_repeat;
    Tetrion::Controls current_control;
    Tetrion::Controls previous_control = Tetrion::Controls::None;

    tetrion_ = std::make_shared<Tetrion>();
    while (!quit) {
      SDL_Event event;

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          quit = true;
          break;
        }
        auto button_type = ButtonType::JoyButton;

        if (SDL_JOYHATMOTION == event.type) {
          button_type = ButtonType::HatButton;
          event.type = (event.jhat.value == 0) ? SDL_JOYBUTTONUP : SDL_JOYBUTTONDOWN;

          if (event.type == SDL_JOYBUTTONDOWN) {
            event.jbutton.button = event.jhat.value;
          }
        }
        else if (use_axismotion_ && SDL_JOYAXISMOTION == event.type) {
          if (event.jaxis.value == 0 && repeat_count == 0) {
            event.type = SDL_FIRSTEVENT;
          } else {
            event.type = (0 == event.jaxis.value) ? SDL_JOYBUTTONUP : SDL_JOYAXISMOTION;
          }
        }
        current_control = Tetrion::Controls::None;

        switch (event.type) {
        case SDL_KEYDOWN:
          current_control = TranslateKeyboardCommands(event, button_pressed);
          button_pressed = (current_control != Tetrion::Controls::None);
          break;
        case SDL_JOYAXISMOTION:
          current_control = TranslateJoystickAxisMotion(event.jbutton.which, event.jaxis.axis, event.jaxis.value);
          button_pressed = (current_control != Tetrion::Controls::None);
          break;
        case SDL_JOYBUTTONDOWN:
          current_control = TranslateJoystickCommands(event.jbutton.which, button_type, event.jbutton.button);
          button_pressed = (current_control != Tetrion::Controls::None);
          break;
        case SDL_KEYUP:
        case SDL_JOYBUTTONUP:
          repeat_count = 0;
          button_pressed = false;
          function_to_repeat = nullptr;
          previous_control = Tetrion::Controls::None;
          break;
        case SDL_JOYDEVICEADDED:
          AttachJoystick(event.jbutton.which);
          break;
        case SDL_JOYDEVICEREMOVED:
          DetachJoystick(event.jbutton.which);
          break;
        }
        if (Tetrion::Controls::None == current_control) {
          continue;
        }
        switch (current_control) {
        case Tetrion::Controls::Left:
          Repeatable<Tetrion::Controls::Left>(previous_control, function_to_repeat, repeat_count, time_since_last_auto_repeat);
          break;
        case Tetrion::Controls::Right:
          Repeatable<Tetrion::Controls::Right>(previous_control, function_to_repeat, repeat_count, time_since_last_auto_repeat);
          break;
        case Tetrion::Controls::SoftDrop:
          Repeatable<Tetrion::Controls::SoftDrop>(previous_control, function_to_repeat, repeat_count, time_since_last_auto_repeat);
          break;
        case Tetrion::Controls::Start:
          tetrion_->NewGame();
          break;
        case Tetrion::Controls::Pause:
          tetrion_->Pause();
          break;
        case Tetrion::Controls::Quit:
          quit = true;
          break;
        case Tetrion::Controls::DebugSendLine:
    #if !defined(NDEBUG)
          tetrion_->GameControl(Tetrion::Controls::DebugSendLine, 9 - (SDL_SCANCODE_9 - event.key.keysym.scancode));
    #endif
          break;
        default:
          tetrion_->GameControl(current_control);
          break;
        }
        if (kAutoRepeatControls.count(current_control) == 0) {
          repeat_count = 0;
          function_to_repeat = nullptr;
          previous_control = Tetrion::Controls::None;
        }
      }
      if (button_pressed && (time_in_ms() - time_since_last_auto_repeat) >= auto_repeat_threshold) {
        if (function_to_repeat) {
          function_to_repeat();
        }
        auto_repeat_threshold = (repeat_count == 0) ? kAutoRepeatInitialDelay : kAutoRepeatSubsequentDelay;
        repeat_count++;
        time_since_last_auto_repeat = time_in_ms();
      }
      tetrion_->Update(delta_timer.GetDelta());
    }
  }

 private:
  int joystick_index_ = -1;
  std::string joystick_name_;
  SDL_Joystick* joystick_ = nullptr;
  bool use_axismotion_ = false;
  std::shared_ptr<Tetrion> tetrion_ = nullptr;
};

int main(int, char *[]) {
  Combatris combatris;

  combatris.Play();

  return 0;
}
