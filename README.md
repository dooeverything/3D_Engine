# OpenGL_Engine

### Screen Shot
![Screenshot](https://github.com/dooeverything/OpenGL_Engine/blob/main/ScreenShots/screenshot-5.png)

### About this project 
I am making this engine just for learning computer graphics using OpenGL and C++, and it is just a hobby project. This project is mostly inspired by [learnopengl.com](learnopengl.com), which is a perfect tutorial for computer graphics beginners, and later I found some more interesting references to understand and build 3D engine features, such as [Real Time Rendering](https://www.amazon.com/Real-Time-Rendering-Fourth-Tomas-Akenine-M%C3%B6ller/dp/1138627003), [Physically Based Rendering](https://www.amazon.com/Physically-Based-Rendering-Theory-Implementation/dp/0128006455), which helped me to dive deep into mathematical and computational theories behind computer graphics.  

### Features include
 1. 3D model loading (.fbx)
 2. 3D game engine like gui (by using ImGui)
 3. Phong Shading Model
 4. 3D Gizmos (only translation)
 5. Outline for picking objects (by using [jump flood algorithm](https://www.comp.nus.edu.sg/~tants/jfa.html))
 6. Shadow (Directional light)
 7. Load and apply texture to a game object
 8. Physically Based Rendering (PBR)
 9. 3D grid
 10. Marching Cube (Terrain, Metaball)
 11. Terrain Editor (Sculpting and removing)

### Libraries
 1. C++17
 2. OpenGL 3.3
 3. [glm](https://github.com/g-truc/glm)
 4. [ImGui](https://github.com/ocornut/imgui)
 5. [stb library](https://github.com/nothings/stb)

### References
 1. [Learn OpenGL](learnopengl.com)
 2. [Game Programming in C++](https://www.amazon.com/Game-Programming-Creating-Games-Design/dp/0134597206)
 3. [Real Time Rendering, 4th Edition](https://www.amazon.com/Real-Time-Rendering-Fourth-Tomas-Akenine-M%C3%B6ller/dp/1138627003)
 4. [Computer Graphics Principles and Practice, 3/E](https://www.amazon.com/Computer-Graphics-Principles-Practice-3rd/dp/0321399528)
 5. [Physically Based Rendering, 3rd edition](https://www.amazon.com/Physically-Based-Rendering-Theory-Implementation/dp/0128006455)

### Mouse Inputs
 1. With the left mouse button being clicked, press W/A/S/D to move around
 2. Right mouse button to pick and move the object and to play around with GUI
 
### Features to be added
- [ ] Shadow (Point light)
- [ ] Skeleton Animation
- [ ] Environment mapping
- [ ] enhanced fbx loader
- [ ] Bump mapping
- [X] Marching Cube
- [X] Terrain
- [ ] 3D gizmos for rotation and scale
