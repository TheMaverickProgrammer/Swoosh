#pragma once

#include "Activity.h"
#include "Segue.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <list>
#include <functional>
#include <utility>
#include <cstddef>

namespace swoosh {
  class CopyWindow; //!< forward decl

  class ActivityController {
    friend class swoosh::Segue;

  private:
    swoosh::Activity* last{ nullptr }; //!< Pointer of the last activity
    std::stack<swoosh::Activity*> activities; //!< Stack of activities
    sf::RenderWindow& handle; //!< sfml window reference
    sf::Vector2u virtualWindowSize; //!< Window size requested to render with
    bool willLeave{}; //!< If true, the activity will leave
    bool useShaders{ true }; //!< If false, segues can considerately use shader effects
    mutable sf::RenderTexture* surface{ nullptr }; //!< Render surface to draw to

    //!< Useful for state management and skipping need for dynamic casting
    enum class SegueAction : int {
      pop = 0,
      push,
      replace,
      none
    } segueAction;

    //!< Useful for state management
    enum class StackAction : int {
      pop = 0,
      push,
      replace,
      none
    } stackAction;

    quality qualityLevel{ quality::realtime }; //!< requested render quality

  public:
    /**
      @brief constructs the activity controller, sets the virtual window size to the window, and initializes default values
    */
    ActivityController(sf::RenderWindow& window) : handle(window) {
      virtualWindowSize = handle.getSize();

      surface = new sf::RenderTexture();
      surface->create((unsigned int)handle.getSize().x, (unsigned int)handle.getSize().y);
      willLeave = false;
      segueAction = SegueAction::none;
      stackAction = StackAction::none;

      last = nullptr;
    }

    /**
      @brief constructs the activity controller, sets the virtual window size to the user's desired size, and initializes default values
    */
    ActivityController(sf::RenderWindow& window, sf::Vector2u virtualWindowSize) : handle(window) {
      this->virtualWindowSize = virtualWindowSize;

      surface = new sf::RenderTexture();
      surface->create((unsigned int)virtualWindowSize.x, (unsigned int)virtualWindowSize.y);
      willLeave = false;
      segueAction = SegueAction::none;
      stackAction = StackAction::none;

      last = nullptr;
    }

    /**
      @brief Deconstructor deletes all activities and surfaces cleanly
    */
    virtual ~ActivityController() {
      if (segueAction != SegueAction::none) {
        swoosh::Segue* effect = static_cast<swoosh::Segue*>(activities.top());
        delete effect;
        activities.pop();
      }

      while (!activities.empty()) {
        swoosh::Activity* activity = activities.top();
        activities.pop();
        delete activity;
      }

      delete surface;
    }

    /**
      @brief Returns the virtual window size set at construction
    */
    const sf::Vector2u getVirtualWindowSize() const {
      return virtualWindowSize;
    }

    /**
      @brief Returns the render window
    */
    sf::RenderWindow& getWindow() {
      return handle;
    }

    /**
      @brief Returns the render surface pointer
    */
    sf::RenderTexture* getSurface() {
      return surface;
    }

    /**
      @brief Query the number of activities on the stack
    */
    const std::size_t getStackSize() const {
      return activities.size();
    }

    /**
      @brief Request the activity controller and segue effects to use the provided quality mode
      @param mode. Default is real-time and high performance. See: @quality enum class.
    */
    void optimizeForPerformance(quality mode) {
      qualityLevel = mode;
    }

    /**
      @brief Returns a quick check if the AC has been configured to use something other than realtime (default)
      @return false if qualityLevel == quality::realtime (default)
    */
    const bool isOptimizedForPerformance() const {
      return qualityLevel != quality::realtime;
    }

    /**
      @brief Request the activity controller and segue effects to use shaders
      @param enabled. Default is shader support. If false, segues should not use shaders.
    */
    void enableShaders(bool enabled) {
      useShaders = enabled;
    }

    /**
      @brief Returns a quick check if the AC has been configured to use shaders
      @return true by default unless manually disabled by `enableShaders(false)`
    */
    const bool isShadersEnabled() const {
      return useShaders;
    }

    /**
      @brief Query the requested quality mode
      
      This should be used by segue effects to provide alternative visuals for various-grade GPUs
    */
    const quality getRequestedQuality() const {
      return qualityLevel;
    }

