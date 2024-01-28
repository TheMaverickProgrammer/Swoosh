#pragma once
#include <Swoosh/Activity.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Timer.h>
#include <Swoosh/Renderers/Renderer.h>
#include <SFML/Graphics.hpp>
#include <stack>
#include <list>
#include <utility>
#include <cstddef>
#include <assert.h>

namespace swoosh
{
  class CopyWindow; //!< forward decl

  class ActivityController
  {
    friend class swoosh::Segue;
    friend class swoosh::Activity;

  public:
    // Forward decl.
    template<typename T, typename DurationType>
    class segue;

  private:
    // helper util for `hasPendingChanges`
    class pending_raii
    {
      bool &value;

    public:
      pending_raii(bool &source) : value(source)
      {
        // Behave like a spin-lock
        // If this true else-where, wait our turn to acquire it
        while (value)
        {
        };
        value = true;
      }

      ~pending_raii()
      {
        value = false;
      }

      void set(const bool newValue)
      {
        value = newValue;
      }
    };

    swoosh::Activity *last{nullptr};           //!< Pointer of the last activity
    std::stack<swoosh::Activity *> activities; //!< Stack of activities
    sf::RenderWindow &handle;                  //!< sfml window reference
    sf::Vector2u virtualWindowSize;            //!< Window size requested to render with
    bool hasPendingChanges{};                  //!< If true, the activity controller is mutating the stack
    bool isUpdating{};                         //!< If true, the controller is updating activities and may mutate the stack
    bool useShaders{true};                     //!< If false, segues can considerately use shader effects
    bool clearBeforeDraw{true};                //!< If true, clears the render target with the Activity's bg color
    mutable IRenderer *renderer{nullptr};      //!< Active renderer to submit draw events to
    std::size_t rendererIdx{0};                //!< Active renderer index
    const RendererEntries rendererEntries;     //!< All registered renderers

    //!< Useful for state management and skipping need for dynamic casting
    enum class SegueAction : int
    {
      none = 0,
      push,
      replace,
      pop,
      rewind
    } segueAction;

    //!< Useful for state management
    enum class StackAction : int
    {
      none = 0,
      push,
      replace,
      clear, // clears entire stack
      pop
    } stackAction;

    quality qualityLevel{quality::realtime}; //!< requested render quality

  public:
    /**
      @brief constructs the activity controller, sets the virtual window size to the window, and initializes default values
    */
    ActivityController(sf::RenderWindow &window, const RendererEntries &rendererEntries) : handle(window),
                                                                                           rendererEntries(rendererEntries)
    {
      virtualWindowSize = handle.getSize();

      assert(!rendererEntries.empty() && "ActivityController RenderEntries was empty!");
      renderer = &rendererEntries.front().renderer;

      segueAction = SegueAction::none;
      stackAction = StackAction::none;
      last = nullptr;
    }

    /**
      @brief constructs the activity controller, sets the virtual window size to the user's desired size, and initializes default values
    */
    ActivityController(sf::RenderWindow &window, sf::Vector2u virtualWindowSize, const RendererEntries &rendererEntries) : handle(window),
                                                                                                                           rendererEntries(rendererEntries)
    {
      this->virtualWindowSize = virtualWindowSize;

      assert(!rendererEntries.empty() && "ActivityController RenderEntries was empty!");
      renderer = &rendererEntries.front().renderer;

      segueAction = SegueAction::none;
      stackAction = StackAction::none;

      last = nullptr;
    }

    /**
      @brief Deconstructor deletes all activities and surfaces cleanly
    */
    virtual ~ActivityController()
    {
      executeClearStackSafely();
    }
    
    /**
      @brief Returns the virtual window size set at construction
    */
    const sf::Vector2u getVirtualWindowSize() const
    {
      return virtualWindowSize;
    }

    /**
      @brief Returns the render window
    */
    sf::RenderWindow &getWindow()
    {
      return handle;
    }

    /**
      @brief Query the number of activities on the stack
    */
    const std::size_t getStackSize() const
    {
      return activities.size();
    }

    /**
      @brief Query if the stack is undergoing a change
      Useful for multithreaded applications
    */
    const bool pendingChanges() const
    {
      return hasPendingChanges;
    }

    /**
      @brief Query the number of registered renderers
    */
    const std::size_t getNumOfRenderers() const
    {
      return rendererEntries.size();
    }

