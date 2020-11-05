#pragma once
#include <SFML/Graphics.hpp>

namespace swoosh {
  class ActivityController; /* forward decl */

  /**
  @class Activity
  @brief An activity is an isolated screen with content drawn onto it or a unique scene in a game

  Every scene in your application must inherit Activity to work with the Swoosh library.

  An activity has 8 unique lifecycle events that can be overriden:
    - onStart , called once when this activity begins for the first time
    - onExit  , called once before this activity is deleted
    - onEnter , called when this activity is entering the view during a segue
    - onResume, called when this activity has finished entering the view after a segue
    - onLeave , called when this activity is leaving the view during a segue
    - onEnd   , called when the activity has finished leaving a view after a segue
    - onUpdate, called every tick while still in view
    - onDraw  , called every tick while still in view *
    
    * some segues may optimize and skip draw calls (see: class WhiteWashFade)
  */
  class Activity {
    friend class ActivityController;

  private:
    bool started; //!< Flag denotes if an activity should call onStart() or onResume()

  protected:
    ActivityController* controller{ nullptr }; //!< Pointer to the activity controller
    sf::View view; //!< Custom view for this activity
    sf::Color bgColor; //!< Color to paint the background

  public:
    Activity() = delete;

    /**
      @brief constructs the activity
    */

    Activity(ActivityController* controller) : controller(controller) { started = false; }
    virtual void onStart() = 0;
    virtual void onLeave() = 0;
    virtual void onExit() = 0;
    virtual void onEnter() = 0;
    virtual void onResume() = 0;
    virtual void onEnd() = 0;
    virtual void onUpdate(double elapsed) = 0;
    virtual void onDraw(sf::RenderTexture& surface) = 0;
    virtual ~Activity() { ; }
    void setView(const sf::View& view) { this->view = view; }
    void setView(const sf::Vector2u& size) { this->view = sf::View(sf::FloatRect(0.0f, 0.0f, (float)size.x, (float)size.y)); }
    void setView(const sf::FloatRect& rect) { this->view = sf::View(rect); }
    void setBGColor(const sf::Color color) { this->bgColor = color;  }
    const sf::View getView() const { return this->view; }
    const sf::Color getBGColor() const { return this->bgColor; }
    ActivityController& getController() { return *controller; }
  };
}
