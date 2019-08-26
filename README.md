# RTES_Project

## Introduction

Telescopes. Simulate an array of N telescopes (positioned at the bottom of the screen) controlled to point at the centroid of the image of a moving planet (passing on top of the screen). Each telescope introduces some random noise, hence the system integrates the various images to produce an average output image. All images must be shown on the screen. The program must allow the user to change the noise level and motor control parameters of each telescope.

Parameters can be modified on the start page:

![ ](media/Start_UI.jpg?raw=true "Start page")

When the user selects "OK", the simulation begins:

![ ](media/Obs1.jpg?raw=true "Simulation")

Each telescope has an observation window (in other words, it's what a telescope can see), that is portraied at the bottmo of the screen. When a telescope centers the planet, an image is taken and it stops.\
When all telescopes took an image of the planet, an average output image is computed.

![ ](media/Obs3.png?raw=true "Result")

---

## Instructions

### Prerequisites

>sudo apt-get install git\
sudo apt-get install build-essential\
sudo apt-get install cmake\
sudo apt-get install liballegro4.4 liballegro4-dev


**Ptask is also required**:

git clone https://github.com/glipari/ptask.git  \

In ptask folder:\
>mkdir build\
cd build\
cmake ..\
make\
make install

### Compile and Run

In project folder:\
>mkdir build\
cmake ..\
make\
sudo ./observatory
   
