# Draughts
C++ Draughts game with AI opponent and with command-line or OpenGL UI

## Compiling the command-line version

At the top level (Draughts/), run:
```
  g++ draughts.cpp board.cpp -o draughts.exe -std=c++11
```

## Compiling the OpenGL version

### Libraries

Download the following libraries (the versions used in this project are listed below):

- freeglut 3.0.0  (https://sourceforge.net/projects/freeglut/)

- glew 2.0.0      (http://glew.sourceforge.net/)

- glm 0.9.7.6     (http://glm.g-truc.net/0.9.8/index.html)

- soil            (http://lonesock.net/soil.html)
  

Different versions of the libraries may work, but have not been tested.


Create the following folders at the top level:
```
  Draughts/libs_opengl/freeglut-3.0.0/
  
  Draughts/libs_opengl/glew-2.0.0/
  
  Draughts/libs_opengl/glm/
  
  Draughts/libs_opengl/soil/
```


Put all of the library files into the appropriate folders

  e.g. for the freeglut files the directory structure should end up looking something like:
```
  Draughts/libs_opengl/freeglut-3.0.0/android/

  Draughts/libs_opengl/freeglut-3.0.0/doc/

  Draughts/libs_opengl/freeglut-3.0.0/include/

  Draughts/libs_opengl/freeglut-3.0.0/progs/

  etc.
``` 

### Eclipse

Eclipse project files (.cproject, .project) are located in the following directory:
```
  Draughts/gui_opengl/draughts_opengl
```


The program should build using Eclipse set up to compile using the MinGW-w64 compilers


The inlude paths in the Eclipse project will need to be changed if the naming of the folders inside "libs_opengl" is different to that described above.
