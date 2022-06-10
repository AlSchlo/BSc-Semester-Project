// SPDX-License-Identifier: MIT

pragma solidity ^0.7.0;

import "./F.sol";
import "./M.sol";
import "./MUtils.sol";

/**
 * A classification contract that uses a multi-layer perceptron neural network.
 * Uses ReLu as intermediate activation functions.
 * Training is done with stochastic gradient descent.
 * @author Alexis Schlomer
 */
contract ClassificationContract {

    using M for M.Matrix;
    using F for bytes16;

    M.Matrix public trainInput;
    M.Matrix public testInput;

    M.Matrix public trainOutput;
    M.Matrix public testOutput;

    uint[] public dimensions;

    M.Matrix[] public allW;

    /**
     * Creates a new contract given raw (non-normalized) input data and their associated
     * output probability for every class
     * 
     * @dev Arrays are only one-dimensional since two-dimensional array are not yet supported by the ABI.
     * Thus we suppose row-majored arrays (and we pass the dimensions manually).
     * @dev The data is given as integer values, since floating points are not natively supported by the EVM.
     * This contract uses the ABDK library to emulate them (on 128b). Asking bytes16 as arguments could be done 
     * but requires impractical conversions (using the ABDK library, not standardized).
     *
     * @param X The raw N samples of M dimensions matrix (N x M), row-majored
     * @param Y The raw N classification encoded as a probability between the C classes (N x C), row-majored
     * @param tX The raw N samples of T dimensions matrix (T x M), row-majored
     * @param tY The raw N classification encoded as a probability between the C classes (T x C), row-majored
     *
     * @param hiddenLayers The dimensions (and number) of hidden layers
     * @param nbTrainSamples N 
     * @param nbFeatures M 
     * @param nbTestSamples T 
     * @param nbClasses C 
     */
    constructor(int[] memory X, int[] memory Y, int[] memory tX, int[] memory tY, uint[] memory hiddenLayers, uint nbTrainSamples, uint nbFeatures, uint nbTestSamples, uint nbClasses) {
        // Load and normalize the data
        trainInput = loadInMatrix(X, nbTrainSamples, nbFeatures);
        trainOutput = loadInMatrix(Y, nbTrainSamples, nbClasses);
        testInput = loadInMatrix(tX, nbTestSamples, nbFeatures);
        testOutput = loadInMatrix(tY, nbTestSamples, nbClasses);

        // Compute neural network dimensions
        uint nbHiddenLayers = hiddenLayers.length;
        dimensions = new uint[](nbHiddenLayers + 2);
        dimensions[0] = nbFeatures;
        for(uint i = 0; i < nbHiddenLayers; i++) {
            dimensions[i + 1] = hiddenLayers[i]; 
        }
        dimensions[nbHiddenLayers + 1] = nbClasses;
    }

    /**
     * Normalizes the input data.
     */
    function normalize() public {
        trainInput = trainInput.normalize();
        testInput = testInput.normalize();
    }

    /** 
     * Trains the data provided in the constructor with a given number of epoch and learning rate.
     * 
     * @param numEpoch The number of epoch (= rounds of stochastic gradient descent)
     * @param thousandthOfLearningRate The desired learning rate multiplied by 1'000.
     * Could be bytes16 but would require impractical conversions (using the ABDK library, not standardized).
     */ 
    function train(uint numEpoch, uint thousandthOfLearningRate) public {
        bytes16 learningRate = F.fromUInt(thousandthOfLearningRate).div(F.fromUInt(1000));
        M.Matrix[] memory result = MUtils.train(trainInput, trainOutput, dimensions, numEpoch, learningRate);
        
        // Copy data manually since not supported as such by Solidity
        delete allW; // Since it has to be dynamic, we don't want to keep previous results
        for(uint i = 0; i < result.length; i++) {
            allW.push(result[i]); // Has to be dynamic since else not supported by Solidity
        }
    }

    /**
     * Tests the contract with the provided testing dataset.
     * Return the accuracy in ‰ (per mille). Could be bytes16 but would require
     * impractical conversions (using the ABDK library, not standardized).
     * @return accuracy (per mille)
     */
    function test() public view returns (uint) {
        // Check if the model has been trained
        require(allW.length != 0);
        uint nbSame = 0;
        uint totalSamples = testInput.rows;
        for(uint i = 0; i < totalSamples; i++) {
            uint prediction = MUtils.nn(testInput.takeRows(i, i + 1).transpose(), allW).argMax();
            uint expected = testOutput.takeRows(i, i + 1).argMax();
            if(prediction == expected) nbSame++;
        }

        return F.toUInt(F.fromUInt(nbSame).div(F.fromUInt(totalSamples).mul(F.fromUInt(1000))));
    }

    function loadInMatrix(int[] memory data, uint rows, uint cols) internal pure returns (M.Matrix memory result) {
        uint nbElements = rows * cols;

        // Sanitization
        require(data.length == nbElements);
        result.cols = cols;
        result.rows = rows;
        result.data = new bytes16[](nbElements);
        
        // From int to float (ABDK)
        for(uint i = 0; i < nbElements; i++) {
            result.data[i] = F.fromInt(data[i]);
        }
    } 
}