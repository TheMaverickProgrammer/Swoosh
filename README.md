![logo](https://i.imgur.com/tri24Y5.png)
# Swoosh v1.1
Header-only SFML Activity and Segue Mini Library

Tested across MSVC, GNU C++, and Clang compilers on Windows, Linux, OSX operating systems.

## Get Jump Started
See all the effects and more that comes with the library on the [wiki](https://github.com/TheMaverickProgrammer/Swoosh/wiki).

See the [demo project](https://github.com/TheMaverickProgrammer/Swoosh/tree/master/ExampleDemo/Swoosh) for examples on how to use. You can also copy the segues in the source folder and use them immediately into your games with no extra configuration.

# Updates
![Twitter](https://proxy.duckduckgo.com/ip3/twitter.com.ico) Follow [@swooshlib](https://twitter.com/swooshlib) on Twitter to get instant updates!

# Technology
SFML 2.5, C++14, GLSL 1.10

## Optional
Includes visual studio project but not needed. Source code will work on other operating systems as long as it has C++17 support. You will need to provide your own build scripts to run the project. Swoosh header files require _zero_ building.

# Video
Click the gif for the full video!

[![SlideIn Segue](https://media.giphy.com/media/2jsQgGNqmHU3HB3tZN/giphy.gif)](https://streamable.com/qb023)

See the pokemon demo using just swoosh!

[![clip](https://media.giphy.com/media/1WbJank711TIIMmVr4/giphy.gif)](https://streamable.com/vyfhq)


# Integrating Swoosh into your SFML application
Copy the headers found in the root at `src/Swoosh`. Optionally you can include the segues at `src/Segues`.

Adding the mini library into your SFML application is very simple. See [this example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/ExampleDemo/Swoosh/Demo.cpp)

# Philosophy 
When creating polished applications it should not be a concern to the user how to handle the memory for a scene or video game level. 
These activities are just shells around the incoming or outgoing data in visual form; a container for the important stuff that shows up 
on the target device's screen. The biggest goal when designing this software was allowing user's to write complex transitions as simple as possible 
and have the syntax to perform said action be human readable.

# Syntax
Swoosh addresses these issues by wrapping push and pop calls with templated types that expect either a class derived from `Activity` for screens or `Segue` for transition effects.

For example

```c++
ActivityController controller;
controller.push<MainMenuScene>();

...

// User selects settings
using intent::segue;
controller.push<segue<BlendFadeIn>::to<AppSettingsScene>>();
```

The syntax is human readable and flows naturally. Swoosh hides the intricacies from the user so they can focus on what's really important: writing the application!

## Changing Time
The `Segue` class takes in two arguments: the next activity type, and the duration for the transition to last. By default the transition is set to 1 second. 
This may be too fast or too slow for your needs. The `DurationType` class takes a templated wrapper for SFML time functions. They are found in the `swoosh::intent` namespace.

For example

```c++
using namespace swoosh::intent;
controller.push<segue<Cube3D<direction::left>, seconds<5>>::to<DramaticIntroScene>>();
```

## Writing Clearer Intents
The last example had a segue that required directional input and the syntax was longer than we'd like. 
Although Swoosh is doing a ton behind the scenes for us, we lost some clarity.

We can clean up the intent by creating our own segue typename alias. 

```c++
using segue  = segue<Cube3D<direction::up>, sec<2>>;
using intent = segue::to<DramaticIntroScene>;

getController().push<intent>();
```

Much more elegant!

## Less Typing!
There are 3 wrappers and each have a shorthand alias

`.......seconds<int val>` -> `sec<int val>`

`milliseconds<Int32 val>` -> `milli<Int32 val>`

`microseconds<Int64 val>` -> `micro<Int64 val>`

## Supplying Additional Arguments
Your activity classes may be dependant on external information like loading your game from a save file or displaying important business data exported from another screen. 

```c++
SaveInfo info = LoadSaveFile(selectedProfile);

// Pass on specific data the level wants from the save file
controller.push<SuperJumpManLevel1>({info.getLives(), info.getCoins(), info.getMapData()});
```

This is the same for segues

```c++
ActivityController& controller = getController();

// write clear intents
using segue  = segue<CheckerboardEffect, sec<3>>;
using intent = segue::to<MatchMakingLobby>;

LobbyInfo data = queryLobbyServer().get(); // blocking future request

// Go!
controller.push<intent>(data);
```

# Leaving Activities
The `ActivityController` class can _push_ and _pop_ states but only when it's safe to do so. It does not pop in the middle of a cycle and does not push when in the middle of a segue.
Make sure your activity controller calls are in an Activity's `onUpdate(double elapsed)` function to avoid having _push_ or _pop_ intents discarded.

## Push
```c++
controller.push<MyScene>();
controller.push<segue<FadeIn>::to<MyScene>>();
```

## Pop
Pushed activities are added to the stack immediately. However there are steps involved in the controller's update loop that do not make this
safe to do for _pop_. Instead, the function `queuePop()` is supplied, signalling the controller to pop as soon as it can.

```
controller.queuePop(); 
controller.queuePop<segue<BlurFadeIn>>();
```

## Rewinding
Rewinding is useful when you have an inactive Activity lingering in your stack from before and you wish to go back to that point. Rewinding the activity stack pops and ends all previous activities until it finds the first matching activity type. _queuePop_ is an intent to pop once where _queueRewind_ is an intent to pop _many times_.

Example: jumping from the menu to the battle screen to a hiscore screen back to the menu. 

This is useful to simulate persistent behavior such as in a top-down adventure game where you star as a young elf crossing the overworld, going deep into dungeons, and then teleporting yourself back to the overworld; exactly the way it was before you left.

The syntax is close to _push_ except if it succeeds, activities are ended and discarded.

```c++
using segue = segue<BlackWashFadeIn>;
using intent = segue::to<LOZOverworld>;

bool found = controller.queueRewind<intent>();

if(!found) {
    // Perhaps we're already in overworld. Certain teleport items cannot be used!
}
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

[Here is an example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/ExampleDemo/Swoosh/DemoActivities/aboutscene.h) of a simple About scene in your app. It shows text and has a button to click next for more info. The SFML logo rolls across the top just for visual effect.

## Defining a View
If you need to define a view for one activity without affecting another you use that Activity's `setView(sf::View view)` function. You can set once and forget! The controller will make sure everything looks right.

# Writing Segues
When writing transitions or action-dependant software, one of the worst things that can happen is to have a buggy action. 
If one action depends on another to finish, but never does, the app will hang in limbo. 

This fact inspired Swoosh to be dependant on a timer. When the timer is up the Segue will be deleted and the next scene 
added on top of the stack. The time elapsed and total time alloted can be retrieved in the class body to make some cool effects
from start to finish.

The class for Segues depends only on one overloaded function `void OnDraw(sf::RenderTexture& surface)`.
The constructor must take in the duration, the last activity, and the next activity.

```c++
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
* `getController().getVirtualWindowSize()` returns sf::Vector2u of the window when your app is created. 

_getVirtualWindowSize()_ is useful when wanting to keep your graphics consistent with scale. By default it is the same value as your window when it is first created. 

## Drawing To The Screen
Segues are made up of two Activities: the last and the next. For most segues you need to draw one and then the other with some applied effect.

* `drawNextActivity(sf::RenderTexture& surface);` 
* `drawLastActivity(sf::RenderTexture& surface);`

Both draw their respective activity's contents to a sf::RenderTexture that can be used later. Read on below for an example.

[This example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/src/Segues/PushIn.h) Segue will slide a new screen in while pushing the last scene out. Really cool!

## Embedding GLSL and textures
Some post processing effects require samples as inputs. In order to make Swoosh 100% header-only the scripts and samples had to be embedded. This is purely optional for your projects and if you want to share your custom segue effects, is the best practice.

Learn [how to embed GLSL and textures here](https://github.com/TheMaverickProgrammer/Swoosh/wiki/Embed-GLSL).

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
