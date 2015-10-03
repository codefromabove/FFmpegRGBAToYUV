FFmpegRGBAToYUV
===============

An application may generate sequences of "raw" RGB or RGBA images, for example a 3D application that renders an animation. In such cases, it would be useful to create a movie file in some standard format. FFmpeg has the capability to create movie files, but as far as I could determine, no direct way to dump raw RGB(A) data.

I found a nice solution on stackoverflow:

  http://stackoverflow.com/questions/16667687/how-to-convert-rgb-from-yuv420p-for-ffmpeg-encoder
 
and created a little program to test it. I got a copy of an example C file from the FFmpeg web site:

  https://www.ffmpeg.org/doxygen/2.1/decoding__encoding_8c.html

and hacked up the video output function it contained, to convert some synthetically generated RGBA values to YUV, and then out to a file.

For Mac users, I've provided an Xcode 6 project; it assumes you've used MacPorts to install the FFmpeg libraries and headers in /opt/local ("sudo port install ffmpeg-devel"). But it's fairly generic C/C++, so compiling on Linux should be simple.

Have a look at the [Code From Above](http://codefromabove.com/2014/10/ffmpeg-convert-rgba-to-yuv/) blog post, which discusses this solution.
