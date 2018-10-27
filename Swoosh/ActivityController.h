#pragma once
#include "Activity.h"
#include "Segue.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <functional>
#include <utility>

namespace swoosh {
  class ActivityController {
    friend class swoosh::Segue;

  private:
    std::stack<Activity*> activities;
    sf::RenderTexture* surface;
    bool willLeave;
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

    template<typename T, typename DurationType = Duration<&sf::seconds, 1>>
    class Segue {
    public:
      void DelegateActivityPop(ActivityController& owner) {
        Activity* last = owner.activities.top();
        owner.activities.pop();

        Activity* next = owner.activities.top();
        owner.activities.pop();

        swoosh::Segue* effect = new T(DurationType::value(), last, next);

        effect->onStart();

        owner.activities.push(effect);
      }

      template<typename U>
      class To {
      public:
        template<typename... Args >
        To(ActivityController& owner, Args&&... args) {
          bool hasLast = (owner.activities.size() > 0);
          Activity* last = hasLast ? owner.activities.top() : nullptr;
          Activity* next = new U(owner, std::forward<Args>(args)...);
          swoosh::Segue* effect = new T(DurationType::value(), last, next);

          effect->onStart();

          owner.activities.push(effect);
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

    template <typename T, bool U>
    struct ResolvePushSegueIntent
    {
      ResolvePushSegueIntent(ActivityController& owner) {
        static_assert("Swoosh could not handle the segue intent");
      }
    };

    template<typename T>
    struct ResolvePushSegueIntent<T, false>
    {
      template<typename... Args >
      ResolvePushSegueIntent(ActivityController& owner, Args&&... args) {
        if (owner.segueAction == SegueAction::NONE) {
          owner.segueAction = SegueAction::PUSH;
          T segueResolve(owner, std::forward<Args>(args)...);
        }
      }
    };

    template<typename T>
    struct ResolvePushSegueIntent<T, true>
    {
      template<typename... Args>
      ResolvePushSegueIntent(ActivityController& owner, Args&&... args) {
        bool hasLast = (owner.activities.size() > 0);
        Activity* last = hasLast ? owner.activities.top() : nullptr;
        Activity* next = new T(owner, std::forward<Args>(args)...);
        if (hasLast) {
          last->onEnd();
          owner.activities.pop();
        }
        if (hasLast)
          delete last;

        next->onStart();
        owner.activities.push(next);
      }
    };

    template<typename T, typename... Args>
    void push(Args&&... args) {
      ResolvePushSegueIntent<T, ActivityTypeQuery<T>::value> intent(*this, std::forward<Args>(args)...);
    }

    //template<typename T, typename U = typename std::enable_if<T::DelegateActivityPop>::type>
    template<typename T>
    const bool queuePop() {
      // Have to have more than 1 on the stack...
      bool hasLast = (activities.size() > 1);
      if (!hasLast || segueAction != SegueAction::NONE) return false;

      segueAction = SegueAction::POP;
      T segueResolve;
      segueResolve.DelegateActivityPop(*this);

      return true;
    }


    const bool queuePop() {
      bool hasMore = (activities.size() > 0);

      if (!hasMore || segueAction != SegueAction::NONE) return false;

      willLeave = true;

      return true;
    }

    void update(double elapsed) {
      if (activities.size() == 0)
        return;

      if (willLeave) {
        pop();
        willLeave = false;
      }

      if (activities.size() == 0)
        return;

      activities.top()->onUpdate(elapsed);

      if (segueAction != SegueAction::NONE) {
        swoosh::Segue* segue = dynamic_cast<swoosh::Segue*>(activities.top());
        if (segue->timer.getElapsed().asMilliseconds() >= segue->duration.asMilliseconds()) {
          endSegue(segue);
        }
      }
    }

    void draw() {
      if (activities.size() == 0)
        return;

      activities.top()->onDraw(*surface);

      surface->display();

      // Capture buffer in a drawable context
      sf::Sprite post(surface->getTexture());

      // drawbuffer on top of the scene
      handle.draw(post);

      // Prepare buffer for next cycle
      surface->clear(sf::Color::Transparent);
    }
  private:
    void endSegue(swoosh::Segue* segue) {
      segue->onEnd();
      activities.pop();

      Activity* next = segue->next;

      if (segueAction == SegueAction::PUSH) {
        next->onStart(); // new item on stack first time call
      }
      else if (segueAction == SegueAction::POP) {
        // We're removing an item from the stack
        Activity* last = segue->last;
        last->onEnd();
        next->onResume();
        delete last;
      }

      delete segue;
      activities.push(next);
      segueAction = SegueAction::NONE;
    }

    void pop() {
      Activity* activity = activities.top();
      activity->onEnd();
      activities.pop();

      if (activities.size() > 0)
        activities.top()->onResume();

      delete activity;
    }
  };
}