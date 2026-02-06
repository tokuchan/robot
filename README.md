# Robot

## Quick Start

### Prerequisites

Ensure you have the following installed:
- git
- make
- podman

### Getting Started

1. Clone the repository
2. Run `make` for available commands and instructions
3. Run `make ui` to build and run the project

## Description

_(To be completed)_

## Design

### Overview

This is a system that allow someone to remotely "teleoperate" a robot. You are able to use either a keyboard or a joystick (gamepad) to control a virtual "robot" and avoid both static and dynamic obstacles.

### Goals and Non-Goals

## Goals

* Showcase a C++ server backend.
* Define a simple 2D world with static and dynamic obstacles, composed of simple convex polygons or sets thereof.
* Provide a simple user-controlled robot that can move about the scene. This robot accepts commands to set the velocity vector (steering angle and speed).
* Provide a simple web client frontend that connects to the backend to offer a UI.
* This front end simply draws the world and offers a keyboard or gamepad control interface.

## Non-goals

* This is not a videogame, so there's no need for scorekeeping, multi-player, fancy graphics, sound, or most of the things that games expect to provide.
* There is no need to provide any kind of authentication or security model, as this program is never meant to be run on the public Internet.

### Design Alternatives

I considered several possible alternatives to design. There are several "layers" to consider for design choices:

#### Language Alternatives

* There are two main alternatives for my choice of backend programming language at this time: C++ and Python.
  * Python is:
    * easier to prototype in, and the standard library is vast.
    * offers tools like UV make working with 3rd-party librarys very easy.
    * is not very performant, especially when it comes to massively-concurrent or parallel problems.
  * C++ is:
    * extremely performant, provided one is careful.
    * has an extremely mature toolchain, with excellent support for language features as far as C++20, and decent support for C++23 at this time.
    * has Boost, which is almost as nice as the Python standard library.
    * is harder to work with, and more verbose.
    * requires more up-front effort to manage tooling and the build environment.
* There are two main alternatives for my choice of UI programming language: Javascript and Typescript.
  * Javascript is:
    * automatically supported by every web browser in common usage.
    * offers sufficient support for event-driven programming and the Canvas object for 2D rendering.
    * obnoxious to program in, with a lot of annoying sharp edges.
    * essentially requires constant effort to remain current with libraries.
  * Typescript is:
    * statically type-checked and therefore likely safer to work in.
    * well-supported in the community.
    * able to compile to javascript, so it can deploy to any current web browser.
    * hampered by the requirement to "compile" code to JS as a required build step.

#### UI Design Alternatives

* I only need a very basic UI for this to work. 
  * I could design a UI using something like react.js or vue (I think!).
    * This may be simpler to do, as I can work within the framework to design elements rather than scratch-coding an event loop by myself.
    * It would be heavier-weight though, as I'd have to require the UI framework.
    * It would also require me to work within the framework itself, even if there might be a simpler way to do this from scratch.
  * I could also scratch-code a UI using Canvas and the built-in events system.
    * This option wouldn't require any extra libraries.
    * I'd be free to hand-code just the parts I need (Canvas and keyboard/gamepad event handlers).
    * I would have to implement everything myself however, and I'm _not_ a JS or TS programmer. I'd have to rely on AI and web searches to implement this.
  
#### System Design Alternatives

* I could offer a _single_ program that serves both the UI _and_ operates the backend.
  * This could be a bit simpler, since then there'd just be a _single_ program to write and debug.
  * The build environment would be simpler, since the server could even be statically-linked to avoid dependencies.
  * I'd have to implement a web server however, as the UI would need to be served.
  * I wouldn't be able to implement a compliant web server, especially one with TLS support in a reasonable amount of time.
  * There exist excellent and lightweight options, such as NGinx that I would not be able to avail myself of.
* I could use a server like Nginx to serve the UI components, and a _seperate_ C++ backend.
  * This would allow me to avoid having to deal with a web server, since I can just use NGinx for the UI.
  * The backend then is _just_ a simple REST server.
  * The whole thing can be containerized, so the end user _still_ only sees a _single_ program.

#### Backend Design Alternatives

* I could roll my own REST implementation or use Boost::Beast.
  * If I roll my own implementation:
    * I have total control over how it communicates to the user.
    * I have to implement at least enough of HTTP to support the client.
    * I'd have to implement the low-level server code, which is rather painful.
  * If I use Beast:
    * I have a dependency on Boost.
    * I have to work within the requirements of the framework.
    * I _don't_ have to think about the networking stuff.
    * I _don't_ have to worry about the HTTP/HTTPS or REST stuff, as Beast takes care of that for me.
    * I've worked with Beast before, so the cognitive burden is lower for me.
* I can use OOP to design the game elements.
  * This is simpler, and the "classical" approach to program design.
  * Each entity gets it's own type, and I can create a "family tree" of types to relate back to a `GameObject` type.
  * It is a lot less flexible, because multiple inheritance suffers from the "diamond" problem and the C++ rules are subtle and complex here.
  * This approach tends to encourage "God" objects, as many data and methods need to be shared across types, thus forcing one to move them "up" in the class hierarchy.
  * This approach works fine at small scales, but will start to struggle when I need to consider 100,000 game entities. Mainly, this is due to the sparse usage of memory.
  * Packing and data-oriented design are harder to do here, as objects expect to have all data to hand, either directly or by pointer/reference.
* I can use ECS (Entity Component System pattern) to design the game elements.
  * This is a well-known pattern in high-performance simulation and game design.
  * It scales extremely well, often allowing games to support tens of thousands of on-screen entities at once even with modest hardware.
  * It requires the implementation of more complex types, however, such as: `SparseSet`, `Component`, and `ComponentType` or `ComponentQuery` which abstract some of the salient features of ECS.
  * One point: I _don't_ have to implement the "full monty", as once entities are created, they will _not_ be destroyed or reassigned. This simulation does not need "bullets" or destruction.
