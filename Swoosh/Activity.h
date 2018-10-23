#pragma once

/*
forward decl
*/
namespace sf {
  class RenderTexture;
}

class ActivityController;

class Activity {
  friend class Segue;

protected:
  ActivityController& controller;
public:
  Activity() = delete;
  Activity(ActivityController& controller) : controller(controller) { ; }
  virtual void OnStart() = 0;
  virtual void OnUpdate(double elapsed) = 0;
  virtual void OnLeave() = 0;
  virtual void OnResume() = 0;
  virtual void OnDraw(sf::RenderTexture& surface) = 0;
  virtual void OnEnd() = 0;
  virtual ~Activity() { ; }
};