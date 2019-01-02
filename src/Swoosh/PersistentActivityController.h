#pragma once
#include "Activity.h"
#include "Segue.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <functional>
#include <utility>


namespace swoosh {
  class PersistentActivityController {
    friend class swoosh::Segue;

  private:
    typedef std::map<std::string, Activity*> ActivityMap;
    typedef ActivityMap::iterator ActivityIter;
    ActivityMap activities;
    ActivityIter currentActivity; // sometimes reffered to as "last"
    ActivityIter nextActivity;
    swoosh::Segue* effect;

    mutable sf::RenderTexture* surface;
    sf::RenderWindow& handle;
    sf::Vector2u initWindowSize;

    bool willLeave;

    enum SegueAction {
      NEWTYPE,
      OLDTYPE,
      NONE
    } segueAction;

    template<typename T>
    const bool hasActivity() const {
      return (activities.find(typeid(T).name) != activities.end());
    }

    template<typename T>
    Activity* getActivity() const {
      return activities[typeid(T).name];
    }

  public:
    PersistentActivityController(sf::RenderWindow& window) : handle(window) {
      initWindowSize = handle.getSize();

      surface = new sf::RenderTexture();
      surface->create((unsigned int)handle.getSize().x, (unsigned int)handle.getSize().y);
      willLeave = false;
      segueAction = SegueAction::NONE;
    }

    ~PersistentActivityController() {
      if (segueAction != SegueAction::NONE) {
        delete effect;

        for (ActivityIter iter = activites.begin(); iter != activities.end(); iter++) {
          delete iter->second;
        }

        activies.clear();
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
      template<typename U>
      class to {
      public:

        to() { ; }

        template<typename... Args >
        void delegateActivityNext(PersistentActivityController& owner, Args&&... args) {
          bool hasLast = (owner.currentActivity != owner.activities.end());
          Activity* last = hasLast ? owner.currentActivity->second : nullptr;

          Activity* next; 
          
          if (owner.hasActivity<T>()) {
            owner.segueAction = SegueAction::OLDTYPE;
            next = owner.getActivity<T>();
          }
          else {
            next = new U(owner, std::forward<Args>(args)...);
            next->setView(owner.handle.getDefaultView());
            owner.activities.insert({ typeid(T).name, next });
            owner.segueAction = SegueAction::NEWTYPE;
          }

          owner.effect = new T(DurationType::value(), last, next);
          owner.effect->setActivityViewFunc = &PersistentActivityController::setActivityView;
          owner.effect->resetViewFunc = &PersistentActivityController::resetView;

          owner.effect->onStart();
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
    struct ResolveSegueIntent
    {
      ResolveSegueIntent(PersistentActivityController& owner) {
        static_assert("Swoosh could not handle the segue intent");
      }
    };

    template<typename T>
    struct ResolveSegueIntent<T, false>
    {
      template<typename... Args >
      ResolveSegueIntent(PersistentActivityController& owner, Args&&... args) {
        if (owner.segueAction == SegueAction::NONE) {
          T segueResolve;
          segueResolve.delegateActivityNext(owner, std::forward<Args>(args)...);
        }
      }
    };

    template<typename T>
    struct ResolveSegueIntent<T, true>
    {
      template<typename... Args>
      ResolveSegueIntent(PersistentActivityController& owner, Args&&... args) {
        bool hasLast = (owner.currentActivity != owner.activities.end());
        Activity* last = hasLast ? owner.currentActivity->second : nullptr;
        Activity* next;
        
        if (owner.hasActivity<T>()) {
          owner.segueAction = SegueAction::OLDTYPE;
          next = owner.getActivity<T>();
          next->onResume();
        }
        else {
          next = new T(owner, std::forward<Args>(args)...);
          next->setView(owner.handle.getDefaultView());
          owner.segueAction = SegueAction::NEWTYPE;
          owner.activities.insert({ typeid(T).name, next });
          next->onStart();
        }

        if (hasLast) {
          last->onExit();
        }
      }
    };

    template<typename T, typename... Args>
    void goTo(Args&&... args) {
      ResolveSegueIntent<T, ActivityTypeQuery<T>::value> intent(*this, std::forward<Args>(args)...);
    }

    void update(double elapsed) {
      if (activities.size() == 0)
        return;

      currentActivity->second->onUpdate(elapsed);

      if (segueAction != SegueAction::NONE) {
        if (effect->timer.getElapsed().asMilliseconds() >= effect->duration.asMilliseconds()) {
          endSegue(effect);
        }
      }
    }

    void draw() {
      if (activities.size() == 0)
        return;

      surface->setView(currentActivity->second->view);
      currrentActivity->second->onDraw(*surface);

      surface->display();

      // Capture buffer in a drawable context
      sf::Sprite post(surface->getTexture());

      // drawbuffer on top of the scene
      handle.draw(post);

      // Prepare buffer for next cycle
      surface->clear(sf::Color::Transparent);
    }

    void draw(sf::RenderTexture& external) {
      if (activities.size() == 0)
        return;

      external.setView(currentActivity->second->view);
      currentActivity->second->onDraw(external);
    }

  private:
    void setActivityView(sf::RenderTexture& surface, Activity* activity) {
      surface.setView(activity->getView());
    }

    void resetView(sf::RenderTexture& surface) {
      surface.setView(handle.getDefaultView());
    }

    void endSegue(swoosh::Segue* segue) {
      segue->onEnd();

      if (segueAction == SegueAction::NEWTYPE) {
        nextActivity->second->onStart(); // new item on stack first time call
      }
      else if (segueAction == SegueAction::OLDTYPE) {
        // We're removing an item from the stack
        currentActivity->second->onEnd();
        nextActivity->second->onResume();
      }

      delete segue;
      segueAction = SegueAction::NONE;
      currentActivity = nextActivity;
  };

  namespace intent {
    enum direction : int {
      left, right, up, down, max
    };

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