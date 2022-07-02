<h1 align="center">
 <br />
   <img src="EPFL.png" width="200">  <br />
 <br />
</h1>


<h2 align="center">
    <br />
    <font size="5"> Numerically-Intensive Deterministic Smart Contracts <br /></font>
    <br />
    <font size="3">School of Computer and Communication Sciences<br />
    Decentralized and Distributed Systems Lab <br />
    BSc Semester Project <br />  
    <br />
    June 2022</font>  
    <br />
    <br />
</h2>

<br />
<p align="center">
  <a href="#Presentation">Presentation</a> - 
  <a href="#Setup">Setup</a> -
  <a href="#Author">Author</a> -
  <a href="#Thanks">Thanks</a>
</p>

# Presentation

This project studies *two* smart contract applications and compares their implementation on *three* execution platforms in terms of **efficiency** and **ease of programming**.

You can read our full report [here](Project-Report.pdf) for more details. This project was realized in the context of a BSc semester project in the DEDIS laboratory at EPFL.

## Applications

The following applications were chosen as they encompass a *wide* range of common problems when coding smart contracts. Emphasis was placed on numerically intensive applications that use **floating point arithmetic**.

### 1. Revenue Distribution

A generic and flexible version of a revenue distribution application. We present the default naive implementation of such problem as well as a more sophisticated optimized approach. You can check out [this](optimized-revenue-distribution.pdf) for more details.

### 2. Neural Network

An arbitrary complex multilayer perceptron classifier that uses ReLu as activation function and stochastic gradient descent for training. Supports normalization, training and testing as operations on any shape of the network.

## Execution Environments

We compared all the aforementioned applications on the following execution environments.

- Ethereum Virtual Machine (EVM) : on **Solidity**
- Web Assembly (WASM) :  on **C**
- Java Virtual Machine (JVM) : on **Java**

# Setup

You can follow these steps to reproduce our results on every execution environments.

## JVM

Maven should take car of all dependencies. Benchmarks can be instrumented to select what application to run, using the *JMH* framework.

## EVM

We used the **0.7.0** version of Solidity compiler (solc). You can run the following commands to benchmark and test our applications.

```sh
pip3 install solc-select
solc-select install 0.7.0
solc-select use 0.7.0
cd path-to-Solidity-dir
cd bsol/ && sh install.sh && cd .. && bsol --sol path-to-file --execution-time --runs 1
```

<u>Note:</u> For this last step to run our instrumented version of *Geth* and not the original one, you can replace the *Geth* directory content located at /go/pkg/mod/github.com/ethereum (on your machine) with the content of Solidity/go-ethereum.

Although not very clean, this solution is quick and easy. You are free to create a new package and change the import directives in the Go source code.

You can modify the constants present in the benchmarking source code to set the desired benchmarking parameters.

## WASM

If necessary, install Emscripten using the following commands.

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
# Activate PATH and other environment variables in the current terminal (has to be rerun when changing terminal)
source ./emsdk_env.sh
```

For the neural network application, you can proceed as follows.

```sh
cd path-to-neural-network
# This should work on any machine since the files are precompiled on WASM (so no need to build). If this does not work, launch a new emmake build
emcc Simulation.c ClassificationContract.c MatrixNNUtils.c Random.c ./gsl-2.7.1/.libs/libgsl.so.27 PATH/neural_network/gsl-2.7.1/cblas/*.o -I PATH/neural_network/gsl-2.7.1 -lm -s ALLOW_MEMORY_GROWTH=1
# To launch a new emmake build (if the previous command fails)
cd gsl-2.7.1
emmake make
```
And for the revenue distribution application, you only need to run these commands.

```sh
emcc [Non]OptimizedSimulation.c DistributionContract.c hashmap.c -s ALLOW_MEMORY_GROWTH=1
```

In both cases, you can modify the macros in the source code to set the desired benchmarking parameters.

# Thanks

This project has been supervised by EPFL PhD Student Enis Ceyhun Alp, under the responsability of Prof. Bryan Ford.

We would like to express our gratitude for their support and help in the realization of this project.

# Author

This work has been realized by EPFL BSc Student Alexis Schlomer. You can contact me for any inquiry using the following email address: alexis.schlomer@epfl.ch.
