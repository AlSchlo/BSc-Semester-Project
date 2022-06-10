// SPDX-License-Identifier: MIT

pragma solidity ^0.7.0;

import "./F.sol";

// This library is functional although solidity does no garbage collection
// It is easier to understand
library M {

    using F for bytes16;

    // Defines a matrix
    struct Matrix {
        // Dimensions
        uint rows;
        uint cols;
        // Data in floating point format, row-majored
        bytes16[] data;
    }

    function add(Matrix memory m1, Matrix memory m2) internal pure returns (Matrix memory result) {
        require(m1.rows == m2.rows && m1.cols == m2.cols);
        result.rows = m1.rows;
        result.cols = m1.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.data.length; i++) {
            result.data[i] = m1.data[i].add(m2.data[i]);
        }
    }
    
    function minus(Matrix memory m1, Matrix memory m2) internal pure returns (Matrix memory) {
        return add(m1, scale(m2, F.fromInt(-1)));
    }

    function add(Matrix memory m, bytes16 shift) internal pure returns (Matrix memory result) {
        result.rows = m.rows;
        result.cols = m.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.data.length; i++) {
            result.data[i] = m.data[i].add(shift);
        }
    }

    function minus(Matrix memory m, bytes16 shift) internal pure returns (Matrix memory) {
        return add(m, shift.mul(F.fromInt(-1)));
    }

    function scale(Matrix memory m, bytes16 factor) internal pure returns (Matrix memory result) {
        result.rows = m.rows;
        result.cols = m.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.data.length; i++) {
            result.data[i] = m.data[i].mul(factor);
        }
    }

    function divide(Matrix memory m, bytes16 factor) internal pure returns (Matrix memory) {
        return scale(m, F.fromUInt(1).div(factor));
    }

    function mul(Matrix memory m1, Matrix memory m2) internal pure returns (Matrix memory result) {
        require(m1.cols == m2.rows);
        result.rows = m1.rows;
        result.cols = m2.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.rows; i++) {
            for(uint j = 0; j < result.cols; j++) {
                bytes16 temp = 0;
                for(uint k = 0; k < m1.cols; k++) {
                    temp = temp.add(getValue(m1, i, k).mul(getValue(m2, k, j)));
                }
                setValue(result, i, j, temp);
            }
        }
    }

    function elementSquare(Matrix memory m) internal pure returns (Matrix memory result) {
        result.rows = m.rows;
        result.cols = m.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.data.length; i++) {
            result.data[i] = m.data[i].mul(m.data[i]);
        }
    }

    function elementSum(Matrix memory m) internal pure returns (bytes16) {
        bytes16 sum = 0;
        for(uint i = 0; i < m.data.length; i++) {
            sum = sum.add(m.data[i]);
        }
        return sum;
    }

    function verticalStack(Matrix memory m1, Matrix memory m2) internal pure returns (Matrix memory result) {
        require(m1.cols == m2.cols);
        result.rows = m1.rows + m2.rows;
        result.cols = m1.cols;
        result.data = new bytes16[](result.rows * result.cols);
        uint i;
        for(i = 0; i < m1.data.length; i++) {
            result.data[i] = m1.data[i];
        }
        for(uint j = 0; j < m2.data.length; j++) {
            result.data[i++] = m2.data[j];
        }
    }

    function horizontalStack(Matrix memory m1, Matrix memory m2) internal pure returns (Matrix memory) {
        return transpose(verticalStack(transpose(m1), transpose(m2)));
    }

    function argMax(Matrix memory v) internal pure returns (uint) {
        require(v.cols == 1 || v.rows == 1);
        int maxIdx = -1;
        bytes16 max = F.NEGATIVE_INFINITY;
        for(uint i = 0; i < v.data.length; i++) {
            bytes16 val = v.data[i];
            int8 cmp = val.cmp(max); 
            maxIdx = cmp >= 0 ? int(i) : maxIdx;
            max = cmp >= 0 ? val : max;
        }
        require(maxIdx >= 0);
        return uint(maxIdx);
    }

    function transpose(Matrix memory m) internal pure returns (Matrix memory result) {
        result.rows = m.cols;
        result.cols = m.rows;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < m.rows; i++) {
            for(uint j = 0; j < m.cols; j++) {
                setValue(result, j, i, getValue(m, i, j));
            }
        }
    }

    function reshape(Matrix memory m, uint rows, uint cols) internal pure returns (Matrix memory result) {
        require(m.cols * m.rows == rows * cols);
        result.rows = rows;
        result.cols = cols;
        result.data = m.data;
    }

    function takeRows(Matrix memory m, uint start, uint end) internal pure returns (Matrix memory result) {
        require(start < end && start < m.rows && end <= m.rows);
        result.rows = end - start;
        result.cols = m.cols;
        result.data = new bytes16[](result.rows * result.cols);
        for(uint i = 0; i < result.rows; i++) {
            for(uint j = 0; j < result.cols; j++) {
                setValue(result, i, j, getValue(m, i + start, j));
            }
        }
    }

    function takeCols(Matrix memory m, uint start, uint end) internal pure returns (Matrix memory) {
        return transpose(takeRows(transpose(m), start, end));
    }

    function eye(uint dim) internal pure returns (Matrix memory result) {
        result.rows = dim;
        result.cols = dim;
        result.data = new bytes16[](result.rows * result.cols);
        bytes16 zero = F.fromUInt(0);
        bytes16 one = F.fromUInt(1);
        for(uint i = 0; i < result.rows; i++) {
            for(uint j = 0; j < result.cols; j++) {
                if(i == j) {
                    setValue(result, i, j, one);
                } else {
                    setValue(result, i, j, zero);
                }
            }
        }
    }

    function normalize(M.Matrix memory m) internal pure returns (M.Matrix memory) {
        bytes16 nbElements = F.fromUInt(m.data.length);
        bytes16 totalMean = elementSum(m).div(nbElements);
        bytes16 totalStd = F.sqrt(elementSum(elementSquare(minus(m, totalMean))).div(nbElements));
        return totalStd == 0 ? minus(m, totalMean) : divide(minus(m, totalMean), totalStd);
    }

    function getValue(Matrix memory m, uint row, uint col) internal pure returns (bytes16) {
        return m.data[row * m.cols + col]; 
    }

    function setValue(Matrix memory m, uint row, uint col, bytes16 value) internal pure {
        m.data[row * m.cols + col] = value; 
    }
}