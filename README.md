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

### Proposed Architecture

_(To be completed)_

### Component Design

_(To be completed)_

### Data Models

_(To be completed)_

### API Design

_(To be completed)_

### Security Considerations

_(To be completed)_

### Performance Considerations

_(To be completed)_

### Testing Strategy

_(To be completed)_

### Deployment

_(To be completed)_

### Future Work

_(To be completed)_