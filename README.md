![logo](https://i.imgur.com/tri24Y5.png)
# Swoosh v1.2.4
Header-only SFML Activity and Segue Mini Library

Tested across MSVC, GNU C++, and Clang compilers on Windows, Linux, OSX, and Android operating systems.

[See what else comes with Swoosh](https://github.com/TheMaverickProgrammer/Swoosh/wiki/Namespaces)

> 🚨 Critical changes from v1.2.3+
> 1. queuePop() and queueRewind() are now just pop() and rewind()
> 2. optimizeForPerformance(true/false) is changed to optimizeForPerformance(const quality& mode)
> 3. quality can be { realtime, reduced, mobile } where each is best-to-worst quality but worst-to-best performance depending on your hardware
> 4. Segues can query the controller's quality set with getRequestedQuality()
> 5. Added much-needed doxygen style documentation throughout the entire project
> 6. New Dream segue effect
> 7. Fixed some bugs with view toggling between activities
> See older changes at the [changelog](https://github.com/TheMaverickProgrammer/Swoosh/wiki/Changelog)

# ✨ Get Jump Started
See all the effects and more that come with the library on the [wiki](https://github.com/TheMaverickProgrammer/Swoosh/wiki).

See the [demo project](https://github.com/TheMaverickProgrammer/Swoosh/tree/master/ExampleDemo) for examples on how to use Swoosh. You can also copy the segues in the source folder and use them immediately in your games with no extra configuration.

# Instant Updates
![Twitter](https://proxy.duckduckgo.com/ip3/twitter.com.ico) Follow [@swooshlib](https://twitter.com/swooshlib) on Twitter to get instant updates!

# Technology
SFML 2.5, C++14, GLSL 1.10

# Video
Click the gif for the full video!

[![SlideIn Segue](https://media.giphy.com/media/2jsQgGNqmHU3HB3tZN/giphy.gif)](https://streamable.com/qb023)

See the pokemon demo using just Swoosh!

[![clip](https://media.giphy.com/media/1WbJank711TIIMmVr4/giphy.gif)](https://streamable.com/vyfhq)

---

# § Integrating Swoosh into your SFML application in 2 steps
✔️ Copy the headers found in the root at `src/Swoosh`. Optionally you can include the segues at `src/Segues`.

✔️ See [this example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/ExampleDemo/Demo.cpp) for how you should structure your main loop with the Activty Controller.

### ⚙️ Inheriting the AC (Activity Controller)
You can inherit the activity controller to extend and supply more complex data to your applications. For instance, you could extend the AC to know about your TextureResource class or AudioResource class so that each Activity instance has a way to load your game's media.

### 📱 Optimizing for Mobile
Mobile hardware cannot capture the screen, draw to it, and write back onto the frame buffer as quickly as we can on PC. There are some ways to do this faster but not with SFML at this time. In order to solve this, the AC has a new function pair `isOptimizedForPerformance()` and `optimizeForPerformance(const quality& mode)` that will allow you to query if you should go easy on the target device's GPU. These 2 functions by themselves mostly do nothing but with `getRequestedQuality()` can be used in both your custom Activities and your custom Segue effects to use different behavior for each possible quality mode.

As of **v1.2.4** all segue effects shipped with this library have quality mode support and should be very performant on low-end mobile hardware

# § API and Memory Management 
When creating polished applications, it should not be a concern to the user how to handle the memory for a scene or video game level. If you think about it, these scenes are just shells around the incoming or outgoing data in visual form. In mobile apps, activities are just container for the important stuff that shows up on the target device's screen. 

With this in mind, the biggest goal when designing this software was allowing users to write complex transitions as simple as possible and have the syntax to perform said action be human-readable while also discouraging memory allocation and management.

### 📝 Syntax
Swoosh addresses these issues by wrapping push and pop calls with templated types that expect either a class derived from `Activity` for screens or `Segue` for transition effects.

For example

```c++
ActivityController controller;
controller.push<MainMenuScene>();

...

// User selects settings
using types::segue;
controller.push<segue<BlendFadeIn>::to<AppSettingsScene>>();
```

The syntax is human-readable and flows naturally. Swoosh hides the intricacies from the user so they can focus on what's really important: Writing the application!

### ⏰ Changing Time
The `Segue` class takes in two arguments: The next activity type, and the duration for the transition to last. By default the transition is set to 1 second. 
This may be too fast or too slow for your needs. The `DurationType` class takes a templated wrapper for SFML time functions. They are found in the `swoosh::types` namespace.

For example

```c++
using namespace swoosh::types;
controller.push<segue<Cube3D<direction::left>, seconds<5>>::to<DramaticIntroScene>>();
```

### 🔍 Writing Clearer Code
The last example had a segue that required directional input and the line of code was longer than we'd like. 
Although Swoosh is doing a ton behind the scenes for us, we lost clarity.

We can clean up the code by creating our own typename aliases. Later, modifying your screen transition effect is as easy as changing one line.

```c++
using effect = segue<Cube3D<direction::up>, sec<2>>;
getController().push<effect::to<DramaticIntroScene>>();
```

Much more elegant!

### 🏭 Supplying Additional Arguments
Your activity classes may be dependant on external information like loading your game from a save file or displaying important business data exported from another screen. 

```c++
SaveInfo info = LoadSaveFile(selectedProfile);

// Pass on specific data the level wants from the save file
controller.push<SuperJumpManLevel1>({info.getLives(), info.getCoins(), info.getMapData()});
```

This is the same for segues

```c++
ActivityController& controller = getController();
LobbyInfo data = queryLobbyServer().get(); // blocking future request

using effect  = segue<CheckerboardEffect, sec<3>>;

// Go!
controller.push<effect::to<MatchMakingLobby>>(data);
```

# § Actions & Leaving Activities
The `ActivityController` class can _push_ and _pop_ states but only when it's safe to do so. It does not pop in the middle of a cycle and does not push when in the middle of a segue.
Make sure your activity controller calls are in an Activity's `onUpdate(double elapsed)` function to avoid having _push_ or _pop_ intents discarded.

### Push
```c++
controller.push<MyScene>();
controller.push<segue<FadeIn>::to<MyScene>>();
```

### Pop
Pushed activities are added to the stack immediately. However there are steps involved in the controller's update loop that do not make this safe to do for _pop_. Instead, the function `pop()` may return `false`, signalling the controller cannot pop. If `true`, the controller will wait until it is safe to unload your scene.

```
controller.pop(); 
controller.pop<segue<BlurFadeIn>>();
```

### Rewinding 
Rewinding is useful when you have an inactive Activity lingering in your stack from before and you wish to go back to that point. Rewinding the activity stack pops and ends all previous activities until it finds the first matching activity type. _pop_ is an intent to pop once where _rewind_ is an intent to pop _many times_.

Example: jumping from the menu to the battle screen to a hiscore screen back to the menu. 

This is useful to simulate persistent behavior such as in a top-down adventure game where you star as a young elf crossing the overworld, going deep into dungeons, and then teleporting yourself back to the overworld; exactly the way it was before you left.

The syntax is close to _push_ except if it succeeds, activities are ended and discarded.

```c++
using effect = segue<BlackWashFadeIn>;
using action = effect::to<LOZOverworld>;

bool found = controller.rewind<action>();

if(!found) {
    // Perhaps we're already in overworld. Certain teleport items cannot be used!
}
```

### Replacing
Sometimes you need to directly modify the current item on the stack. Some games let you restart levels. Others have dozens in a row and tracking each one would eat up too much memory!

Now you can replace the current activity safely like so:

```c++
if(restartLevel == true) {
    controller.replace<MyLevel>();
}
```

This works like any other action and so it will work with segues too!

# § Writing Activities
An activity has 8 unique lifecycle events that can be overriden:
- onStart , called once when this activity begins for the first time
- onExit  , called once before this activity is deleted
- onEnter , called when this activity is entering the view during a segue
- onResume, called when this activity has finished entering the view after a segue
- onLeave , called when this activity is leaving the view during a segue
- onEnd   , called when the activity has finished leaving a view after a segue
- onUpdate, called every tick while still in view
- onDraw  , called every tick while still in view

[Here is an example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/ExampleDemo/Scenes/AboutScene.h) of a simple About scene in your app. It shows text and has a button to click next for more info. The SFML logo rolls across the top just for visual effect.

### Defining a View
If you need to define a view for one activity without affecting another you use that Activity's `setView(sf::View view)` function. You can set once and forget! The controller will make sure everything looks right.

# § Writing Segues
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

### Useful Properties
In order to make use of the remaining time in your segue, two member functions are supplied

* `getElapsed()` returns sf::Time 
* `getDuration()` returns sf::Time

Sometimes you may need to step over the render surface and draw directly to the window

* `getController()` returns the ActivityController that owns it
* `getController().getWindow()` returns sf::RenderWindow
* `getController().getVirtualWindowSize()` returns sf::Vector2u of the window when your app is created. 

_getVirtualWindowSize()_ is useful when wanting to keep your graphics consistent with scale. By default it is the same value as your window when it is first created. 

### Drawing To The Screen
Segues are made up of two Activities: the last and the next. For most segues you need to draw one and then the other with some applied effect.

* `drawNextActivity(sf::RenderTexture& surface);` 
* `drawLastActivity(sf::RenderTexture& surface);`

Both draw their respective activity's contents to a sf::RenderTexture that can be used later. Read on below for an example.

[This example](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/src/Segues/PushIn.h) Segue will slide a new screen in while pushing the last scene out. Really cool!

### Embedding GLSL and textures
Some post processing effects require samples as inputs. In order to make Swoosh 100% header-only the scripts and samples had to be embedded. This is purely optional for your projects and if you want to share your custom segue effects, is the best practice.

Learn [how to embed GLSL and textures here](https://github.com/TheMaverickProgrammer/Swoosh/wiki/Embed-GLSL).

### Segue's & Activity States
It's important to note that Segues are responsible for triggering 6 of the 8 states in your activities.

* onStart -> the next scene starts once when the previous scene ends
* onLeave -> the previous scene has lost focus
* onExit  -> the previous scene when the next scene starts
* onEnd   -> the previous scene when the next scene starts after _pop_ intent (discards previous scene when finished)
* onEnter -> the next scene has gained focus
* onResume -> if the next scene had started before, it will resume after the previous scene ends from a _pop_ intent (discards previous scene when finished)

It might help to remember during a segue, both scenes are replacing eachother in the same frame. When a segue begins, the current scene is leaving and the other is entering. When the segue ends, the current scene exits and the other begins. 

# § Special Topic: Mobile Optimization
The default quality mode is `realtime` which will influence segue effects to capture your screen's contents into a render texture (aka buffer) to apply shader effects onto. This looks the best and is impressive but SFML has a GPU<->CPU bottleneck and will choke on mobile devices (or low-end computer GPUs) when doing this.

In order to provide an alternative, you can set the quality mode to `mobile` like so:

```cpp
app.optimizeForPerformance(quality::mobile);
```

You can gauge the outcome of each quality in the following way:
- `realtime` Default behavior. Best looks. May hog mobile or low-end gpu hardware
- `reduced` Programmer may reduce the looks and will have better performance than `realtime`
- `mobile` NO activities are updated in transitions and the looks will be drastically reduced but should have best performance.

Importantly, this specific mode will tell the activity controller to **not** update the next and last scenes in a segue effect. They will resume updating when the segue is over. This helps speed your mobile device up. 

Segues by themselves only draw what the programmer wants to have in the transition effect. The supplied segues have support for all 3 quality levels.

See the guassian blur effect [here](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/src/Segues/BlurFadeIn.h#L96).
Typically having a blur with the kernel size of 40 kills my mobile device's performance. But I've toggled the kernel size [here](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/src/Segues/BlurFadeIn.h#L96). Additionally, I have opted to capture the next and last activity screens **only once** when the segue firsts begins [here](https://github.com/TheMaverickProgrammer/Swoosh/blob/master/src/Segues/BlurFadeIn.h#L96).

By providing alternative segue effect behavior for the quality modes, you can ensure your segues will run on anyone's devices.

# § Special Topic: Copying the Window
If you have a particular structure how your game should end (like a GameOverScreen), it would make sense to have that screen be at the bottom of the stack at ALL times. We can start the player in the main menu and let them make other choices to config their controllers. If the player presses start, we can pop the main menu off the stack and begin the game. With this structure in mind, we might have something like the following:

```cpp
ActivityController ac(window);
ac.push<GameOverScreen>();
ac.push<GameWorld>();
ac.push<MainMenuScreen>();
```

But what if you need to load the game first? You might have a lot of other modules to load and threads to call that can't be safely put into an activity in a clean way. You might want to draw directly onto the window for a while to show a cool loading screen. 

Swoosh comes with a special class called `CopyWindow` that's shipped with file `ActivityController.h`. It will copy your screen's contents right before it is displayed and uses that as a "blank" Activity. 

Your code would need to include this when the loading is complete:

```cpp
// ... All the code we had before

// ... load stuff

// Load is complete!
if(gameIsLoaded == true) {
  ac.push<CopyWindow>();
  ac.pop<segue<FadeOut>>(); // Go to the MainMenuScreen
}
```

You can also inherit from this as a base class and have your screen's contents be captured in the next one.
