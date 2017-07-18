Dependencies Following packages are required to compile the application.
We also provide versions of packages used to create and test our implementation.
package version
libbullet-dev 2.83.7+dfsg-5
libirrlicht-dev 1.8.4+dfsg1-1
libcgal-dev 4.9-1build2
voro++-dev 0.4.6+dfsg1-2
HACD is bundled with the application1
A C++ compiler capable of compiling the C++14 standard is also needed,
we have used GCC version 6.3.0-2ubuntu1 with flags -std=c++14 -O2 -frounding-math.

To compile the application use make from this directory.
