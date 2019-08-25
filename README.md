# RTES_Project

## Introduction

Telescopes. Simulate an array of N telescopes (positioned at the bottom of the screen) controlled to point at the centroid of the image of a moving planet (passing on top of the screen). Each telescope introduces some random noise, hence the system integrates the various images to produce an average output image. All images must be shown on the screen. The program must allow the user to change the noise level and motor control parameters of each telescope.

## Instructions

### Prerequisites

sudo apt-get install git\
sudo apt-get install cmake\
sudo apt-get install liballegro4.4 liballegro4-dev


Ptask is also required:

git clone https://github.com/glipari/ptask.git  \
mkdir build\
cd build\
cmake ..\
make\
make install

### Compile and Run

Running the program:\
mkdir build\
cmake ..\
make\
sudo ./observatory
   
