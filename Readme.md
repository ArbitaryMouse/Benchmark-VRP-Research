# Benchmarking the Vehicle Routing Problem
This Repl is designated to solve the problem of the VRP problem using one parallel algorithm and measure the speedup.

## Important: How to run this program
To run the parallization of the program, we need to type two commands in the **Linux Shell**:

Or it may output runtime errors.

### Warning! When our group tested our code, we did not install any packages. However, we acknowledge that cases will be different on different devices, so runner may need to install `omp.h` or other essential packages.

### serial program

Command 1: `g++ -o main main.cpp`

Command 2: `./main`

### parallel program

Before you run our program, you need to think about the number of processors you are using.

Command 1: `g++ -o para_omp -fopenmp para_omp.cpp`

Command 2: `./para_omp THE_NUMBER_OF_PROCESSORS_YOU_ARE_USING`

for example, if you want to use 128 processors, in Command 2 you type: `./para_omp 128`

***Note***: Please be aware that the input file read by the program is fixed as 'M-n200-k16.vrp'. To test mutiple datasets provided in the folder, change the parameter in **line 33** of **'main.cpp'** and **line 35** of **'para_omp.cpp'**, i.e. **ifstream myfile ("M-n200-k16.vrp");**, to other file names such as 'B-n31-k5.vrp'.

## VRP Problem

The vehicle routing problem (VRP) is a combinatorial optimization and integer programming problem which has been widely used to optimize industrial problems. It aims to minimize routing costs, given a set of vehicles delivering goods to a set of customers.
![VRP Problem Demo](https://www.mdpi.com/processes/processes-08-01363/article_deploy/html/images/processes-08-01363-g001-550.jpg)

## Introduction
This research project is intended to design a parallization algorithm based upon a serial algorithm of the VRP problem, and measure the speedup of the vehicle routing problem and evaluate its performance based on different number of processors.

## High Level idea
This problem is followed by several steps of code:

1. Make n routes: v0 -> vi -> v0, for each i >= 1;
2. Compute the savings for merging delivery locations i and j, which is given
by s_ij = d_0i + d_0j - d_ij;
3. Sort the savings in decending order
4. Starting at the top of the (remaining) list of savings, merge the two routes associated with the largest (remaining) savings, provided that:
* The two delivery locations are not already on the same route;
*  Neither delivery location is interior to its route, meaning that both notes are still directly connected to the depot on their respective routes;
* The demand G and distance constraints D are not violated by the merged route.
5. Repeat step (3) until **no additional savings can be achieved**.

## Algorithm Details
When realizing the high-level idea described in the algorithm section into actual code, we used a 2-d triangle to represent distance between nodes. The distance between i and j is recorded in the index ( i , j ) of the 2-d triangle with i > j or ( j , i ) if j < i. 
Also, for sorting in step 3, we used the bubble sort algorithm in the serial environment.
Step 4 of the serial algorithm involves multiple substeps. We start with the top of the sorted list of savings returned by Step 3, identify the 2-node route, i.e. i -> j, that we are trying to merge to the current routes. 

## Rroject relevant files links
[Project Proposal](https://docs.google.com/document/d/1LiBV606uDWVpwARkNH5aghCL8QzSrRXZ0ccsRHc-H68/edit)

[Project Final Report](https://docs.google.com/document/d/1h3JIpmhggAmWbqeoD01FmbGD_c-g633qCGRqMp9a-l8/edit)

## Project Contributor
|Name|Email|
|----|-----|
|Runchen Zhang|runchenz@usc.edu|
|Ziang Qin|ziangq@usc.edu|