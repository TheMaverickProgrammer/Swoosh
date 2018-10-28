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
    sf::RenderWindow& handle;
    sf::Vector2u initWindowSize;

    bool willLeave;

    enum SegueAction {
      POP,
      PUSH,
      NONE
    } segueAction;

  public:
    ActivityController(sf::RenderWindow& window) : handle(window) {
      initWindowSize = handle.getSize();

      surface = new sf::RenderTexture();
      surface->create((unsigned int)handle.getSize().x, (unsigned int)handle.getSize().y);
      willLeave = false;
      segueAction = SegueAction::NONE;
    }

    ~ActivityController() {
      if (segueAction != SegueAction::NONE) {
        swoosh::Segue* effect = dynamic_cast<swoosh::Segue*>(activities.top());

        if (segueAction == SegueAction::PUSH) {
          delete effect->next;
        }

        delete effect;
        activities.pop();
      }

      while (!activities.empty()) {
        Activity* activity = activities.top();
        activities.pop();
        delete activity;
      }

      delete surface;
    }

    const sf::Vector2u getInitialWindowSize() const {
      return initWindowSize;
    }

    sf::RenderWindow& getWindow() {
      return handle;
    }

    template<typename T, typename DurationType>
    class segue {
    public:
      void delegateActivityPop(ActivityController& owner) {
        Activity* last = owner.activities.top();
        owner.activities.pop();

        Activity* next = owner.activities.top();
        owner.activities.pop();

        swoosh::Segue* effect = new T(DurationType::value(), last, next);

        effect->onStart();

        owner.activities.push(effect);
      }

      template<typename U>
      class to {
      public:

        to() { ; }

        template<typename... Args >
        void delegateActivityPush(ActivityController& owner, Args&&... args) {
          bool hasLast = (owner.activities.size() > 0);
          Activity* last = hasLast ? owner.activities.top() : nullptr;
          Activity* next = new U(owner, std::forward<Args>(args)...);
          swoosh::Segue* effect = new T(DurationType::value(), last, next);

          effect->onStart();

          owner.activities.push(effect);
        }

        template<typename... Args >
        bool delegateActivityRewind(ActivityController& owner, Args&&... args) {
          std::stack<Activity*> original;

          bool hasMore = (owner.activities.size() > 1);

          if (!hasMore) { return false; }

          Activity* last = owner.activities.top();
          owner.activities.pop();

          Activity* next = owner.activities.top();

          while (dynamic_cast<T*>(next) == 0 && owner.activities.size() > 1) {
            original.push(next);
            owner.activities.pop();
            next = owner.activities.top();
          }

          if (owner.activities.empty()) {
            // We did not find it, push the states back on the list and return false
            while (original.size() > 0) {
              owner.activities.push(original.top());
              original.pop();
            }

            owner.activities.push(last);

            return false;
          }

          // We did find it, call on end to everything and free memory
          while (original.size() > 0) {
            Activity* top = original.top();
            top->onEnd();
            delete top;
            original.pop();
          }

          // Remove next from the activity stack
          owner.activities.pop();

          swoosh::Segue* effect = new T(DurationType::value(), last, next);

          effect->onStart();

          owner.activities.push(effect);

          return true;
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
          T segueResolve;
          segueResolve.delegateActivityPush(owner, std::forward<Args>(args)...);
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

    template<typename T>
    const bool queuePop() {
      // Have to have more than 1 on the stack...
      bool hasLast = (activities.size() > 1);
      if (!hasLast || segueAction != SegueAction::NONE) return false;

      segueAction = SegueAction::POP;
      T segueResolve;
      segueResolve.delegateActivityPop(*this);

      return true;
    }

    const bool queuePop() {
      bool hasMore = (activities.size() > 0);

      if (!hasMore || segueAction != SegueAction::NONE) return false;

      willLeave = true;

      return true;
    }

    template <typename T, bool U>
    struct ResolveRewindSegueIntent
    {
      ResolveRewindSegueIntent(ActivityController& owner) {
        static_assert("Swoosh could not handle the segue intent");
      }

      bool RewindSuccessful;
    };

    template<typename T>
    struct ResolveRewindSegueIntent<T, false>
    {
      bool RewindSuccessful;

      template<typename... Args >
      ResolveRewindSegueIntent(ActivityController& owner, Args&&... args) {
        if (owner.segueAction == SegueAction::NONE) {
          owner.segueAction = SegueAction::POP;
          T segueResolve;
          RewindSuccessful = segueResolve.delegateActivityRewind(owner, std::forward<Args>(args)...);
        }
      }
    };

    template<typename T>
    struct ResolveRewindSegueIntent<T, true>
    {
      bool RewindSuccessful;

      template<typename... Args>
      ResolveRewindSegueIntent(ActivityController& owner, Args&&... args) {
        std::stack<Activity*> original;

        bool hasLast = (owner.activities.size() > 0);

        if (!hasLast) { RewindSuccessful = false; return; }

        Activity* next = owner.activities.top();

        while (dynamic_cast<T*>(next) == 0 && owner.activities.size() > 1) {
          original.push(next);
          activities.pop();
          next = owner.activities.top();
        }

        if (activities.empty()) {
          // We did not find it, push the states back on the list and return false
          while (original.size() > 0) {
            activities.push(original.top());
            original.pop();
          }

          RewindSuccessful = false;
          return;
        }

        next->onResume();
      }
    };

    template<typename T, typename... Args>
    bool queueRewind(Args&&... args) {
      ResolveRewindSegueIntent<T, ActivityTypeQuery<T>::value> intent(*this, std::forward<Args>(args)...);
      return intent.RewindSuccessful;
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

  namespace intent {
    template<int val = 0>
    struct seconds
    {
      static sf::Time value() { return sf::seconds(val); }
    };

    template<sf::Int32 val = 0>
    struct milliseconds
    {
      static sf::Time value() { return sf::milliseconds(val); }
    };

    template<sf::Int64 val = 0>
    struct microseconds
    {
      static sf::Time value() { return sf::microseconds(val); }
    };

    /*
    shorthand notations*/

    template<typename T, typename DurationType = seconds<1>>
    using segue = ActivityController::segue<T, DurationType>;

    template<int val = 0>
    using sec = seconds<val>;

    template<sf::Int32 val = 0>
    using milli = milliseconds<val>;

    template<sf::Int64 val = 0>
    using micro = microseconds<val>;
  }
}