# leapVtk
Integrating camera control with the Leap Motion Controller with the VTK

This is the work I did during the Sprint 2015 LLNL Hackathon.

Dependencies:

* VTK 6.0.2 -> http://www.vtk.org/VTK/resources/software.html
* LeapSDK   -> https://developer.leapmotion.com/

You will need to alter the CMakeList.txt file to point to the location you installed the LeapSDK.

Right now Im just focusing on controlling the camera pan/tilt/zoom. Later if I have time, and access to the hardware, I
would love to add addition gesture control.

A video of the project in action can be found [here](https://www.youtube.com/watch?v=Bd_-4hPuJ-w)