    /**
      @brief Query the active renderer index
    */
    const std::size_t getCurrentRendererIndex() const
    {
      return rendererIdx;
    }

    /**
      @brief Query the active renderer name
    */
    const std::string getCurrentRendererName() const
    {
      return std::next(rendererEntries.begin(), rendererIdx)->name;
    }

    /**
      @brief Sets the active renderer to the entry at the given index parameter
      @param idx the base-0 index of the renderer in the RendererEntries list to use
      @return true if the renderer was set successfully, false if the index is invalid
    */
    bool setRenderer(std::size_t idx)
    {
      if (idx < 0 || idx >= getNumOfRenderers())
        return false;

      rendererIdx = idx;
      renderer = &std::next(rendererEntries.begin(), idx)->renderer;

      return true;
    }

    /**
      @brief Query a provided renderer by its entry index
      @param idx the base-0 index of the renderer in the RendererEntries list
      @return A pointer to the renderer if valid, nullptr if invalid.
    */
    IRenderer* getRenderer(std::size_t idx) {
      if (idx < 0 || idx >= getNumOfRenderers())
        return nullptr;

      return &std::next(rendererEntries.begin(), idx)->renderer;
    }

    /**
      @brief Query a provided renderer by its typename `RendererT`
      @return A pointer to the renderer that matches first, nullptr if not found.
    */
    template<typename RendererT>
    RendererT* getRenderer() {
      auto query = [this](RendererEntry& entry) {
        return typeid(entry.renderer) == typeid(RendererT);
      };

      auto iter = 
        std::find_if(rendererEntries.begin(), rendererEntries.end(), query);

      if (iter == rendererEntries.end())
        return nullptr;

      return &iter->renderer;
    }

    /**
      @brief Request the activity controller to clear the render target (window or texture) before drawing.
      @param enabled. Default is true. If false, the programmer must control clearing the targets themselves.
    */
    void clearRenderTargetBeforeDraw(bool enabled)
    {
      this->clearBeforeDraw = enabled;
    }

    /**
      @brief Request the activity controller and segue effects to use the provided quality mode
      @param mode. Default is real-time and high performance. See: @quality enum class.
    */
    void optimizeForPerformance(quality mode)
    {
      qualityLevel = mode;
    }

    /**
      @brief Returns a quick check if the AC has been configured to use something other than realtime (default)
      @return false if qualityLevel == quality::realtime (default)
    */
    const bool isOptimizedForPerformance() const
    {
      return qualityLevel != quality::realtime;
    }

    /**
      @brief Request the activity controller and segue effects to use shaders
      @param enabled. Default is shader support. If false, segues should not use shaders.
    */
    void enableShaders(bool enabled)
    {
      useShaders = enabled;
    }

    /**
      @brief Returns a quick check if the AC has been configured to use shaders
      @return true by default unless manually disabled by `enableShaders(false)`
    */
    const bool isShadersEnabled() const
    {
      return useShaders;
    }

    /**
      @brief Query the requested quality mode

      This should be used by segue effects to provide alternative visuals for various-grade GPUs
    */
    const quality getRequestedQuality() const
    {
      return qualityLevel;
    }