    /**
      @class segue
      @brief This class is used internally to hide a lot of ugliness that makes the segue transition API easier to read
    */
    template<typename T, typename DurationType>
    class segue {
    public:
      /**
        @brief This will start a POP state for the activity controller and creates a segue object onto the stack

        e.g. queuePop<segue<FadeOut>>(); // will transition from the current scene to the last with a fadeout effect
      */
      void delegateActivityPop(ActivityController& owner) {
        swoosh::Activity* last = owner.activities.top();
        owner.activities.pop();

        swoosh::Activity* next = owner.activities.top();
        owner.activities.pop();

        swoosh::Segue* effect = new T(DurationType::value(), last, next);
        sf::Vector2u windowSize = owner.getVirtualWindowSize();
        sf::View view(sf::FloatRect(0, 0, (float)windowSize.x, (float)windowSize.y));
        effect->setView(view);

        effect->setActivityViewFunc = &ActivityController::setActivityView;
        effect->resetViewFunc = &ActivityController::resetView;
        effect->onStart();
        effect->started = true;
        owner.activities.push(effect);
      }

      /**
       @class to
       @brief This class is used internally to make the segue API readable

       e.g. segue<effect>::to<LevelType> // pretty readable, transition to the level with the desired segue effect
     */
      template<typename U>
      class to {
      public:

        to() { ; }

        /**
          @brief This will start a PUSH state for the activity controller and creates a segue object onto the stack
        */
        template<typename... Args >
        void delegateActivityPush(ActivityController& owner, Args&&... args) {
          bool hasLast = (owner.activities.size() > 0);
          swoosh::Activity* last = hasLast ? owner.activities.top() : owner.generateActivityFromWindow();
          swoosh::Activity* next = new U(owner, std::forward<Args>(args)...);

          swoosh::Segue* effect = new T(DurationType::value(), last, next);
          sf::Vector2u windowSize = owner.getVirtualWindowSize();
          sf::View view(sf::FloatRect(0.f, 0.f, (float)windowSize.x, (float)windowSize.y));
          effect->setView(view);

          effect->setActivityViewFunc = &ActivityController::setActivityView;
          effect->resetViewFunc = &ActivityController::resetView;

          effect->onStart();
          effect->started = true;
          owner.activities.push(effect);
        }

        /**
          @brief This will start a REWIND state for the activity controller and creates a segue object onto the stack
          @return True if the target activity type T is found in the stack, false otherwise. Found activities transform into segues.

          This must use dynamic casting to find the target type T* in the activity stack.
          If no activity is found, all the activities are carefully placed back into the stack.
          If the target ativity is found, the traverse activities are deleted.
        */
        template<typename... Args >
        bool delegateActivityRewind(ActivityController& owner, Args&&... args) {
          std::stack<swoosh::Activity*> original;

          bool hasMore = (owner.activities.size() > 1);

          if (!hasMore) { return false; }

          swoosh::Activity* last = owner.activities.top();
          owner.activities.pop();

          swoosh::Activity* next = owner.activities.top();

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
            swoosh::Activity* top = original.top();
            top->onEnd();
            delete top;
            original.pop();
          }

          // Remove next from the activity stack
          owner.activities.pop();

          swoosh::Segue* effect = new T(DurationType::value(), last, next);
          sf::Vector2u windowSize = owner.getVirtualWindowSize();
          sf::View view(sf::FloatRect(0.0f, 0.0f, (float)windowSize.x, (float)windowSize.y));
          effect->setView(view);
          
          effect->setActivityViewFunc = &ActivityController::setActivityView;
          effect->resetViewFunc = &ActivityController::resetView;

          effect->onStart();
          effect->started = true;
          owner.activities.push(effect);

          return true;
        }
      };
    };

    /**
      @class IsSegueType
      @brief This template structure determines if this input type derives from Activity or Segues
    */
    template <class T>
    struct IsSegueType
    {
      static char is_to(swoosh::Activity*) { return 0; }

      static double is_to(...) { return 0; }

      static T* t;

      // Returns true if type is Segue, false if otherwise
      enum { value = sizeof(is_to(t)) != sizeof(char) };
    };

