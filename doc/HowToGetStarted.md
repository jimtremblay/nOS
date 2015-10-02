# How to Get Started

### Create a new project

1. Create a new project with your preferred IDE.
2. Clone this repo anywhere on your disk.
3. Copy include and source folders in a subfolder of your project.
4. Copy TEMPLATE_nOSConfig.h from config folder in your project and rename it to nOSConfig.h
5. Add all source files from source folder and the Compiler/CPU specific files (nOSPort) to your project.
6. Add include folder and Compiler/CPU specific include folder to the path of your project.
7. In your main source file, include nOS.h and try to compile to see if you have any errors.
8. You shouldn't have any errors and be ready to create your app.

### Create a nOS app

1. You need to call nOS_Init before any other nOS functions and you should call it very soon in your main.
2. After nOS_Init, you can create any nOS objects (Mutex, Queue, Thread, ...) depending on the needs of your application.
3. When you're ready to allow context switching, you should call nOS_Start with a pointer to a function of your choice, if your need it; typically, you can initialize and start the tick timer in this function.
4. You should have the minimal nOS app.

[Return to index](https://github.com/jimtremblay/nOS/blob/master/doc/index.md)