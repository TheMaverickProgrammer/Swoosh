#pragma once
#include <SFML\Graphics.hpp>

namespace swoosh {
  class Activity {
    friend class Segue;
    friend class ActivityController;

  private:
    ActivityController& controller;
    sf::View view;

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
    void setView(sf::View view) { this->view = view; }
    const sf::View getView() const { return this->view;  }
    ActivityController& getController() { return controller; }
  };
}