    /** 
      @class ResolvePushSegueIntent
      @brief This template structure will perform different acttions if the 2nd template argument is true or false
    */
    template <typename T, bool U>
    struct ResolvePushSegueIntent; // left empty

    /**
      @class ResolvePushSegueIntent<T, true>
      @brief If type is a segue, resolves the intention by calling delegateActivityPush()
    */
    template<typename T>
    struct ResolvePushSegueIntent<T, true>
    {
      template<typename... Args >
      ResolvePushSegueIntent(ActivityController& owner, Args&&... args) {
        if (owner.segueAction == SegueAction::none) {
          owner.segueAction = SegueAction::push;
          T segueResolve;
          segueResolve.delegateActivityPush(owner, std::forward<Args>(args)...);
        }
      }
    };

    /**
      @class ResolvePushSegueIntent<T, false>
      @brief If type is NOT a segue, resolves the intention by directly modifying the stack
    */
    template<typename T>
    struct ResolvePushSegueIntent<T, false>
    {
      template<typename... Args>
      ResolvePushSegueIntent(ActivityController& owner, Args&&... args) {
        if (owner.segueAction != SegueAction::none) return;

        swoosh::Activity* next = new T(owner, std::forward<Args>(args)...);

        if (owner.last == nullptr && owner.activities.size() != 0) {
          owner.last = owner.activities.top();
        }

        owner.activities.push(next);
        owner.stackAction = StackAction::push;
      }
    };

    /**
      @brief Immediately pushes a segue or activity onto the stack depending on the resolved class type
    */
    template<typename T, typename... Args>
    void push(Args&&... args) {
      ResolvePushSegueIntent<T, IsSegueType<T>::value> intent(*this, std::forward<Args>(args)...);
    }

    /**
    @brief Immediately replaces the current activity on the stack with a segue or activity depending on the resolved class type
    */
    template<typename T, typename... Args>
    void replace(Args&&... args) {
      size_t before = this->activities.size();
      ResolvePushSegueIntent<T, IsSegueType<T>::value> intent(*this, std::forward<Args>(args)...);
      size_t after = this->activities.size();

      // quick feature hack:
      // We check if the push intent was resolved (+1 activity stack)
      // And then figure out which event was called (segue or direct stack modification?)
      // And flip that flag to become REPLACE
      if (before < after) {
        if (segueAction != SegueAction::none) {
          segueAction = SegueAction::replace;
        }
        else if (stackAction != StackAction::none) {
          stackAction = StackAction::replace;
        }
      }
    }

    /**
      @brief Tries to pop the activity of the stack that may be replaced with a segue to transition to the previous activity on the stack
      @return true if we are able to pop, false if there less than 1 item on the stack or in the middle of a segue effect
    */
    template<typename T>
    const bool pop() {
      // Have to have more than 1 on the stack to have a transition effect...
      bool hasLast = (activities.size() > 1);
      if (!hasLast || segueAction != SegueAction::none) return false;

      segueAction = SegueAction::pop;
      T segueResolve;
      segueResolve.delegateActivityPop(*this);

      return true;
    }

    /**
     @brief Tries to pop the activity of the stack
     @return true if we are able to pop, false if there are no more items on the stack or in the middle of a segue effect
   */
    const bool pop() {
      bool hasMore = (activities.size() > 0);

      if (!hasMore || segueAction != SegueAction::none) return false;

      willLeave = true;

      return true;
    }


    /**
      @class ResolveRewindSegueIntent
      @brief This template structure will perform different acttions if the 2nd template argument is true or false
    */
    template <typename T, bool U>
    struct ResolveRewindSegueIntent
    {
      ResolveRewindSegueIntent(ActivityController& owner) {
      }

      bool RewindSuccessful;
    };

    /**
      @class ResolveRewindSegueIntent<T, true>
      @brief If type is a segue, resolves the intention by calling delegateActivityRewind()
    */
    template<typename T>
    struct ResolveRewindSegueIntent<T, true>
    {
      bool RewindSuccessful;

      template<typename... Args >
      ResolveRewindSegueIntent(ActivityController& owner, Args&&... args) {
        if (owner.segueAction == SegueAction::none) {
          owner.segueAction = SegueAction::pop;
          T segueResolve;
          RewindSuccessful = segueResolve.delegateActivityRewind(owner, std::forward<Args>(args)...);
        }
      }
    };

