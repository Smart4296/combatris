#pragma once

#include "utility/color.h"
#include "game/assets.h"
#include "game/panes/pane_interface.h"

class Pane : public PaneInterface {
 public:
  Pane(SDL_Renderer* renderer, int x, int y, const std::shared_ptr<Assets>& assets) : renderer_(renderer), x_(x), y_(y), assets_(assets) {}

 protected:
  void RenderText(int x, int y, Font font, const std::string& text, Color text_color) const {
    ::RenderText(renderer_, x_ + x, y_ + y, assets_->GetFont(font), text, text_color);
  }

  void SetDrawColor(const Color& c) const {
    auto color = GetColor(c);

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
  }

  void FillRect(int x, int y, int w, int h) const {
    SDL_Rect rc = { x_ + x, y_ + y, w, h };
    SDL_RenderFillRect(renderer_, &rc);
  }

  void RenderCopy(SDL_Texture *texture, int x, int y, int w, int h) const {
    SDL_Rect rc = { x_ + x, y_ + y, w, h };
    SDL_RenderCopy(renderer_, texture, nullptr, &rc);
  }

  SDL_Renderer* renderer_;
  int x_;
  int y_;
  const std::shared_ptr<Assets>& assets_;
};

class TextPane : public Pane {
 public:
  enum class Orientation { Left, Right };
  static const int kBoxWidth = 148;
  static const int kBoxHeight = 84;
  static const int kBoxInteriorWidth = 138;
  static const int kBoxInteriorHeight = 74;

  TextPane(SDL_Renderer* renderer, int x, int y, const std::string& text, const std::shared_ptr<Assets>& assets) : Pane(renderer, x, y, assets) {
    std::tie(caption_texture_, caption_width_, caption_height_) = CreateTextureFromText(renderer_, assets_->GetFont(Bold25), text, Color::White);
  }

  void SetCaptionOrientation(Orientation orientation) { orientation_ = orientation; }

  void SetCenteredText(int text) { SetCenteredText(std::to_string(text)); }

  void SetCenteredText(const std::string& text) {
    std::tie(text_texture_, txt_width_, txt_height_) = CreateTextureFromText(renderer_, assets_->GetFont(Bold45), text, Color::SteelGray);

    txt_x_ = ((kBoxWidth - txt_width_) / 2);
    txt_y_ = ((kBoxHeight - txt_height_) / 2) + (caption_height_ + 5);
  }

  virtual void Render() const override {
    if (Orientation::Right == orientation_) {
      RenderCopy(caption_texture_.get(), kBoxWidth - caption_width_, 0, caption_width_, caption_height_);
    } else {
      RenderCopy(caption_texture_.get(), 0, 0, caption_width_, caption_height_);
    }
    SetDrawColor(Color::Gray);
    FillRect(0, 5 + caption_height_, kBoxWidth, kBoxHeight);
    SetDrawColor(Color::Black);
    FillRect(5, 10 + caption_height_, kBoxInteriorWidth, kBoxInteriorHeight);
    if (text_texture_) {
      RenderCopy(text_texture_.get(), txt_x_, txt_y_, txt_width_, txt_height_);
    }
  }

 protected:
  int txt_x_ = 0;
  int txt_y_ = 0;
  int txt_width_ = 0;
  int txt_height_ = 0;
  int caption_width_ = 0;
  int caption_height_ = 0;
  UniqueTexturePtr caption_texture_;

 private:
  Orientation orientation_ = Orientation::Right;
  UniqueTexturePtr text_texture_;
};
