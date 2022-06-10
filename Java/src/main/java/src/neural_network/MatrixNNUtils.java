package src.neural_network;

import org.ejml.simple.SimpleMatrix;

import java.util.Random;

import static java.lang.Math.*;

public final class MatrixNNUtils {

    // Utility class
    private MatrixNNUtils() {}

    private static SimpleMatrix relu(SimpleMatrix m) {
        assert(m.isVector());
        var nbElements = m.getNumElements();
        double [] output = new double[m.getNumElements()];
        for(int i = 0; i < nbElements; i++) {
            output[i] = max(m.get(i), 0);
        }
        return new SimpleMatrix(nbElements, 1, true, output);
    }

    private static SimpleMatrix identity(SimpleMatrix m) {
        assert(m.isVector());
        return m;
    }

    private static SimpleMatrix dIdentityDx(SimpleMatrix x) {
        assert(x.isVector());
        return SimpleMatrix.identity(x.getNumElements());
    }

    private static SimpleMatrix dL2Dx(SimpleMatrix x, SimpleMatrix y) {
        assert(x.isVector() && y.isVector());
        var derivative = x.minus(y).scale(2);
        derivative.reshape(1, x.getNumElements());
        return derivative;
    }

    private static SimpleMatrix affine(SimpleMatrix x, SimpleMatrix W) {
        assert(x.isVector());
        double[][] one = {{ 1.0 }};
        var nbElements = x.getNumElements();
        var extendedX = new SimpleMatrix(nbElements + 1, 1);
        extendedX.insertIntoThis(0, 0, x);
        extendedX.insertIntoThis(nbElements, 0, new SimpleMatrix(one));
        return W.mult(extendedX);
    }

    private static SimpleMatrix initMatrix(int rows, int columns) {
        var init = new SimpleMatrix(rows, columns);
        var rand = new Random(0);
        var l = sqrt(3.0 / (double) columns);
        for(int i = 0; i < rows * columns; i++) {
            init.set(i, rand.nextDouble(-l, l));
        }
        return init;
    }

    private static SimpleMatrix [] initNetwork(int[] dimensions) {
        var nbDimensions = dimensions.length;
        assert(nbDimensions > 1);
        SimpleMatrix [] matrices = new SimpleMatrix[nbDimensions - 1];
        for(int i = 0; i < nbDimensions - 1; i++) {
            var Wi = dimensions[i];
            var Wip1 = dimensions[i + 1];
            matrices[i] = initMatrix(Wip1, Wi).concatColumns(new SimpleMatrix(Wip1, 1));
        }
        return matrices;
    }

    private static SimpleMatrix dReluDx(SimpleMatrix x) {
        assert(x.isVector());
        var nbElements = x.getNumElements();
        var derivative = new SimpleMatrix(nbElements, nbElements);
        for(int i = 0; i < nbElements; i++) {
            derivative.set(i, i, x.get(i) > 0 ? 1 : 0);
        }
        return derivative;
    }

    private static SimpleMatrix dAffineDx(SimpleMatrix W) {
        return W.extractMatrix(0, W.numRows(), 0, W.numCols() - 1);
    }

    private static SimpleMatrix dAffineDw(SimpleMatrix x, SimpleMatrix W) {
        assert(x.isVector());
        var nbRows = W.numRows();
        var nbElements = x.getNumElements();
        var derivative = new SimpleMatrix(nbRows, nbRows * (nbElements + 1));
        for(int i = 0; i < nbRows; i++) {
            for(int j = 0; j < nbElements; j++) {
                derivative.set(i, i * (nbElements + 1) + j, x.get(j));
            }
            derivative.set(i, i * (nbElements + 1) + nbElements , 1);
        }
        return derivative;
    }

    private static SimpleMatrix[] backpropagation(SimpleMatrix x, SimpleMatrix y, SimpleMatrix [] allW) {
        assert(x.isVector() && y.isVector());
        var nbLayers = allW.length;
        SimpleMatrix [] inputs = new SimpleMatrix[nbLayers];
        SimpleMatrix [] combinations = new SimpleMatrix[nbLayers];
        // Forward propagation
        for(int i = 0; i < nbLayers; i++) {
            inputs[i] = x;
            var comb = affine(x, allW[i]);
            combinations[i] = comb;
            x = i == nbLayers - 1 ? identity(comb) : relu(comb);
        }
        var currJac = dL2Dx(x, y);
        // Backpropagation
        SimpleMatrix [] jacs = new SimpleMatrix[nbLayers];
        for(int i = nbLayers - 1; i >= 0; i--) {
            var comb = combinations[i];
            var W = allW[i];
            currJac = i == nbLayers - 1 ? currJac.mult(dIdentityDx(comb)) : currJac.mult(dReluDx(comb));
            var Jw = currJac.mult(dAffineDw(inputs[i], W));
            Jw.reshape(W.numRows(), W.numCols());
            jacs[i] = Jw;
            currJac = currJac.mult(dAffineDx(W));
        }
        return jacs;
    }

    public static SimpleMatrix [] train(SimpleMatrix trainInput, SimpleMatrix trainOutput, int [] dimensions,
                                         int numEpoch, double learningRate) {
        SimpleMatrix [] allW = initNetwork(dimensions);
        int nbSamples = trainInput.numRows();
        // Loop over all epochs
        for(int i = 0; i < numEpoch; i++) {
            // Loop over all samples
            for(int r = 0; r < nbSamples; r++) {
                SimpleMatrix [] jacs = backpropagation(trainInput.rows(r, r + 1).transpose(),
                                                        trainOutput.rows(r, r + 1).transpose(), allW);
                // Update weights
                for(int j = 0; j < allW.length; j++) {
                    var oldW = allW[j];
                    var jac = jacs[j];
                    allW[j] = oldW.minus(jac.scale(learningRate));
                }
            }
        }
        return allW;
    }

    public static int argMax(SimpleMatrix v) {
        assert(v.isVector());
        int nbElements = v.getNumElements();
        int maxIdx = -1;
        double max = Double.MIN_VALUE;
        for(int i = 0; i < nbElements; i++) {
            var val = v.get(i);
            maxIdx = val >= max ? i : maxIdx;
            max = Math.max(val, max);
        }
        return maxIdx;
    }

    public static SimpleMatrix normalize(SimpleMatrix m) {
        var nbElements = m.getNumElements();
        var totalMean = m.elementSum() / nbElements;
        var totalStd = sqrt(m.minus(totalMean).elementPower(2).elementSum() / nbElements);
        return totalStd == 0 ? m.minus(totalMean) : m.minus(totalMean).divide(totalStd);
    }

    public static SimpleMatrix nn(SimpleMatrix x, SimpleMatrix [] allW) {
        assert(x.isVector());
        for(SimpleMatrix simpleMatrix : allW) {
            x = relu(affine(x, simpleMatrix));
        }
        return identity(x);
    }
}