    /**
    @class ResolveRewindSegueIntent<T, false>
    @brief If type is not a segue, looks for the matching activity. If it is not found, it does not rewind to that activity.

    If a rewind is successful, all spanned activities are deleted.
  */
    template<typename T>
    struct ResolveRewindSegueIntent<T, false>
    {
      bool RewindSuccessful;

      template<typename... Args>
      ResolveRewindSegueIntent(ActivityController& owner, Args&&... args) {
        std::stack<swoosh::Activity*> original;

        bool hasLast = (owner.activities.size() > 0);

        if (!hasLast) { RewindSuccessful = false; return; }

        swoosh::Activity* next = owner.activities.top();

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

          RewindSuccessful = false;
          return;
        }

        next->onResume();
      }
    };

    /**
     @brief Tries to rewind the activity to a target activity type T in the stack
     @return true if we are able to rewind (activity found), false if not found or in the middle of a transition
    */
    template<typename T, typename... Args>
    bool rewind(Args&&... args) {
      if (this->activities.size() <= 1) return false;

      ResolveRewindSegueIntent<T, IsSegueType<T>::value> intent(*this, std::forward<Args>(args)...);
      return intent.RewindSuccessful;
    }

    /**
      @brief Returns the current activity pointer. Nullptr if no acitivty exists on the stack.
    */
    const swoosh::Activity* getCurrentActivity() const {
      if (getStackSize() > 0) return activities.top();
      return nullptr;
    }

    /**
     @brief Updates the current activity or segue. Will manage the segue transition states.
     @param elapsed. Time in seconds

     If optimized for performance and the quality mode is set to `mobile`, will not update the 
     activities in the segue to help increase performance on lower end hardware
    */
    void update(double elapsed) {
      if (activities.size() == 0)
        return;

      if (willLeave) {
        executePop();
        willLeave = false;
      }

      if (activities.size() == 0)
        return;

      if (stackAction == StackAction::push || stackAction == StackAction::replace) {
        if (activities.size() > 1 && last) {
          last->onExit();

          if (stackAction == StackAction::replace) {
            auto top = activities.top();
            activities.pop(); // top
            activities.pop(); // last, to be replaced by top
            activities.push(top); // fin
            delete last;
          }

          last = nullptr;
        }

        activities.top()->onStart();
        activities.top()->started = true;

        stackAction = StackAction::none;
      }

      if (segueAction != SegueAction::none) {
        swoosh::Segue* segue = static_cast<swoosh::Segue*>(activities.top());

        if (getRequestedQuality() == quality::mobile) {
          segue->timer.update(sf::seconds(static_cast<float>(elapsed)));
        }
        else {
          segue->onUpdate(elapsed);
        }

        if (segue->timer.getElapsed().asMilliseconds() >= segue->duration.asMilliseconds()) {
          endSegue(segue);
        }
      }
      else {
        activities.top()->onUpdate(elapsed);
      }
    }

    /**
     @brief Draws the current activity or segue and displays the result onto the window
    */
    void draw() {
      if (activities.size() == 0)
        return;

      surface->setView(activities.top()->view);
      activities.top()->onDraw(*surface);

      surface->display();

      // Capture buffer in a drawable context
      sf::Sprite post(surface->getTexture());

      // Fill in the bg color
      handle.clear(activities.top()->bgColor);

      // drawbuffer on top of the scene
      handle.draw(post);

      // Prepare buffer for next cycle
      surface->clear(sf::Color::Transparent);
    }

    /**
      @brief Draws the current activity or segue onto an external render buffer
      @param external. A render texture buffer to draw the content onto
     */
    void draw(sf::RenderTexture& external) {
      if (activities.size() == 0)
        return;

      external.setView(activities.top()->view);

      // Fill in the bg color
      handle.clear(activities.top()->bgColor);

      activities.top()->onDraw(external);
    }

  private:
    /**
      @brief Applies an activity's view onto the render surface. This is used internally for segues.

      This function is kept here to make the library files header-only and avoid linkage.
    */
    void setActivityView(sf::RenderTexture& surface, swoosh::Activity* activity) {
      surface.setView(activity->getView());
    }

    /**
     @brief Resets the render surface's view to the window's default view. This is used internally for segues.

     This function is kept here to make the library files header-only and avoid linkage.
   */
    void resetView(sf::RenderTexture& surface) {
      surface.setView(handle.getDefaultView());
    }

    /**
     @brief This function properly terminates an active segue and pushes the next activity onto the stack
   */
    void endSegue(swoosh::Segue* segue) {
      segue->onEnd();
      activities.pop();

      swoosh::Activity* next = segue->next;

      if (segueAction == SegueAction::pop || segueAction == SegueAction::replace) {
        // We're removing an item from the stack
        swoosh::Activity* last = segue->last;
        last->onEnd();

        if (next->started) {
          next->onResume();
        }
        else {
          // We may have never started this activity because it existed
          // in the activity stack before being used...
          next->onStart();
          next->started = true;
        }

        if (segueAction == SegueAction::replace) {
          activities.pop(); // remove last
        }

        delete last;
      }
      else if (segueAction == SegueAction::push) {
        next->onStart(); // new item on stack first time call
        next->started = true;
      }

      delete segue;
      activities.push(next);
      segueAction = SegueAction::none;
    }

    /**
       @brief When pop() is invoked, the pop is not executed immediately. It is deffered until it is safe to pop the activity off the stack.
     */
    void executePop() {
      swoosh::Activity* activity = activities.top();
      activity->onEnd();
      activities.pop();

      if (activities.size() > 0)
        activities.top()->onResume();

      delete activity;
    }

    /**
      @brief Copy's the screen buffer and creates a temporary "blank" activity
      @return the newly generated blank activity

      Used internally
    */
    inline Activity* generateActivityFromWindow();
  }; // ActivityController


  /**
    @class CopyWindow

    This special class will copy the window's framebuffer contents and use it as a temp scene
    even if the programmer didn't create one before using swoosh.

    Example use case: a segue fading in an intro screen when there were no previous scenes at launch

    This is best suited for all actions: push, pop, and replace.

    @warning This is a costly operation in SFML and can cause your program to stall for a split second 
             depending on your computer. You should also be sure you don't clear your window content
             until AFTER the AC's update loop
  */
  class CopyWindow : public Activity {
    sf::Texture framebuffer;
    sf::Sprite drawable;
    bool captured;

    void copyWindowContents() {
      if(captured) return;

      // get the window handle
      auto& window = getController().getWindow();

      // get all original view and viewport settings
      auto& view = window.getView();
      auto viewSize = view.getSize();
      auto viewportIntRect = window.getViewport(view);

      // calculate the view based on any viewport adjustments
      // because we will copy the viewport pixels and we don't want those in our re-rendered image
      sf::View newView = sf::View(sf::FloatRect((float)viewportIntRect.left, (float)viewportIntRect.top, (float)viewportIntRect.width, (float)viewportIntRect.height));

      // screen size in pixels
      sf::Vector2u windowSize = window.getSize();
      float w = (float)windowSize.x;
      float h = (float)windowSize.y;

      // copy screen contents
      framebuffer.create((unsigned int)w, (unsigned int)h);
      framebuffer.update(window);
      drawable.setTexture(framebuffer, true);

      // Use the view that cleanly renders the copied screen (as if a viewport never existed)
      setView(newView);

      // flag screen copy op as complete
      captured = true;
    }

  public:
    CopyWindow(ActivityController& ac) : Activity(&ac) { 
      captured = false;
     }

    virtual ~CopyWindow() { ; }

    void onStart()  override { copyWindowContents(); };
    void onLeave()  override { copyWindowContents(); };
    void onExit()   override { };
    void onEnter()  override { copyWindowContents(); };
    void onResume() override { };
    void onEnd()    override { };

    void onUpdate(double elapsed) override { };

    void onDraw(sf::RenderTexture& surface) override {
      surface.draw(drawable);
    }
  }; // CopyWindow

  // deferred implementation
  Activity* ActivityController::generateActivityFromWindow() {
    return new CopyWindow(*this);
  }

  // useful types in their own namespace
  namespace types {
    enum class direction : int {
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
