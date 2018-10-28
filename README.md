# Swoosh
Header-only SFML Activity and Segue Mini Library

# Technology
SFML 2.5

C++14

## Optional
Includes visual studio project but will work on other operating systems as long as it has C++14 support

# Video
[Click to watch!](https://streamable.com/qb023)

# Philosophy 
When creating polished applications it should not be a concern to the user how to handle the memory for a scene or video game level. 
These activities are just shells around the incoming or outgoing data in visual form; a container for the important stuff that shows up 
on the target device's screen. The biggest goal when designing this software was allowing user's to write complex transitions as simple as possible 
and have the syntax to perform said action be human readable.

# Syntax
Swoosh addresses these issues by wrapping push and pop calls with templated types that expect either a class derived from `Activity` for screens or `Segue` for transition effects.

For example

```
ActivityController controller;
controller.push<MainMenuScene>();

...

// User selects settings
using intent::segue;
controller.push<segue<SlideInLeft>::to<AppSettingsScene>>();
```

The syntax is human readable and flows naturally. Swoosh hides the intricacies from the user so they can focus on what's really important: writing the application!

## Changing Time
The `Segue` class takes in two arguments: the next activity type, and the duration for the transition to last. By default the transition is set to 1 second. 
This may be too fast or too slow for your needs. The `DurationType` class takes a templated wrapper for SFML time functions. They are found in the `swoosh::intent` namespace.

For example

```
using namespace swoosh::intent;
controller.push<zegue<FadeIn, seconds<5>>::to<DramaticIntroScene>>();
```

There are 3 wrappers and each have a shorthand alias

`seconds<int val>       ` -> `sec<int val>`

`milliseconds<Int32 val>` -> `milli<Int32 val>`

`microseconds<Int64 val>` -> `micro<Int64 val>`

## Supplying Additional Arguments
Your activity classes may be dependant on external information like loading your game from a save file or displaying important business data exported from another screen. 

```
SaveInfo = info = LoadSaveFile(selectedProfile);
controller.push<SuperJumpManLevel1>({info.getLives(), info.getCoins(), info.getMapData()});
```

This is the same for segues

```
FinancialInfo* data = loadFinancialResult(calender.getDate());
controller.push<segue<CheckerboardEffect, sec<3>>::to<FinancialReport>>(data);
```

# Leaving Activities
The `ActivityController` class can _push_ and _pop_ states but only when it's safe to do so. It does not pop in the middle of a cycle and does not push when in the middle of a segue.
Make sure your activity controller calls are in an Activity's `onUpdate(double elapsed)` function to avoid having _push_ or _pop_ intents discarded.

## Push
```
controller.push<MyScene>();
controller.push<segue<FadeIn>::to<MyScene>>();
```

## Pop
Pushed activities are added to the stack immediately. However there are steps involved in the controller's update loop that do not make this
safe to do for _pop_. Instead, the function `queuePop()` is supplied, signalling the controller to pop as soon as it can.

```
controller.queuePop(); 
controller.queuePop<segue<SlideIn>>();
```

# Writing Activities
An activity has 7 states it can be in:
* Starting for the first time
* Entering the focus of the app
* Leaving the focus of the app
* Inactive (but not terminating)
* Resuming 
* Ending (to be terminated)
* Updating 

Here is an example of the most simplest scene using Swoosh:

```
class DemoScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;

  sf::Font   menuFont;
  sf::Text   menuText;

public:
  DemoScene(ActivityController& controller) : Activity(controller) { 
    bgTexture = LoadTexture("resources/scene.png");
    bg = sf::Sprite(*bgTexture);

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red); 
  }

  virtual void onStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
  }

  virtual void onUpdate(double elapsed) {
  }

  virtual void onLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;

  }

  virtual void onExit() {
    std::cout << "DemoScene OnExit called" << std::endl;
  }

  virtual void onEnter() {
    std::cout << "DemoScene OnEnter called" << std::endl;
  }

  virtual void onResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void onDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("Hello World");
    surface.draw(menuText);
  }

  virtual void onEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};
```

# Writing Segues
When writing transitions or action-dependant software, one of the worst things that can happen is to have a buggy action. 
If one action depends on another to finish, but never does, the app will hang in limbo. 

This fact inspired Swoosh to be dependant on a timer. When the timer is up the Segue will be deleted and the next scene 
added on top of the stack. The time elapsed and total time alloted can be retrieved in the class body to make some cool effects
from start to finish.

The class for Segues depends only on one overloaded function `void OnDraw(sf::RenderTexture& surface)`.
The constructor must take in the duration, the last activity, and the next activity.

```
  SlideIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { 
    /* ... */ 
  }
```

## Useful Properties
In order to make use of the remaining time in your segue, two member functions are supplied

* `getElapsed()` returns sf::Time 
* `getDuration()` returns sf::Time

Sometimes you may need to step over the render surface and draw directly to the window

* `getController()` returns the ActivityController that owns it
* `getController().getWindow()` returns sf::RenderWindow

## Drawing To The Screen
Segues are made up of two Activities: the last and the next. For most segues you need to draw one and then the other with some applied effect.

* `drawNextActivity(sf::RenderTexture& surface);` 
* `drawLastActivity(sf::RenderTexture& surface);`

Both draw their respective activity's contents to a sf::RenderTexture that can be used later. Read on below for an example.

[This example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/Swoosh/SlideIn.h) Segue will slide a new screen in while pushing the last scene out. Really cool!

## Segue's & Activity States
It's important to note that Segue's are responsible for triggering 6 of the 7 states in your activities.

* onLeave -> the last scene has lost focus
* onExit  -> the last scene when the segue ends
* onEnd   -> the last scene when the segue ends after a _Pop_ intent
* onEnter -> the **next** scene when the segue begins
* onResume -> the **next** scene when the segue ends after a _Pop_ intent

_OR_

* onStart -> the **next** scene when the segue ends after a _Push_ intent

It might help to remember that when a segue begins, the current activity is leaving and the other is entering. When the segue ends, the current activity exits and the other begins.

# Integrating Swoosh into your SFML application
Adding Swoosh, the acitivity and segue mini library into a fresh SFML application is very simple. See [Example.cpp](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/Swoosh/Example.cpp)

# Namespaces
Last but not least, Swoosh as a stand-alone library comes with other utilities specific but not limited to gaming.

## using namespace swoosh;
The basics

* class Activity
* class Segue
* class ActivityController
* class Timer

## using namespace swoosh::intent;
Aliases tucked away to make the library easier to use

* typename segue<_Segue_, _Duration_>::to<_Destination_>
* typename seconds
* typename milliseconds
* typename microseconds

## using namespace swoosh::game;
Game logic and utility functions to shorten SFML code

* function _bool_ doesCollide(_a_, _b_)
* function _double_ angleTo(_subject_, _target_)
* function _vector2_ normalize(_input_)
* function _vector2_ direction(_target_, _dest_)
* function void setOrigin(_sprite_, _fx_, _fy_) 
* function void setOrigin(_text_, _fx_, _fy_) 

## using namespace swoosh::ease;
Math that makes cool animations over time with little effort

* function _double_ linear(_elapsed_, _duration_, _exponent_)
* function _double_ wideParabola(_elapsed_, _duration_, _exponent_)

