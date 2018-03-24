#include "utility/timer.h"
#include "game/tetrion.h"

#include <set>
#include <functional>
#include <unordered_map>

namespace {

// DAS settings
uint32_t kAutoRepeatInitialDelay = 150; // milliseconds
uint32_t kAutoRepeatSubsequentDelay = 85; // milliseconds

constexpr int HatValueToButtonValue(Uint8 value) { return (0xFF << 8) | value; }

const std::unordered_map<std::string, const std::unordered_map<int, Tetrion::Controls>> kJoystickMappings = {
  {"Logitech Dual Action", {
      { HatValueToButtonValue(8), Tetrion::Controls::Left },
      { HatValueToButtonValue(2), Tetrion::Controls::Right },
      { HatValueToButtonValue(4), Tetrion::Controls::SoftDrop },
      { 0, Tetrion::Controls::RotateCounterClockwise },
      { 2, Tetrion::Controls::RotateClockwise },
      { 3, Tetrion::Controls::Hold },
      { 1, Tetrion::Controls::HardDrop },
      { 9, Tetrion::Controls::Start },
      { 8, Tetrion::Controls::Pause }
    }
  },
  {"Logitech Logitech Dual Action", {
      { HatValueToButtonValue(8), Tetrion::Controls::Left },
      { HatValueToButtonValue(2), Tetrion::Controls::Right },
      { HatValueToButtonValue(4), Tetrion::Controls::SoftDrop },
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
      { 15, Tetrion::Controls::RotateCounterClockwise },
      { 13, Tetrion::Controls::RotateClockwise },
      { 12, Tetrion::Controls::Hold },
      { 14, Tetrion::Controls::HardDrop },
      { 3, Tetrion::Controls::Start },
      { 0, Tetrion::Controls::Pause }
    }
  }
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
    if (SDL_NumJoysticks() > 0) {
      AttachJoystick(0);
    }
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

    if (kJoystickMappings.count(joystick_name_) == 0) {
      std::cout << "Non supported joystick found: " <<  joystick_name_ << " it will be DISABLED" << std::endl;
      DetachJoystick(joystick_index_);
    } else {
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
    }
    return current_control;
  }

  Tetrion::Controls TranslateJoystickCommands(SDL_JoystickID index, ButtonType type, Uint8 button) const {
    const auto& mapping = kJoystickMappings.at(joystick_name_);
    int v = (type == ButtonType::JoyButton) ? button : HatValueToButtonValue(button);

    if (index != joystick_index_ || !mapping.count(v)) {
      return Tetrion::Controls::None;
    }
    return mapping.at(v);
  }

  void Repeatable(const Tetrion::Controls control, Tetrion::Controls& previous_control, RepeatFunc& func, int64_t& time_since_last_auto_repeat) {
    if (control == previous_control) {
      return;
    }
    previous_control = control;
    func = [this, control]() { tetrion_->GameControl(control); };
    func();
    time_since_last_auto_repeat = time_in_ms();
  }

  void Play() {
    bool quit = false;
    DeltaTimer delta_timer;
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
        current_control = Tetrion::Controls::None;

        switch (event.type) {
          case SDL_KEYDOWN:
            current_control = TranslateKeyboardCommands(event, button_pressed);
            button_pressed = true;
            break;
          case SDL_JOYBUTTONDOWN:
            current_control = TranslateJoystickCommands(event.jbutton.which, button_type, event.jbutton.button);
            button_pressed = true;
            break;
          case SDL_KEYUP:
          case SDL_JOYBUTTONUP:
            button_pressed = false;
            function_to_repeat = nullptr;
            auto_repeat_threshold = kAutoRepeatInitialDelay;
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
            Repeatable(Tetrion::Controls::Left, previous_control, function_to_repeat, time_since_last_auto_repeat);
            break;
          case Tetrion::Controls::Right:
            Repeatable(Tetrion::Controls::Right, previous_control, function_to_repeat, time_since_last_auto_repeat);
            break;
          case Tetrion::Controls::SoftDrop:
            Repeatable(Tetrion::Controls::SoftDrop, previous_control, function_to_repeat, time_since_last_auto_repeat);
            break;
          case Tetrion::Controls::RotateCounterClockwise:
            tetrion_->GameControl(Tetrion::Controls::RotateCounterClockwise);;
            break;
          case Tetrion::Controls::RotateClockwise:
            tetrion_->GameControl(Tetrion::Controls::RotateClockwise);
            break;
          case Tetrion::Controls::HardDrop:
            tetrion_->GameControl(Tetrion::Controls::HardDrop);
            break;
          case Tetrion::Controls::Hold:
            tetrion_->GameControl(Tetrion::Controls::Hold);
            break;
          case Tetrion::Controls::Start:
            tetrion_->NewGame();
            break;
          case Tetrion::Controls::Pause:
            tetrion_->Pause();
            break;
          default:
            break;
        }
        if (kAutoRepeatControls.count(current_control) == 0) {
          function_to_repeat = nullptr;
          previous_control = Tetrion::Controls::None;
        }
      }
      if (button_pressed && (time_in_ms() - time_since_last_auto_repeat) >= auto_repeat_threshold) {
        if (function_to_repeat) {
          function_to_repeat();
        }
        auto_repeat_threshold = kAutoRepeatSubsequentDelay;
        time_since_last_auto_repeat = time_in_ms();
      }
      tetrion_->Update(delta_timer.GetDelta());
    }
  }

 private:
  int joystick_index_ = -1;
  std::string joystick_name_;
  SDL_Joystick* joystick_ = nullptr;
  std::shared_ptr<Tetrion> tetrion_ = nullptr;
};

int main(int, char*[]) {
  Combatris combatris;

  combatris.Play();

  return 0;
}
