#pragma once
#include "ActivityController.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>

namespace swoosh {
  class Segue : public Activity {
    friend class ActivityController;

  private:
    Activity* last;
    Activity* next;
    sf::Time duration;
    Timer timer;

  protected:
    const sf::Time getDuration() { return duration; }
    const sf::Time getElapsed() { return timer.getElapsed(); }

    void drawLastActivity(sf::RenderTexture& surface) {
      if (last)
        last->onDraw(surface);
    }

    void drawNextActivity(sf::RenderTexture& surface) {
      next->onDraw(surface);
    }

  public:
    virtual void onStart() final { next->onEnter();  last->onLeave(); timer.reset(); }

    virtual void onUpdate(double elapsed) final {
      if (last) last->onUpdate(elapsed);
      next->onUpdate(elapsed);
    }

    virtual void onLeave() final { timer.pause(); }
    virtual void onExit() final { ; }
    virtual void onEnter() final { ; }
    virtual void onResume() final { timer.start(); }
    virtual void onDraw(sf::RenderTexture& surface) = 0;
    virtual void onEnd() final { last->onExit(); }

    Segue() = delete;
    Segue(sf::Time duration, Activity* last, Activity* next) : duration(duration), last(last), next(next), Activity(next->controller) { /* ... */ }
    virtual ~Segue() { }
  };
}