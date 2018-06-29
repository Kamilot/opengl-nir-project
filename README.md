To build the project following libraries should be installed in the 3rd-party folder:
- GLFW (https://github.com/glfw/glfw)
- GLAD (https://github.com/Dav1dde/glad)
- GLM (https://github.com/g-truc/glm)
- ASSIMP (https://github.com/assimp/assimp)

Models and Motion Capture files should be put under *resources/models*.  
Textures should be put under *resources/textures*.

# Linking instructions:
Cmake files are set up to build project under linux system:

Recommended libraries (might be required, didn't run into the building issue):
```
X11 Xi Xrandr pthread
```
required libraries:
```
glfw3 GL dl
```
