To replicate our results, you can follow the next steps:

- Java: Maven should take car of all the imports. The benchmarks can simply be instrumented and run on any IDE. 

- Solidity: We used the 0.7.0 version of Solidity compiler (solc).
    - pip3 install solc-select
    - solc-select install 0.7.0
    - solc-select use 0.7.0
    - From the Solidity directory run:
        - cd bsol/ && sh install.sh && cd .. && bsol --sol PATH-TO-BENCHMARK-FILE --execution-time --runs 1
    - NOTE: For this last step to run our instrumented version of Geth (and output gas breakdowns), you can either swap the implementations of the bsol imported files or change your local Geth files globally. You can simply replace the go-ethereum directory completely. The modified file was the interpreter.go file located in the core/vm.

- C (WASM):
    - For neural networks:
        - Start by installing Emscripten
        - emcc Simulation.c ClassificationContract.c MatrixNNUtils.c Random.c ./gsl-2.7.1/.libs/libgsl.so.27 PATH/neural_network/gsl-2.7.1/cblas/*.o -I PATH/neural_network/gsl-2.7.1 -lm -s ALLOW_MEMORY_GROWTH=1
        -  This should work on any machine since the files are precompiled on WASM (so no need to build). If this does not work, launch a new emmake build.
    - For revenue distribution: emcc (Non)OptimizedSimulation.c DistributionContract.c hashmap.c -s ALLOW_MEMORY_GROWTH=1

You must modify the source code files of the benchmarks manually to change the parameters.