#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include <Defaults.hpp>
#include <GameModel.hpp>

class IRenderer{
public:
  virtual ~IRenderer() {}
  virtual void init() = 0;
  virtual void render (const GameModel & model) = 0;
  virtual void clear() = 0;
};

#endif