// SPDX-License-Identifier: MIT

pragma solidity ^0.7.0;

import "./ClassificationContract.sol";

contract BenchmarkClassification is ClassificationContract {

    uint constant NB_TRAIN_SAMPLES = 100;
    uint constant NB_FEATURES = 5;
    uint constant NB_TEST_SAMPLES = 2;
    uint constant NB_CLASSES = 2;

    uint constant NB_EPOCH = 1;
    uint constant LEARNING_RATE = 10;

    uint constant NB_LAYERS = 1;
    uint constant LAYER_SIZE = 3;

    bool normalized = true;
    bool trained = false;

    constructor() 
        ClassificationContract(generateData(NB_TRAIN_SAMPLES, NB_FEATURES), generateData(NB_TRAIN_SAMPLES, NB_CLASSES), 
            generateData(NB_TEST_SAMPLES, NB_FEATURES), generateData(NB_TEST_SAMPLES, NB_CLASSES), generateLayers(),
            NB_TRAIN_SAMPLES, NB_FEATURES, NB_TEST_SAMPLES, NB_CLASSES) {
                if(normalized) {
                    super.normalize();
                    if(trained) {
                        super.train(NB_EPOCH, LEARNING_RATE);
                    }
                }
            }

    function generateData(uint rows, uint cols) private pure returns (int[] memory) {
        int[] memory data = new int[](rows * cols);
        for (uint i = 0; i < data.length; i++) {
            data[i] = int(i % 1000);
        }
        return data;
    }

    function generateLayers() private pure returns (uint[] memory) {
        uint[] memory layers = new uint[](NB_LAYERS);
        for(uint i = 0; i < NB_LAYERS; i++) {
            layers[i] = LAYER_SIZE;
        }
        return layers;
    }

    /*function BenchmarkNormalize() external {
        super.normalize();
    }*/
    
    function BenchmarkTrain() external {
        super.train(NB_EPOCH, LEARNING_RATE);
    }

    /*function BenchmarkTest() external view returns (uint) {
        return super.test();
    }*/
}