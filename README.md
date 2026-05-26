# 🦖 CanyonSkull-OS (Dino Runner Clone)

A C++ SFML-based endless runner game inspired by Google Chrome Dino, featuring:

- Dynamic day/night cycle 🌅🌙
- Auto-play mode 🤖
- Increasing difficulty system 📈
- Sound effects + background music 🎵
- High score persistence 💾
- FPS counter 📊

---

## 🎮 Gameplay
Jump over cacti and flying birds. Survive as long as possible while speed increases over time.

Controls:
- Space → Jump
- ESC → Pause/Resume
- Ctrl + A → Toggle AutoPlay
- R → Restart after death

---

## 🧠 Features (Technical)
- Object-oriented design (Dino, Obstacles, GameState, etc.)
- SFML graphics, audio, and window system
- Collision detection using bounding boxes
- Frame-based sprite animation
- File I/O for high score saving
- Procedural obstacle spawning
- Time-based difficulty scaling

---

## 🛠 Build Instructions

### Requirements
- SFML 2.5+
- C++17
- g++

### Build (Linux/Mac)
```bash
make
./CanyonSkull
