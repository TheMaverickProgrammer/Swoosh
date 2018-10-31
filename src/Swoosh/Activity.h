#pragma once
/*
forward decl
*/
namespace sf {
  class RenderTexture;
  class RenderWindow;
}

namespace swoosh {
  class ActivityController;

  class Activity {
    friend class Segue;

  private:
    ActivityController& controller;
  public:
    Activity() = delete;
    Activity(ActivityController& controller) : controller(controller) { ; }
    virtual void onStart() = 0;
    virtual void onUpdate(double elapsed) = 0;
    virtual void onLeave() = 0;
    virtual void onExit() = 0;
    virtual void onEnter() = 0;
    virtual void onResume() = 0;
    virtual void onDraw(sf::RenderTexture& surface) = 0;
    virtual void onEnd() = 0;
    virtual ~Activity() { ; }
    ActivityController& getController() { return controller; }
  };
}