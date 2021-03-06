#pragma once

#include "game/panes/pane.h"
#include "network/multiplayer_controller.h"

#include <functional>

namespace {

const int kX = kMatrixEndX + kMinoWidth + (kSpace * 4) + TextPane::kBoxWidth;
const int kY = kMatrixStartY - kMinoHeight;

} // namesapce

class Player final {
 public:
  enum TextureID { Name, State, ScoreCaption, Score, KOCaption, KO, LevelCaption, Level, LinesCaption, Lines, LinesSentCaption, LinesSent};
  using Ptr = std::shared_ptr<Player>;
  using GameState = network::GameState;
  using Function = std::function<std::string(int)>;
  using MatrixType = std::array<std::array<uint8_t, kVisibleCols + 2>, kVisibleRows + 2>;

  Player(SDL_Renderer* renderer, const std::string& name, uint64_t host_id, const std::shared_ptr<Assets>& assets);

  Player(const Player&) = delete;

  Player(const Player&&) noexcept = delete;

  const std::string& name() const { return name_; }

  uint64_t host_id() const { return host_id_; }

  bool ProgressUpdate(int lines, int score, int level, bool set_to_zero = false);

  int score() const { return score_; }

  void SetMatrixState(const network::MatrixState& state);

  void SetState(GameState state, bool set_to_zero = false);

  GameState state() const { return state_; }

  void AddLinesSent(int lines_sent, bool set_to_zero = false);

  int lines_sent() const { return lines_sent_; }

  void AddKO(int ko, bool set_to_zero = false);

  int ko() const { return ko_; }

  void Reset();

  void Render(int x_offset, int y_offset, bool is_my_status) const;

 private:
  struct Texture {
    Texture(UniqueTexturePtr&& texture, int w, int h, SDL_Rect rc)
        : texture_(std::move(texture)), w_(w), h_(h), rc_(rc) {}

    void Set(UniqueTexturePtr&& texture, int w, int h) {
      texture_ = std::move(texture);
      w_ = w;
      h_ = h;
    }
    UniqueTexturePtr texture_ = nullptr;
    int w_;
    int h_;
    SDL_Rect rc_;
  };

  int Update(Player::TextureID id, int new_value, int old_value, Function to_string, bool set_to_zero = false);

  SDL_Renderer* renderer_;
  std::string name_;
  uint64_t host_id_;
  const std::shared_ptr<Assets>& assets_;
  int lines_ = 0;
  int lines_sent_ = 0;
  int score_ = 0;
  int level_ = -1;
  int ko_ = 0;
  GameState state_ = GameState::None;
  MatrixType matrix_;
  std::vector<std::shared_ptr<const Tetromino>> tetrominos_;
  std::unordered_map<TextureID, std::shared_ptr<Texture>> textures_;
};
