# 🚀 SpaceSim — Physically-Accurate Microgravity Space Simulator

A **real-time 3D space physics simulator** written in C++17 and OpenGL 3.3 Core Profile, built from scratch without third-party physics engines. The simulation implements true Newtonian mechanics in a zero-gravity environment — objects move only when force is applied, momentum is conserved indefinitely, and there is no artificial atmospheric drag.

The simulator features an authentic solar system environment with a procedural terrestrial planet, distant glowing sun, 6,000-star celestial sphere, modular space station, satellite, tumbling asteroid field, microgravity space dust, vector HUD with prograde/retrograde markers, target tracking, fly-by-wire flight assist, and full dual-hand keyboard controls.

---

## 📋 Table of Contents

- [Physics Design Goals](#-physics-design-goals)
- [Key Features](#-key-features)
- [Controls & Flight Deck](#-controls--flight-deck)
- [Navigation & Flight Systems](#-navigation--flight-systems)
- [Project Structure](#-project-structure)
- [Prerequisites](#-prerequisites)
- [Build Instructions](#-build-instructions)
- [Running the Simulator](#-running-the-simulator)
- [Architecture & Shaders](#-architecture--shaders)
- [Dependencies](#-dependencies)

---

## ⚛️ Physics Design Goals

SpaceSim is designed around real microgravity physics — not arcade game approximations:

| Principle | Implementation |
|---|---|
| **Zero Gravity** | Zero gravity vector — objects remain in free-floating microgravity |
| **Newton's 1st Law** | Velocity is preserved indefinitely (zero vacuum drag) |
| **Newton's 2nd Law** | $F = ma$ — all motion derived from applied RCS thruster forces |
| **Conservation of Momentum** | Linear and angular momentum conserved |
| **6DOF Motion** | Full 6 degrees of freedom: Translation ($X/Y/Z$) + Rotation (Pitch/Yaw/Roll) |
| **Quaternion Integration** | Unit quaternion ($\text{glm::quat}$) rotation — eliminates gimbal lock |
| **Semi-Implicit Euler** | Velocity integrated before position for numerical stability |
| **Flight Assist Modes** | Toggle between FA-ON (fly-by-wire stabilization) and FA-OFF (pure Newtonian drift) |

---

## 🌌 Key Features

### 1. Solar System Environment
* **Terra Planet**: Procedural terrestrial globe generated via multi-octave 3D fractal noise featuring continental biomes (coasts, savannas, mountains, snow peaks), polar ice caps, mirror-like specular ocean sun glint, dynamic swirling clouds with ground shadows, nighttime golden city lights, and Rayleigh atmospheric limb scattering.
* **The Sun**: Distant illuminating star casting directional sunlight and shadows across all bodies, featuring solar limb darkening, fiery corona falloff, and surface granulation noise.
* **Celestial Starfield**: 6,000 stars distributed uniformly on an $850\text{ m}$ celestial sphere using the Fibonacci spiral distribution. Rendered at optical infinity ($z = w$) with realistic astronomical spectral colors (O/B, A/F, G, K/M) and subtle scintillation.
* **Space Station Alpha**: Modular orbital outpost featuring habitat modules, airlocks, docking port rings, a nadir-facing cupola observation dome, $34\text{ m}$ integrated gold truss, 4 massive iridescent photovoltaic solar array wings, thermal radiators, and flashing red/green/white anti-collision navigation strobes.
* **ComSat-1 Satellite**: Golden Kapton thermal foil body, parabolic high-gain communications dish antenna with sub-reflector feed horn, and deployable solar arrays.
* **Asteroid Field**: 5 unique procedural asteroids with high-frequency 3D noise craters tumbling along independent Newtonian angular velocity axes.
* **Cosmic Space Dust**: 1,600 micrometeorite particles toroidally wrapped around the spacecraft providing immediate, visceral velocity feedback.

### 2. Tactical Vector Flight HUD
* **Boresight Reticle & Artificial Horizon**: Central flight cross, roll indicator, and pitch ladder.
* **Prograde Vector (`⨁`)**: Fluorescent green marker indicating your ship's actual physical travel vector.
* **Retrograde Vector (`⊗`)**: Amber marker pointing opposite your drift vector (burn toward this to arrest velocity).
* **Target Lock & Perimeter Pointers**: Diamond target reticles (`◇`) with off-screen directional perimeter arrows guiding you to targets outside your viewport.
* **Real-time Telemetry Panel**: Live readout of target name, distance, closing/opening velocity, and estimated time to intercept (ETA).
* **Docking Approach Corridor**: Concentric glowing alignment gates extending from Space Station Alpha's docking port when within $150\text{ m}$.
* **In-Cockpit Flight Manual (`H`)**: On-screen vector HUD manual displaying all keybindings and flight features.

---

## 🎮 Controls & Flight Deck

SpaceSim supports **pure dual-hand keyboard control** (Apollo/Flight Sim standard) as well as **mouse flight mode**. No mouse interaction is required to fly.

```
        LEFT HAND: Translation (Position)           RIGHT HAND: Rotation (Attitude)
       ┌───┬───┬───┐                               ┌───┬───┬───┐   ┌───┬───┬───┐
       │ Q │ W │ E │  Q/E: Roll Left / Right       │ 7 │ 8 │ 9 │   │   │ ▲ │   │  ▲/8: Pitch Down
       ├───┼───┼───┤  W/S: Thrust Fwd / Rev        ├───┼───┼───┤   ├───┼───┼───┤  ▼/2: Pitch Up
       │ A │ S │ D │  A/D: Strafe Left / Right     │ 4 │ 5 │ 6 │   │ ◄ │ ▼ │ ► │  ◄/4: Yaw Left
       └───┴───┴───┘                               ├───┼───┼───┤   └───┴───┴───┘  ►/6: Yaw Right
       [SPACE]  - Vertical Elevate Up              │ 1 │ 2 │ 3 │
       [L-CTRL] - Vertical Elevate Down            └───┴───┴───┘   [NUMPAD 5] - Level / Stop Spin
       [L-SHIFT]- Afterburner Boost (3.2x)
```

### Complete Control Reference

| Action | Primary Key | Numpad / Mouse Alt | Function |
|---|---|---|---|
| **Pitch Down** | **Up Arrow** | **Numpad 8** / Mouse Y | Push nose down (or up if inverted) |
| **Pitch Up** | **Down Arrow** | **Numpad 2** / Mouse Y | Pull nose up (or down if inverted) |
| **Yaw Left** | **Left Arrow** | **Numpad 4** / Mouse X | Turn nose left |
| **Yaw Right** | **Right Arrow** | **Numpad 6** / Mouse X | Turn nose right |
| **Roll Left** | **Q** | **Numpad 7** | Roll counter-clockwise |
| **Roll Right** | **E** | **Numpad 9** | Roll clockwise |
| **Level / Kill Spin** | — | **Numpad 5** | Instantly zeroes angular tumbling |
| **Pitch Invert** | **I** | — | Toggles between Normal and Flight Stick pitch feel |
| **Turn Sensitivity** | **[ / ]** | — | Decreases / Increases keyboard turn speed |
| **Thrust Forward** | **W** | — | Fire main engine forward (local $-Z$) |
| **Thrust Reverse** | **S** | — | Fire retro-thrusters (local $+Z$) |
| **Strafe Left / Right** | **A / D** | — | Port / Starboard lateral translation |
| **Elevate Up / Down** | **Space / Left Ctrl** | — | Vertical translation |
| **Afterburner Boost** | **Left Shift (Hold)** | — | **$3.2\times$ thrust boost** ($20.8\text{ m/s}^2$) |
| **Cruise Speed (+)** | **= / +** | **Numpad +** / Wheel Up | Increments target cruise speed ($+2.5\text{ m/s}$) |
| **Cruise Speed (-)** | **- / _** | **Numpad -** / Wheel Down | Decrements target cruise speed ($-2.5\text{ m/s}$) |
| **Zero Cruise Throttle**| **0 / Backspace** | **Numpad 0** | Resets cruise throttle to $0\text{ m/s}$ |
| **Retro-Brake** | **X / B** | — | Exponentially brakes drift to a dead stop |
| **Emergency Halt** | **F** | — | Instantly halts all linear and angular momentum |
| **Toggle Flight Assist**| **Z** | — | Switch between **FA-ON** (Fly-By-Wire) and **FA-OFF** (Newtonian) |
| **Next / Prev Target** | **T / G** | — | Cycle through orbital targets |
| **Target Nearest** | **Y** | — | Lock onto closest object |
| **Autopilot: Target** | **U (Hold)** | — | Auto-steers nose directly toward locked target |
| **Autopilot: Prograde** | **P (Hold)** | — | Auto-steers nose along forward velocity vector |
| **Autopilot: Retro** | **O (Hold)** | — | Auto-steers nose opposite velocity for braking |
| **Toggle HUD Manual** | **H / F1** | — | Displays the full keybinding manual on the HUD |
| **Mouse Flight Mode** | **Left Click** | — | Captures mouse cursor for 3DOF mouse steering |
| **Release Mouse** | **Tab / ESC** | — | Frees mouse cursor |
| **Reset Spacecraft** | **R** | — | Teleports back to Space Station Alpha dock |
| **Exit** | **ESC (twice)** | — | Closes the simulator |

---

## 🧭 Navigation & Flight Systems

### Flight Assist Modes (`Z`)
* **FA-ON (Fly-By-Wire)**: Counter-thrusters automatically fire when translation keys are released to arrest drift. Rotational damping (SAS) automatically stabilizes attitude when steering keys are released.
* **FA-OFF (Pure Newtonian)**: All thruster forces are conserved indefinitely. Point in any direction while continuing to coast along your original vector.

### Autopilot Alignment Computers (`U`, `P`, `O`)
Holding an autopilot key engages the flight computer to slew the spacecraft's nose toward:
* **`U` (Target Lock)**: Smoothly points directly at your targeted celestial body or station.
* **`P` (Prograde Vector)**: Aligns with your velocity vector for orbital burns.
* **`O` (Retrograde Vector)**: Aligns opposite your velocity vector for retrograde deceleration burns.

---

## 📁 Project Structure

```
SpaceSim/
├── CMakeLists.txt              # CMake build configuration
├── run_spacesim.bat            # One-click Windows desktop launcher
├── README.md                   # Project documentation
│
├── src/
│   ├── main.cpp                # Render loop, scene setup, HUD builder, input dispatch
│   ├── camera/
│   │   ├── Camera.h            # 6DOF Quaternion Camera & flight systems header
│   │   └── Camera.cpp          # Newtonian physics, FA modes, autopilot alignment
│   └── rendering/
│       ├── Mesh.h              # Mesh structures & procedural factory declarations
│       ├── Mesh.cpp            # Procedural generation (planet, station, asteroids, HUD)
│       ├── Shader.h            # GLSL shader program manager header
│       ├── Shader.cpp          # GLSL compile, link, uniform caching
│       └── shaders/
│           ├── starfield.vert  # Optical-infinity celestial starfield vertex shader
│           ├── starfield.frag  # Scintillation point star fragment shader
│           ├── planet.vert     # Planetary sphere transform vertex shader
│           ├── planet.frag     # Procedural continent, ocean, cloud, atmosphere shader
│           ├── sun.vert        # Stellar body vertex shader
│           ├── sun.frag        # Solar limb darkening & corona fragment shader
│           ├── dust.vert       # Toroidal microgravity space dust vertex shader
│           ├── dust.frag       # Particle point sprite fragment shader
│           ├── hud.vert        # 2D vector HUD overlay vertex shader
│           ├── hud.frag        # HUD vector line fragment shader
│           ├── object.vert     # Spacecraft & asteroid MVP vertex shader
│           └── object.frag     # Blinn-Phong lighting & strobe beacon fragment shader
│
└── vendor/
    ├── glad/                   # GLAD2-generated OpenGL 3.3 Core loader
    └── glm/                    # GLM header-only mathematics library
```

---

## 🛠 Prerequisites

* **C++ Compiler**: MinGW-w64 GCC 13+ (or modern MSVC / Clang) with C++17 support.
* **CMake**: Version 3.20+ (portable binary included in `cmake_bin/`).
* **GPU**: OpenGL 3.3 Core Profile compatible GPU (Intel UHD Graphics, NVIDIA, AMD).

---

## 🔨 Build Instructions

Using MinGW and CMake:

```powershell
# Set compiler in PATH (if using portable MinGW):
$env:PATH = "C:\ProgramData\mingw64\mingw64\bin;$env:PATH"

# Configure:
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build:
cmake --build build -j4
```

Shaders are automatically copied into `build/bin/shaders/` as a post-build step.

---

## ▶️ Running the Simulator

### Quick Launch
Double-click **`run_spacesim.bat`** in the project root.

### Terminal Launch
```powershell
cd build\bin
.\SpaceSim.exe
```

> **Note:** Always run the executable with `build\bin` as the working directory so shader files are located properly.

---

## 📦 Dependencies

| Library | Version | How Obtained | License |
|---|---|---|---|
| **GLFW** | 3.4 | CMake `FetchContent` | zlib |
| **GLM** | 1.0.1 | CMake `FetchContent` | MIT |
| **GLAD2** | 3.3 Core | Vendored in `vendor/glad/` | MIT |
| **OpenGL** | 3.3 Core | System driver (`opengl32.dll`) | — |

No external package managers (vcpkg, Conan) are required. All dependencies are self-contained or auto-downloaded by CMake.

---

*SpaceSim is built from scratch without physics middleware to demonstrate authentic microgravity orbital mechanics and real-time procedural OpenGL rendering.*
