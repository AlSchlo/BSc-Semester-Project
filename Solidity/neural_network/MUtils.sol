// SPDX-License-Identifier: MIT

pragma solidity ^0.7.0;

import "./F.sol";
import "./M.sol";
import "./R.sol";

// This library is functional although solidity does no garbage collection
// It is easier to understand
library MUtils { 

    using F for bytes16;
    using M for M.Matrix;
    using R for R.Random;

    function relu(M.Matrix memory v) internal pure isVector(v) returns (M.Matrix memory result) {
        result.rows = v.rows;
        result.cols = v.cols;
        result.data = new bytes16[](v.data.length);
        bytes16 zero = F.fromUInt(0);
        for(uint i = 0; i < result.data.length; i++) {
            bytes16 val = v.data[i];
            int8 cmp = val.cmp(zero); 
            result.data[i] = cmp >= 0 ? val : zero;
        }
    }

    function identity(M.Matrix memory v) internal pure isVector(v) returns (M.Matrix memory) {
        return v;
    }

    function dIdentityDx(M.Matrix memory v) internal pure isVector(v) returns (M.Matrix memory) {
        return M.eye(v.data.length);
    }

    function dL2Dx(M.Matrix memory v1, M.Matrix memory v2) internal pure isVector(v1) isVector(v2) returns (M.Matrix memory) {
        M.Matrix memory derivative = v1.minus(v2).scale(F.fromUInt(2));
        return derivative.reshape(1, v1.data.length);
    }

    function affine(M.Matrix memory x, M.Matrix memory W) internal pure isVector(x) returns (M.Matrix memory) {
        M.Matrix memory one;
        one.rows = 1;
        one.cols = 1;
        one.data = new bytes16[](1);
        one.data[0] = F.fromUInt(1);
        if(x.rows == 1) {
            x = x.transpose();
        }
        M.Matrix memory extendedX = x.verticalStack(one);
        return W.mul(extendedX);
    }

    function initMatrix(uint rows, uint cols) internal pure returns (M.Matrix memory result) {
        result.rows = rows;
        result.cols = cols;
        result.data = new bytes16[](result.rows * result.cols);
        R.Random memory rand = R.initSeed(0);
        bytes16 high = F.sqrt(F.fromUInt(3).div(F.fromUInt(cols)));
        bytes16 low = high.mul(F.fromInt(-1));
        for(uint i = 0; i < result.data.length; i++) {
            result.data[i] = rand.nextFloat(low, high);
        }
    }

    function initNetwork(uint[] memory dimensions) internal pure returns (M.Matrix[] memory) {
        uint nbDimensions = dimensions.length; 
        require(nbDimensions > 1);
        M.Matrix[] memory result = new M.Matrix[](nbDimensions - 1);
        bytes16 zero = F.fromUInt(0);
        for(uint i = 0; i < nbDimensions - 1; i++) {
            uint Wi = dimensions[i];
            uint Wip1 = dimensions[i + 1];

            M.Matrix memory zeros;
            zeros.rows = Wip1;
            zeros.cols = 1;
            zeros.data = new bytes16[](Wip1);
            
            for(uint j = 0; j < zeros.data.length; j++) {
                zeros.data[j] = zero;
            }
            result[i] = initMatrix(Wip1, Wi).horizontalStack(zeros);
        }
        return result;
    }

    function dReluDx(M.Matrix memory x) internal pure isVector(x) returns (M.Matrix memory result) {
        uint nbElements = x.data.length;
        result.rows = nbElements;
        result.cols = nbElements;
        result.data = new bytes16[](result.rows * result.cols);

        bytes16 zero = F.fromUInt(0);
        bytes16 one = F.fromUInt(1);

        for(uint i = 0; i < nbElements; i++) {
            bytes16 val = x.data[i];
            int8 cmp = val.cmp(zero); 
            result.setValue(i, i, cmp > 0 ? one : zero);
        }
    }

    function dAffineDx(M.Matrix memory W) internal pure returns (M.Matrix memory) {
        return W.takeCols(0, W.cols - 1);
    }

    function dAffineDw(M.Matrix memory x, M.Matrix memory W) internal pure isVector(x) returns (M.Matrix memory result) {
        uint nbRows = W.rows;
        uint nbElements = x.data.length;
        result.rows = nbRows;
        result.cols = nbRows * (nbElements + 1);
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < nbRows; i++) {
            for(uint j = 0; j < nbElements; j++) {
                result.setValue(i, i * (nbElements + 1) + j, x.data[j]);
            }
            result.setValue(i, i * (nbElements + 1) + nbElements, F.fromUInt(1));
        }
    }

    function backpropagation(M.Matrix memory x, M.Matrix memory y, M.Matrix[] memory allW) internal pure isVector(x) isVector(y) returns (M.Matrix[] memory) {
        uint nbLayers = allW.length;
        M.Matrix[] memory inputs = new M.Matrix[](nbLayers);
        M.Matrix[] memory combinations = new M.Matrix[](nbLayers);

        // Forward propagation
        for(uint i = 0; i < nbLayers; i++) {
            inputs[i] = x;
            M.Matrix memory comb = affine(x, allW[i]);
            combinations[i] = comb;
            x = i == nbLayers - 1 ? identity(comb) : relu(comb);
        }

        M.Matrix memory currJac = dL2Dx(x, y);
        // Backpropagation
        M.Matrix[] memory jacs = new M.Matrix[](nbLayers);
        for(uint r = 0; r < nbLayers; r++) {
            // Go in reverse order
            uint i = nbLayers - 1 - r;
            M.Matrix memory comb = combinations[i];
            M.Matrix memory W = allW[i];
            currJac = i == nbLayers - 1 ? currJac.mul(dIdentityDx(comb)) : currJac.mul(dReluDx(comb));
            M.Matrix memory Jw = currJac.mul(dAffineDw(inputs[i], W));
            jacs[i] = Jw.reshape(W.rows, W.cols);
            currJac = currJac.mul(dAffineDx(W));
        }

        return jacs;
    }

    function train(M.Matrix memory trainInput, M.Matrix memory trainOutput, uint[] memory dimensions, uint numEpoch, bytes16 learningRate) internal pure returns (M.Matrix[] memory) {
        M.Matrix[] memory allW = initNetwork(dimensions);
        uint nbSamples = trainInput.rows;
        // Loop over all epochs
        for(uint i = 0; i < numEpoch; i++) {
            // Loop over all samples
            for(uint r = 0; r < nbSamples; r++) {
                M.Matrix[] memory jacs = backpropagation(trainInput.takeRows(r, r + 1).transpose(), trainOutput.takeRows(r, r + 1).transpose(), allW);

                // Update weights
                for(uint j = 0; j < allW.length; j++) {
                    M.Matrix memory oldW = allW[j];
                    M.Matrix memory jac = jacs[j];
                    allW[j] = oldW.minus(jac.scale(learningRate));
                }
            }
        }

        return allW;
    }

    function nn(M.Matrix memory x, M.Matrix[] memory allW) internal pure isVector(x) returns (M.Matrix memory) {
        for(uint i = 0; i < allW.length; i++) {
            x = relu(affine(x, allW[i]));
        }
        return identity(x);
    }

    // -- For argument sanitization -- //
    
    modifier isVector(M.Matrix memory m) {
        require(m.rows == 1 || m.cols == 1);
         _;
   }
}