    /**
      @class segue
      @brief This class is used internally to hide a lot of ugliness that makes the segue transition API easier to read
    */
    template <typename T, typename DurationType>
    class segue
    {
    public:
      /**
        @brief This will start a POP state for the activity controller and creates a segue object onto the stack

        e.g. queuePop<segue<FadeOut>>(); // will transition from the current scene to the last with a fadeout effect
      */
      template<typename... Args>
      void delegateActivityPop(ActivityController& owner, Args&&... args)
      {
        pending_raii _(owner.hasPendingChanges);

        swoosh::Activity *last = owner.activities.top();
        owner.activities.pop();

        swoosh::Activity *next = owner.activities.top();
        owner.activities.pop();

        next->yieldable.context = Context(std::forward<Args>(args)...);

        swoosh::Segue *effect = new T(DurationType::value(), last, next);
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
      template <typename U>
      class to
      {
      public:
        using activity_type = U;

        to() { ; }

        /**
          @brief This will start a PUSH state for the activity controller and creates a segue object onto the stack
        */
        template <typename... Args>
        Yieldable* delegateActivityPush(ActivityController &owner, Args&&... args)
        {
          pending_raii _(owner.hasPendingChanges);

          bool hasLast = (owner.activities.size() > 0);
          swoosh::Activity *last = hasLast ? owner.activities.top() : owner.generateActivityFromWindow();
          swoosh::Activity *next = new U(owner, std::forward<Args>(args)...);

          swoosh::Segue *effect = new T(DurationType::value(), last, next);
          sf::Vector2u windowSize = owner.getVirtualWindowSize();
          sf::View view(sf::FloatRect(0.f, 0.f, (float)windowSize.x, (float)windowSize.y));
          effect->setView(view);

          effect->setActivityViewFunc = &ActivityController::setActivityView;
          effect->resetViewFunc = &ActivityController::resetView;

          effect->onStart();
          effect->started = true;
          owner.activities.push(effect);

          return &last->yieldable;
        }

        /**
          @brief This will start a REWIND state for the activity controller and creates a segue object onto the stack
          @return True if the target activity type T is found in the stack, false otherwise. Found activities transform into segues.

          This must use dynamic casting to find the target type T* in the activity stack.
          If no activity is found, all the activities are carefully placed back into the stack.
          If the target ativity is found, the traverse activities are deleted.
        */
        template <typename... Args>
        bool delegateActivityRewind(ActivityController &owner, Args&&... args)
        {
          pending_raii _(owner.hasPendingChanges);

          std::stack<swoosh::Activity*> original;

          bool hasMore = (owner.activities.size() > 1);

          if (!hasMore)
          {
            return false;
          }

          swoosh::Activity *last = owner.activities.top();
          owner.activities.pop();

          swoosh::Activity *next = owner.activities.top();

          while (dynamic_cast<T *>(next) == 0 && owner.activities.size() > 1)
          {
            original.push(next);
            owner.activities.pop();
            next = owner.activities.top();
          }

          if (owner.activities.empty())
          {
            // We did not find it, push the states back on the list and return false
            while (original.size() > 0)
            {
              owner.activities.push(original.top());
              original.pop();
            }

            owner.activities.push(last);

            return false;
          }

          // We did find it, call on end to everything and free memory
          // Thenables are invalid
          while (original.size() > 0)
          {
            swoosh::Activity *top = original.top();
            top->onEnd();

            delete top;

            original.pop();
          }

          // Remove next from the activity stack
          owner.activities.pop();

          swoosh::Segue *effect = new T(DurationType::value(), last, next);
          sf::Vector2u windowSize = owner.getVirtualWindowSize();
          sf::View view(sf::FloatRect(0.0f, 0.0f, (float)windowSize.x, (float)windowSize.y));
          effect->setView(view);

          effect->setActivityViewFunc = &ActivityController::setActivityView;
          effect->resetViewFunc = &ActivityController::resetView;

          effect->onStart();
          effect->started = true;
          owner.activities.push(effect);

          next->yieldable.context = Context(std::forward<Args>(args)...);

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
      static char is_to(swoosh::Activity *) { return 0; }

      static double is_to(...) { return 0; }

      static T *t;

      // Returns true if type is Segue, false if otherwise
      enum
      {
        value = sizeof(is_to(t)) != sizeof(char)
      };
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
    template <typename T>
    struct ResolvePushSegueIntent<T, true>
    {
      using activity_type = typename T::activity_type;

      Yieldable* yieldable{ nullptr };

      template <typename... Args>
      ResolvePushSegueIntent(ActivityController &owner, Args &&...args)
      {
        if (owner.segueAction != SegueAction::none) {
          yieldable = &Yieldable::dummy;
          return;
        }

        owner.segueAction = SegueAction::push;
        T segueResolve;

        yieldable = &segueResolve.delegateActivityPush(owner, std::forward<Args>(args)...)->reset();
      }
    };

    /**
      @class ResolvePushSegueIntent<T, false>
      @brief If type is NOT a segue (Activity), resolves the intention by directly modifying the stack
    */
    template <typename T>
    struct ResolvePushSegueIntent<T, false>
    {
      using activity_type = T;

      Yieldable* yieldable{ nullptr };

      template <typename... Args>
      ResolvePushSegueIntent(ActivityController &owner, Args &&...args)
      {
        if (owner.segueAction != SegueAction::none) {
          yieldable = &Yeildable::dummy;
          return;
        }

        swoosh::Activity *next = new T(owner, std::forward<Args>(args)...);

        if (owner.last == nullptr && owner.activities.size() != 0)
        {
          owner.last = owner.activities.top();
        }

        pending_raii _(owner.hasPendingChanges);
        owner.activities.push(next);
        owner.stackAction = StackAction::push;

        yieldable = &owner.last->yieldable.reset();
      }
    };

    // Shorthand resolver
    template<typename T>
    using Intent = ResolvePushSegueIntent<T, IsSegueType<T>::value>;

    /**
      @brief Immediately pushes a segue or activity onto the stack depending on the resolved class type
    */
    template <typename T, typename... Args>
    Yieldable& push(Args&&... args)
    {
      Intent<T> intent(*this, std::forward<Args>(args)...);
      return *(intent.yieldable);
    }

    /**
    @brief Immediately replaces the current activity on the stack with a segue or activity depending on the resolved class type
    */
    template <typename T, typename... Args>
    void replace(Args&&...args)
    {
      const size_t before = activities.size();
      ResolvePushSegueIntent<T, IsSegueType<T>::value> intent(*this, std::forward<Args>(args)...);
      const size_t after = activities.size();

      // quick feature hack:
      // We check if the push intent was resolved (+1 activity stack)
      // And then figure out which event was called (segue or direct stack modification?)
      // And flip that flag to become REPLACE
      if (before < after)
      {
        if (segueAction != SegueAction::none)
        {
          segueAction = SegueAction::replace;
        }
        else if (stackAction != StackAction::none)
        {
          stackAction = StackAction::replace;
        }
      }
    }

    /**
      @brief Tries to pop the activity of the stack that may be replaced with a segue to transition to the previous activity on the stack
      @return true if we are able to pop, false if there less than 1 item on the stack or in the middle of a segue effect
    */
    template <typename T, typename... Args>
    const bool pop(Args&&... args)
    {
      // Have to have more than 1 on the stack to have a transition effect...
      const size_t activity_len = activities.size();
      const bool hasLast = (activity_len > 1);
      if (!hasLast || segueAction != SegueAction::none)
        return false;

      segueAction = SegueAction::pop;
      T segueResolve;
      segueResolve.delegateActivityPop(*this, std::forward<Args>(args)...);

      return true;
    }

    /**
     @brief Tries to pop the activity of the stack
     @return true if we are able to pop, false if there are no more items on the stack or in the middle of a segue effect
   */
    template<typename... Args>
    const bool pop(Args&&... args)
    {
      const bool hasMore = (activities.size() > 0);

      if (!hasMore || segueAction != SegueAction::none)
        return;

      last->yieldable.context = Context(std::forward<Args>(args)...);
      stackAction = StackAction::pop;
    }

    /**
      @class ResolveRewindSegueIntent
      @brief This template structure will perform different acttions if the 2nd template argument is true or false
    */
    template <typename T, bool U>
    struct ResolveRewindSegueIntent
    {
      ResolveRewindSegueIntent(ActivityController &owner) {}

      bool rewindSuccessful{};
    };

    /**
      @class ResolveRewindSegueIntent<T, true>
      @brief If type is a segue, resolves the intention by calling delegateActivityRewind()
    */
    template <typename T>
    struct ResolveRewindSegueIntent<T, true>
    {
      bool rewindSuccessful{};

      template <typename... Args>
      ResolveRewindSegueIntent(ActivityController &owner, Args &&...args)
      {
        if (owner.segueAction != SegueAction::none) return;

        owner.segueAction = SegueAction::rewind;
        T segueResolve;
        rewindSuccessful = segueResolve.delegateActivityRewind(owner, std::forward<Args>(args)...);
      }
    };

    /**
    @class ResolveRewindSegueIntent<T, false>
    @brief If type is not a segue, looks for the matching activity. If it is not found, it does not rewind to that activity.

    If a rewind is successful, all spanned activities are deleted.
  */
    template <typename T>
    struct ResolveRewindSegueIntent<T, false>
    {
      bool rewindSuccessful{};

      template <typename... Args>
      ResolveRewindSegueIntent(ActivityController &owner, Args &&...args)
      {
        std::stack<swoosh::Activity*> original;

        const bool hasLast = (owner.activities.size() > 0);

        if (!hasLast)
        {
          rewindSuccessful = false;
          return;
        }

        pending_raii _(owner.hasPendingChanges);

        swoosh::Activity *next = owner.activities.top();

        while (dynamic_cast<T*>(next) == 0 && owner.activities.size() > 1)
        {
          original.push(next);
          owner.activities.pop();
          next = owner.activities.top();
        }

        if (owner.activities.empty())
        {
          // We did not find it, push the states back on the list and return false
          while (original.size() > 0)
          {
            owner.activities.push(original.top());
            original.pop();
          }

          rewindSuccessful = false;
          return;
        }

        next->yieldable.context = Context(std::forward<Args>(args)...);
        next->handleYieldable();

        // Cleanup memory
        while (original.size() > 0) {
          Activity* top = original.top();
          top->onEnd();

          delete original.top();
          original.pop();
        }

        next->onResume();
      }
    };

    /**
     @brief Tries to rewind the activity to a target activity type T in the stack
     @return true if we are able to rewind (activity found), false if not found or in the middle of a transition
    */
    template <typename T, typename... Args>
    bool rewind(Args&&... args)
    {
      if (activities.size() <= 1)
        return false;

      ResolveRewindSegueIntent<T, IsSegueType<T>::value> intent(*this, std::forward<Args>(args)...);
      return intent.rewindSuccessful;
    }

    /**
      @brief Returns the current activity pointer. Nullptr if no acitivty exists on the stack.
    */
    const swoosh::Activity *getCurrentActivity() const
    {
      if (getStackSize() > 0)
        return activities.top();

      return nullptr;
    }

    /**
     @brief Updates the current activity or segue. Will manage the segue transition states.
     @param elapsed. Time in seconds

     If optimized for performance and the quality mode is set to `mobile`, will not update the
     activities in the segue to help increase performance on lower end hardware
    */
    void update(double elapsed)
    {
      pending_raii _(isUpdating);

      if (activities.size() == 0)
        return;

      // Stack actions imply no segue
      if (stackAction == StackAction::pop)
      {
        executePop();
        stackAction = StackAction::none;
        return;
      } 
      else if (stackAction == StackAction::clear)
      {
        executeClearStackSafely();
        return;
      } 
      else if (stackAction == StackAction::push || stackAction == StackAction::replace)
      {
        if (activities.size() > 1 && last)
        {
          last->onExit();

          if (stackAction == StackAction::replace)
          {
            pending_raii _(hasPendingChanges);

            auto top = activities.top();
            activities.pop();     // top
            activities.pop();     // last, to be replaced by top
            activities.push(top); // fin
            delete last;
          }

          last = nullptr;
        }

        activities.top()->onStart();
        activities.top()->started = true;

        stackAction = StackAction::none;
      }

      // Check for segues
      if (segueAction != SegueAction::none)
      {
        swoosh::Segue *segue = static_cast<swoosh::Segue *>(activities.top());

        if (getRequestedQuality() == quality::mobile)
        {
          segue->timer.update(sf::seconds(static_cast<float>(elapsed)));
        }
        else
        {
          segue->onUpdate(elapsed);
        }

        if (segue->timer.getElapsed().asMilliseconds() >= segue->duration.asMilliseconds())
        {
          executePopSegue(segue);
        }
      }
      else
      {
        activities.top()->onUpdate(elapsed);
      }
    }

    /**
     @brief Draws the current activity or segue and displays the result onto the window
    */
    void draw()
    {
      if (activities.size() == 0)
        return;

      // Prepare buffer for this pass
      renderer->clear(sf::Color::Transparent);

      // Update viewport
      renderer->setView(activities.top()->view);

      // Draw to gpu texture
      activities.top()->onDraw(*renderer);

      // Perform our draw operations to the target texture in the renderer
      renderer->draw();

      // Prepare to be displayed
      renderer->display();

      // Create a rectangle with a copy of the texture
      sf::Texture temp = renderer->getTexture();
      sf::Sprite post(temp);

      // Fill the window with the bg color
      if (clearBeforeDraw)
        handle.clear(activities.top()->bgColor);

      // draw screen
      handle.draw(post);

      // Flush renderer-owned memory
      renderer->flushMemory();
    }

    /**
     @brief Pops everything off the stack with respect to the activity life-cycle (onEnd(), remove, then delete for each entry)
     This ensures any activity has proper cleanup in the order the consuming software should expect.
    */
    void clearStackSafely()
    {
      stackAction = StackAction::clear;

      if (isUpdating)
        return;

      executeClearStackSafely();
    }

  private:
    /**
      @brief Applies an activity's view onto the renderer. This is used internally for segues.

      This function is kept here to make the library files header-only and avoid linkage.
    */
    void setActivityView(IRenderer &renderer, swoosh::Activity *activity)
    {
      renderer.setView(activity->getView());
    }

    /**
     @brief Resets the render surface's view to the window's default view. This is used internally for segues.

     This function is kept here to make the library files header-only and avoid linkage.
   */
    void resetView(IRenderer &renderer)
    {
      renderer.setView(handle.getDefaultView());
    }

    /**
     @brief This function properly terminates an active segue and pushes the next activity onto the stack
   */
    void executePopSegue(swoosh::Segue *segue)
    {
      pending_raii _(hasPendingChanges);

      segue->onEnd();

      activities.pop();

      swoosh::Activity *next = segue->next;

      const bool popLike = segueAction == SegueAction::pop
        || segueAction == SegueAction::replace
        || segueAction == SegueAction::rewind;

      if (popLike)
      {
        // We're removing an item from the stack
        swoosh::Activity *last = segue->last;
        last->onEnd();

        if (next->started)
        {
          next->onResume();
        }
        else
        {
          // We may have never started this activity because it existed
          // in the activity stack before being used...
          next->onStart();
          next->started = true;
        }

        if (segueAction == SegueAction::replace)
        {
          activities.pop(); // remove last
        }
        else if (segueAction == SegueAction::pop || segueAction == SegueAction::rewind) {
          // invokes yeild
          next->handleYieldable();
        }

        delete last;
      }
      else if (segueAction == SegueAction::push)
      {
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
    void executePop()
    {
      pending_raii _(hasPendingChanges);

      swoosh::Activity* activity = activities.top();

      activity->onEnd();
      activities.pop();

      if (activities.size() > 0) {
        // Handle our yeild
        activities.top()->handleYieldable();
        activities.top()->onResume();
      }

      delete activity;
    }

    void executeClearStackSafely()
    {
      pending_raii _(hasPendingChanges);

      while (activities.size() > 0)
      {
        if (segueAction != SegueAction::none)
        {
          swoosh::Segue *segue = static_cast<swoosh::Segue *>(activities.top());
          segue->onEnd();
          segue->last->onEnd();
          segue->next->onEnd();
          activities.pop(); // top
          activities.pop(); // last
          delete segue->last, segue->next, segue;
          segueAction = SegueAction::none;
          last = nullptr;
          continue;
        }

        Activity *current = activities.top();
        current->onEnd();
        activities.pop();
        delete current;
      }

      segueAction = SegueAction::none;
      stackAction = StackAction::none;
    }

    /**
      @brief Copy's the screen buffer and creates a temporary "blank" activity
      @return the newly generated blank activity

      Used internally
    */
    inline Activity *generateActivityFromWindow();
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
  class CopyWindow : public Activity
  {
    sf::Texture framebuffer;
    sf::Sprite drawable;
    bool captured;

    void copyWindowContents()
    {
      if (captured)
        return;

      // get the window handle
      auto &window = getController().getWindow();

      // get all original view and viewport settings
      auto &view = window.getView();
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
    CopyWindow(ActivityController &ac) : Activity(&ac)
    {
      captured = false;
    }

    virtual ~CopyWindow() { ; }

    void onStart() override { copyWindowContents(); };
    void onLeave() override { copyWindowContents(); };
    void onExit() override{};
    void onEnter() override { copyWindowContents(); };
    void onResume() override{};
    void onEnd() override{};

    void onUpdate(double elapsed) override{};

    void onDraw(IRenderer &renderer) override
    {
      renderer.submit(&drawable);
    }
  }; // CopyWindow

  // late implementation
  Activity *ActivityController::generateActivityFromWindow()
  {
    return new CopyWindow(*this);
  }

  // useful types in their own namespace
  namespace types
  {
    enum class direction : int
    {
      left,
      right,
      up,
      down,
      max
    };

    template <int val = 0>
    struct seconds
    {
      static sf::Time value() { return sf::seconds(val); }
    };

    template <sf::Int32 val = 0>
    struct milliseconds
    {
      static sf::Time value() { return sf::milliseconds(val); }
    };

    template <sf::Int64 val = 0>
    struct microseconds
    {
      static sf::Time value() { return sf::microseconds(val); }
    };

    /*
    shorthand notations*/
    template <typename T, typename DurationType = seconds<1>>
    using segue = ActivityController::segue<T, DurationType>;

    template <int val = 0>
    using sec = seconds<val>;

    template <sf::Int32 val = 0>
    using milli = milliseconds<val>;

    template <sf::Int64 val = 0>
    using micro = microseconds<val>;
  }
}