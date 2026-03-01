# Custom 2D Platformer Engine (Mega Man Clone)

A custom-built 2D game engine written from scratch in C++ using SFML. This project started as a technical exercise in building a robust Entity-Component System (ECS) and evolved into a physics-accurate platformer framework inspired by classic games like *Mega Man*.

## 🛠️ Tech Stack
* **Language:** C++
* **Graphics & Windowing:** SFML (Simple and Fast Multimedia Library)
* **Architecture:** Custom Entity-Component System (ECS)

## ✨ Core Features
* **Entity-Component System (ECS):** A fully custom ECS architecture separating data (Transforms, Bounding Boxes, States, Animations) from logic (Movement, Collision, Rendering).
* **Robust Physics & Movement:** Delta-time (`dt`) based movement ensuring consistent speeds regardless of frame rate, complete with gravity and jumping mechanics.
* **State-Driven Animation:** A dynamic animation system that switches seamlessly between states (Spawn, Idle, Run, Jump, Shoot) and automatically cleans up temporary entities (like explosions).
* **Hitbox Debugging:** A built-in debug rendering system that draws physical bounding boxes (red for player, green for tiles) entirely decoupled from visual sprite sizes to ensure pixel-perfect padding.
* **Combat System:** Projectile spawning, bullet-to-wall collision detection, and "Spawn & Swap" destructible environments.

## 🚧 Technical Challenges & Solutions

Building a 2D physics engine from scratch comes with infamous mathematical hurdles. Here is how I solved the biggest challenges in this project:

### 1. The "Time Spike" (Collision Tunneling)
* **The Problem:** When the engine first boots up, asset loading takes time. The SFML clock would register a massive delta-time (`dt`) on frame 1, applying seconds worth of gravity instantly. The player would spawn and teleport completely through the solid floor.
* **The Solution:** Implemented a strict clock reset (`m_clock.restart()`) precisely after asset loading and right before the game loop begins, ensuring the first frame evaluates a true `0.0` dt.

### 2. The "Corner Trap" & "Seam Catching"
* **The Problem:** When the player jumped diagonally into a wall, or slid down a wall comprised of multiple stacked blocks, the bounding box would snag on the invisible seams between tiles. The engine would incorrectly read the collision as landing on a floor.
* **The Solution:** Completely rewrote the AABB collision into **Axis-Separated Collision**. By resolving X-axis movement and overlaps *first*, pushing the player flush against the wall, the Y-axis calculation could accurately run without horizontal interference, resulting in buttery-smooth wall sliding.

### 3. The "Gravity Fight" (Stuck Grounded State)
* **The Problem:** The player's animation would rapidly flicker between "Falling" and "Idle" while standing on the floor because gravity constantly pushed velocity down, only for the collision system to immediately zero it back out.
* **The Solution:** Decoupled the grounded state from velocity checks. Created a strict `isGrounded` boolean flag inside the State component that is *only* set to true when the Y-axis collision mathematically pushes the player UP out of a floor block.

## 🚀 How to Build and Run
1. Clone this repository.
2. Link the project with SFML include and Lib
3. Run the Project
2. Ensure you have **SFML** installed and linked in your C++ IDE (e.g., Visual Studio).
3. Link the required SFML modules: `sfml-graphics`, `sfml-window`, `sfml-system`, `sfml-audio` (use the `-d` suffix if compiling in Debug mode).
4. Build and Run!
