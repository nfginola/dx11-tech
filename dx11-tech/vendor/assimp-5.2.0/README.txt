To rebuild from the Assimp Release:

1. Generate solution with CMake
2. Copy include folder containing config.h into the generated solutions include directory
3. Build the solution in Release mode (Debug mode is slow)
4. Get the .dll from the bin folder
5. Get the .lib from the lib folder

6. Use the includes in your project
7. Place the .dll in your working directory and use Post-Build event to paste if necessary 
8. Add .lib folder to Lib Directories
9. Link the .lib file
10. Done!