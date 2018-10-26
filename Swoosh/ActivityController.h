#pragma once
#include "Activity.h"
#include "Segue.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <functional>
#include <utility>

class ActivityController {
  friend class ::Segue;

private:
  std::stack<Activity*> activities;
  sf::RenderTexture* surface;
  bool willLeave;
  bool hasSegue;
  sf::RenderWindow& handle;

  enum SegueAction {
    POP,
    PUSH,
    NONE
  } segueAction;

public:
  ActivityController(sf::RenderWindow& window) : handle(window) {
    surface = new sf::RenderTexture();
    surface->create((unsigned int)handle.getSize().x, (unsigned int)handle.getSize().y);
    willLeave = false;
    segueAction = SegueAction::NONE;
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
  class Segue {
  public:
    void DelegateActivityPop(ActivityController& owner) {
      if (owner.hasSegue) { return; /* this shouldn't happen */ }

      if (owner.segueAction == SegueAction::POP) {
        Activity* last = owner.activities.top();
        owner.activities.pop();

        Activity* next = owner.activities.top();

        ::Segue* effect = new T(DurationType::value(), last, next);

        effect->OnStart();

        owner.activities.push(effect);
      }
      else {
        throw std::logic_error("Segue POP recieved but no action was placed");
      }
    }

    template<typename U, typename... Args>
    class To {
    public:
      bool isToType() { return true; }
    public:
      To(ActivityController& owner, Args... args) {
        if (owner.hasSegue) { return; /* this shouldn't happen */ }

        if (owner.segueAction == SegueAction::PUSH) {
          bool hasLast = (owner.activities.size() > 0);
          Activity* last = hasLast ? owner.activities.top() : nullptr;
          Activity* next = new U(owner, args...);
          ::Segue* effect = new T(DurationType::value(), last, next);

          effect->OnStart();

          owner.activities.push(effect);
        }
        else {
          throw std::logic_error("Segue PUSH recieved but no action was placed");
        }
      }
    };
  };

  template <class T>
  struct ActivityTypeQuery
  {
    static char is_to(Activity*) {}

    static double is_to(...) { ; }

    static T* t;
    enum { value = sizeof(is_to(t)) == sizeof(char) };
  };

  template <typename T, bool U, typename... Args>
  struct ResolveSegueIntent
  {
    ResolveSegueIntent(ActivityController& owner, Args... args) { ; }
  };

  template<typename T, typename... Args>
  struct ResolveSegueIntent<T, false, Args...>
  {
    ResolveSegueIntent(ActivityController& owner, Args... args) {
      owner.segueAction = SegueAction::PUSH;
      T segueResolve(owner, args...);
    }
  };

  template<typename T, typename... Args>
  struct ResolveSegueIntent<T, true, Args...>
  {
    ResolveSegueIntent(ActivityController& owner, Args... args) {
      bool hasLast = (owner.activities.size() > 0);
      Activity* last = hasLast ? owner.activities.top() : nullptr;
      Activity* next = new T(owner, args...);
      if (hasLast) {
        last->OnEnd();
        owner.activities.pop();
      }
      if (hasLast)
        delete last;

      next->OnStart();
      owner.activities.push(next);
    }
  };

  template<typename T, typename... Args>
  void Push(Args... args) {
    ResolveSegueIntent<T, ActivityTypeQuery<T>::value, Args...> intent(*this, args...);
  }

  //template<typename T, typename U = typename std::enable_if<T::DelegateActivityPop>::type>
  template<typename T>
  const bool QueuePop() {
    // Have to have more than 1 on the stack...
    bool hasLast = (activities.size()-1 > 0);
    if (!hasLast || segueAction != SegueAction::NONE) return false;

    segueAction = SegueAction::POP;
    T segueResolve;
    segueResolve.DelegateActivityPop(*this);

    return true;
  }


  const bool QueuePop() {
    bool hasMore = (activities.size() > 0);

    if (!hasMore || segueAction != SegueAction::NONE) return false;

    willLeave = true;

    return true;
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

    if (segueAction != SegueAction::NONE) {
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

    // Remove from the stack 
    if (segueAction == SegueAction::POP) {
      Activity* last = segue->last; 
      last->OnEnd();
      delete last;
    }

    Activity* next = segue->next;
    delete segue;
    activities.push(next);
    segueAction = SegueAction::NONE;
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
