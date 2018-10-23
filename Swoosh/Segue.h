#pragma once
#include "ActivityController.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>

template<sf::Time(*s)(int val), int val = 0>
struct Duration
{
  static sf::Time value() { return (*s)(val); }
};

class Segue : public Activity {
  friend class ActivityController;

private:
  Activity* last;
  Activity* next;
  sf::Time duration;
  Timer timer;

protected:
  const sf::Time getDuration() { return duration; }
  const sf::Time getElapsed() { return sf::milliseconds(timer.GetElapsed()); }
  
  void DrawLastActivity(sf::RenderTexture& surface) {
    if(last)
      last->OnDraw(surface);
  }

  void DrawNextActivity(sf::RenderTexture& surface) {
    next->OnDraw(surface);
  }

public:
  virtual void OnStart() final { next->OnStart(); last->OnLeave(); timer.Reset(); }

  virtual void OnUpdate(double elapsed) final {
    if (last) last->OnUpdate(elapsed);
    next->OnUpdate(elapsed);
  }

  virtual void OnLeave() final { timer.Pause(); }
  virtual void OnResume() final { timer.Start(); }
  virtual void OnDraw(sf::RenderTexture& surface) = 0;
  virtual void OnEnd() final { }

  Segue() = delete;
  Segue(sf::Time duration, Activity* last, Activity* next) : duration(duration), last(last), next(next), Activity(next->controller) { /* ... */ }
  virtual ~Segue() { }
};