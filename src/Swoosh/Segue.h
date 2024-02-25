#pragma once
#include <Swoosh/Timer.h>
#include <Swoosh/Activity.h>
#include <SFML/Graphics.hpp>

namespace swoosh {
  class ActivityController;

  /**
   * @class quality
   * @brief Quality modes to toggle optimization in ActivityControllers and segue effects
   */
  enum class quality {
    realtime = 0, // Request real-time shader effects
    reduced,      // Request reduced, real-time shader effects
    mobile        // Request reduced and single-pass screen render. Also does not call onUpdate() on activities until complete.
  };

  /**
   * @class Segue
   * @brief A segue is a visual transition effect from one activity to another
   * 
   * Segues are activities that properly manage the life-cycle of the 2 activities: next and last
   * 
   * The only function that can be overriden is onDraw()
   * This allows programmers to create custom transition effects while only worrying about one thing: how it looks
   * This rigid design is intentional to influence plug-and-play segues. This way custom content can be shared and it "just works"
   */
  class Segue : public Activity {
    friend class ActivityController;

  private:
    Activity* last{ nullptr };
    Activity* next{ nullptr };
    sf::Time duration;
    Timer timer;

    // Hack to make this lib header-only
    void (ActivityController::*setActivityViewFunc)(IRenderer& renderer, Activity* activity);
    void (ActivityController::*resetViewFunc)(IRenderer& renderer);

  protected:
    const sf::Time getDuration() const { return duration; }
    const sf::Time getElapsed() { return timer.getElapsed(); }
    const sf::Color getLastActivityBGColor() const { return last->getBGColor(); }
    const sf::Color getNextActivityBGColor() const { return next->getBGColor(); }

    void drawLastActivity(IRenderer& renderer) {
      if (last) {
        (this->getController().*setActivityViewFunc)(renderer, last);
        renderer.clear(last->getBGColor());
        last->onDraw(renderer);
        renderer.draw();
        (this->getController().*resetViewFunc)(renderer);
      }
    }

    void drawNextActivity(IRenderer& renderer) {
      (this->getController().*setActivityViewFunc)(renderer, next);
      renderer.clear(next->getBGColor());
      next->onDraw(renderer);
      renderer.draw();
      (this->getController().*resetViewFunc)(renderer);
    }

  public:
    void onStart() override final { next->onEnter();  last->onLeave(); timer.start(); }

    void onUpdate (double elapsed) override final {
      timer.update(sf::seconds(static_cast<float>(elapsed)));

      last->onUpdate(elapsed);
      next->onUpdate(elapsed);
    }

    void onLeave() override final { timer.pause(); }
    void onExit() override final { ; }
    void onEnter() override final { ; }
    void onResume() override final { timer.reset(); timer.start(); }
    virtual void onDraw(IRenderer& renderer) = 0;
    void onEnd() override final { last->onExit(); }

    Segue() = delete;
    Segue(sf::Time duration, Activity* last, Activity* next) 
      : setActivityViewFunc(nullptr), resetViewFunc(nullptr), duration(duration), last(last), next(next), Activity(&next->getController()) { /* ... */ }
    virtual ~Segue() { }
  };
}
