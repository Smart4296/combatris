#pragma once

#include "game/panes/multi_player.h"
#include "game/panes/total_lines.h"
#include "game/panes/scoring.h"
#include "game/panes/high_score.h"
#include "game/panes/next_queue.h"
#include "game/panes/hold_queue.h"
#include "game/panes/moves.h"
#include "game/animation.h"

class Tetrion final {
 public:
  const char* kWindowTitleSinglePlayer = "COMBATRIS - Single Player";
  const char* kWindowTitleBattle = "COMBATRIS - Battle";
  enum class Campaign { SinglePlayer, Battle };
  enum class Controls {
    None,
    RotateClockwise,
    RotateCounterClockwise,
    SoftDrop,
    HardDrop,
    Left,
    Right,
    Hold,
    Pause,
    Start,
    ToggleCampaign
  };

  Tetrion();

  Tetrion(const Tetrion&) = delete;

  Tetrion(const Tetrion&&) = delete;

  ~Tetrion() noexcept;

  void NewGame() { events_.Push(Event::Type::NewGame); }

  void Pause() {
    if (campaign_ == Campaign::Battle || !tetromino_in_play_) {
      return;
    }
    if (!game_paused_) {
      unpause_pressed_ = false;
      game_paused_ = true;
      events_.Push(Event::Type::Pause);
    } else if (!unpause_pressed_) {
      unpause_pressed_ = true;
    }
  }

  void ToggleCampaign() {
    if (tetromino_in_play_) {
      return;
    }
    if (Campaign::SinglePlayer == campaign_) {
      multi_player_->Enable();
      campaign_ = Campaign::Battle;
      SDL_SetWindowTitle(window_, kWindowTitleBattle);
    } else {
      multi_player_->Disable();
      campaign_ = Campaign::SinglePlayer;
      SDL_SetWindowTitle(window_, kWindowTitleSinglePlayer);
    }
  }

  void ResetCountDown();

  void GameControl(Controls control_pressed);

  void Update(double delta_timer);

 protected:
  template<class T, class ...Args>
  void AddAnimation(Args&&... args) { animations_.push_back(std::make_shared<T>(std::forward<Args>(args)...)); }

  void AddPane(PaneInterface* pane) { panes_.push_back(pane); }

  void AddPane(Pane* pane) {
    panes_.push_back(pane);

    auto sink = dynamic_cast<EventSink*>(pane);

    if (nullptr != sink) {
      event_sinks_.push_back(sink);
    }
  }

  void EventHandler(Events& events);

  void Render(double delta_timer);

 private:
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  std::shared_ptr<TetrominoSprite> tetromino_in_play_;
  std::shared_ptr<Assets> assets_;
  std::shared_ptr<TetrominoGenerator> tetromino_generator_;
  std::shared_ptr<Matrix> matrix_;
  std::shared_ptr<Level> level_;
  std::unique_ptr<Scoring> scoring_;
  std::unique_ptr<HighScore> high_score_;
  std::unique_ptr<NextQueue> next_queue_;
  std::unique_ptr<HoldQueue> hold_queue_;
  std::unique_ptr<TotalLines> total_lines_;
  std::unique_ptr<Moves> moves_;
  std::unique_ptr<MultiPlayer> multi_player_;
  std::vector<PaneInterface*> panes_;
  std::vector<EventSink*> event_sinks_;
  Events events_;
  bool game_paused_ = false;
  bool unpause_pressed_ = false;
  Campaign campaign_ = Campaign::SinglePlayer;
  std::deque<std::shared_ptr<Animation>> animations_;
};