#### Build Envronment Alternatives

There is a whole spectrum for designing the build environment. At one end of the scale, I can just offer a simple `build.sh` script that just compiles the code. At the other end, I can offer a fully-containerized build system. The main tradeoff is complexity versus convenience. My general policy is to seriously consider developer ergonomics. I truly believe that the "onboarding" process should be "Clone the project, then run `make` to discover your options". This has the benefit of allowing the developer to focus on _developing_, but at the cost of maintaining a more complex build system.

Here is a summary of the main tradeoffs:

| Step                 | Makefile | CMake  | Container |
|----------------------|---------:|:------:|:---------:|
| Tool Installation    | 👎       | 👎     | 👍        |
| Tool Discovery       | 👎       | 👍     | 👍        |
| Library Installation | 👎       | 👎     | 👍        |
| Library Discovery    | 👎       | 👍     | 👍        |
| Program Build        | 👍       | 👍     | 👍        |
| Program Rebuild      | 👍       | 👍     | 👍        |
| Program Test         | 👍       | 👍     | 👍        |
| Build Artifact Clean | 👍       | 👍     | 👍        |
| Build Tool Clean     | 👎       | 👎     | 👍        |
| Packaging            | 👎       | 👍     | 👍        |
| Deployment           | 👎       | 👎     | 👍        |

(Note: The 👎 means a feature is not supported, or requires manual intervention. The 👍 emoji means that the feature is fully supported or automatic.)

I consider the automatic treatment of environment tooling to be very important for a positive developer experience.

### Proposed Architecture

In general, I believe that a containerized architecture is the best overall approach for the design of this system. First, it abstracts away details of running the system, and presents the same convenient interface a single program would. Second, I have rather extensive experience with containerized systems, so this isn't hard to do. Third, this let's me use NGinx to serve the UI, simplifying the backend program. Fourth, I can use a NIX flake to manage dependencies with the same determinism and convenience one expects from tools like UV or NPM. Fifth, I can make the build fully self-contained, so the onboarding instructions are: clone and run make, which is preferable.

### Component Design

#### Front End (UI)

#### Back End

Overall, the back end will be implemented as a Boost::Beast REST server. This allows me to devote most of my efforts to the ECS system that actually handles the simulation, rather than spend much time on the REST boilerplate. I will need endpoints to allow the player to fetch total state (to initialize) as well as some means to receive geometry updates for rendering, and an endpoint to submit robot commands to. I'll use a polling approach for now, as its simpler and should be robust enough.

The ECS (Entity Component System) pattern rests on the idea of organizing the data by "column" rather than "row". That is, instead of using containers of tuples or structs to represent the entities, I use a simple integer entity ID, and then each subsystem has its own containers to keep track of, which it does by using the entity ID. The advantage, as stated above, is that I can iterate over densly-packed containers, which is _far_ more cache friendly and performant. However, I shall have to design a `SparseSet` object to map entity IDs onto component indices and back efficiently.

This approach means that there is a mainloop, responsible for keeping the simulation up-to-date, which runs in its own thread. This is needed because if we wait for Beast events to update, we will have a harder time figuring out what entities did. For example, which path did this entity take in the time interval between the last Beast event and this one? Therefore, its better to keep Beast and the simulation in seperate threads. This _does_ mean that I have to account for thread safety, but since the only thing Beast _really_ has to do after initialization is to read in commands and output positions for entities, I can just use mutexes and locks to handle shared access to those components.

### Data Models

The ECS approach means that each entity is represented by an `EntityID`, which is just a `size_t` integer. Then, additional data, such as position or velocity are represented as components, and "join" to the entity on the `EntityID`. Finally, logic is defined by `System`s, which encapsulate the idea of mapping an update function over the contents of a set of component containers. For example, the `EntityTransform` `System` would use the `Position` and `Velocity` components to compute new `Positons` for each `EntityID` in its remit (as determined in this case by iterating over all `Position`s).

### API Design

_(To be completed)_

### Security Considerations

There are essentially _no_ considerations given to security at _this_ time, as this system is currently meant to be run _locally_ in a container on a single machine. A future iteration would, of course, _separate_ the UI and the back end, and at that point an authentication system would become necessary. Most likely, I'd set up an authentication service, perhaps using Oauth, that creates JWT (JSON Web Tokens) to represent access, which the UI would then use to talk to the simulator.

### Performance Considerations

Given that the entire system is "single-player", the main considerations for performance are going to focus on the "game" engine and the UI's ability to render all of the geometry quickly. For now, I'll keep it simple by focusing on the back end, and leave the UI to be slow until I either find someone to make it better, or learn enough to be able to do it myself.

### Testing Strategy

In general, my approach to system testing comprises three main components: regression testing, approval testing, and assertive programming. Assertive programming means using lots of assertions in the code, as preferred to using traditional unit test assertions, because assertions are able to be easily exposed to production data, which increases the liklihood of catching problems.

Approval testing, also known as testing against a "golden master" is where test drivers are run against predetermined input, and the received output is compared with an approved "golden master". If there is a difference, the test fails.

Finally, regression testing means capturing cases of known failures and encoding them as tests. Regression tests are generally the only tests I typically use a dedicated test framework for; though I will write up a few obvious "smoke" tests with edge cases as I concieve of them.

In this system, tests are captured using Catch2, v3, and ApprovalTests, and the test drivers are managed via ctest, and exposed under the `test` make target.

### Deployment

There is no concrete deployment plan per se, as the final product is the container itself. Of course, being a container means that future deployment options are myriad.

### Future Work

_(To be completed)_