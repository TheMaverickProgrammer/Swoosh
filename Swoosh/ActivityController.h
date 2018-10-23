#pragma once
#include "Activity.h"
#include "Segue.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <functional>

class ActivityController {
  friend class ::Segue;

private:
  std::stack<Activity*> activities;
  sf::RenderTexture* surface;
  bool willLeave;
  bool hasSegue;
  sf::RenderWindow& handle;

public:
  ActivityController(sf::RenderWindow& window) : handle(window) {
    surface = new sf::RenderTexture();
    surface->create((unsigned int)handle.getSize().x, (unsigned int)handle.getSize().y);
    willLeave = false;
    hasSegue = false;
  }

  ~ActivityController() {
    while (!activities.empty()) {
      Activity* activity = activities.top();
      delete activity;
      activities.pop();
    }

    delete surface;
  }

  sf::RenderWindow& getWindow() {
    return handle;
  }

  template<typename T, typename DurationType = Duration<&sf::seconds, 3>>
  class Segue{
  public:
    template<typename U, typename... Args>
    static std::function<void(ActivityController& owner)> To(Args... args) {
      auto delegate_func = [&](ActivityController& owner) {
        if (owner.hasSegue) { return; /* this shouldn't happen */ }

        bool hasLast = (owner.activities.size() > 0);
        Activity* last = hasLast ? owner.activities.top() : nullptr;
        Activity* next = new U(owner, args...);
        ::Segue* effect = new T(DurationType::value(), last, next);

        effect->OnStart();

        owner.activities.push(effect);

        owner.hasSegue = true;
      };

      return delegate_func;
    }
  };

  template<typename T, typename... Args>
  void Push(Args... args) {
    bool hasLast = (activities.size() > 0);
    Activity* last = hasLast ? activities.top() : nullptr;
    Activity* next = new T(*this, args...);

    if (hasLast) {
      last->OnEnd();
      activities.pop();
    }
    if(hasLast)
      delete last;

    next->OnStart();
    activities.push(next);
  }

  void Push(std::function<void(ActivityController& owner)> callback) {
    callback(*this);
  }

  const bool QueuePop() {
    bool hasMore = (activities.size() > 0);

    if (!hasMore || hasSegue) return false;

    willLeave = true;

    return willLeave;
  }

  void Update(double elapsed) {
    if (activities.size() == 0)
      return;

    if (willLeave) {
      Pop();
      willLeave = false;
    }

    if (activities.size() == 0)
      return;
    
    activities.top()->OnUpdate(elapsed);

    if (hasSegue) {
      ::Segue* segue = dynamic_cast<::Segue*>(activities.top());
      if (segue->timer.GetElapsed() >= segue->duration.asMilliseconds()) {
        EndSegue(segue);
      }
    }
  }

  void Draw() {
    if (activities.size() == 0)
      return;

    activities.top()->OnDraw(*surface);

    surface->display();

    // Capture buffer in a drawable context
    sf::Sprite post(surface->getTexture());

    // drawbuffer on top of the scene
    handle.draw(post);

    // Prepare buffer for next cycle
    surface->clear(sf::Color::Transparent);
  }
private:
  void EndSegue(::Segue* segue) {
    segue->OnEnd();
    activities.pop();
    Activity* next = segue->next;
    delete segue;
    activities.push(next);
    hasSegue = false;
  }

  void Pop() {
    Activity* activity = activities.top();
    activity->OnLeave();
    activity->OnEnd();
    activities.pop();

    if (activities.size() > 0)
      activities.top()->OnResume();

    delete activity;
  }
}; 
