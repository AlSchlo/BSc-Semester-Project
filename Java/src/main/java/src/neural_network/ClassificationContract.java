package src.neural_network;

import org.ejml.simple.SimpleMatrix;
import src.TxnRevertException;

import static src.neural_network.MatrixNNUtils.argMax;
import static src.neural_network.MatrixNNUtils.nn;


/**
 * A classification contract that uses a multi-layer perceptron neural network
 * Uses ReLu as intermediate activation functions
 * Training is done with stochastic gradient descent
 */
public final class ClassificationContract {

    private SimpleMatrix trainInput;
    private SimpleMatrix testInput;
    private final SimpleMatrix trainOutput;
    private final SimpleMatrix testOutput;
    private final int[] dimensions;

    private SimpleMatrix[] allW;

    /**
     * Creates a new contract given raw (non-normalized) input data and their associated
     * output probability for every class
     *
     * @param X The raw N samples of M dimensions matrix (N x M)
     * @param Y The raw N classification encoded as a probability between the C classes (N x C)
     * @param tX The raw N samples of T dimensions matrix (T x M)
     * @param tY The raw N classification encoded as a probability between the C classes (T x C)
     * @param hiddenLayers The dimensions (and number) of hidden layers
     */
    public ClassificationContract(double[][] X, double[][] Y, double[][] tX, double[][] tY, int[] hiddenLayers) {
        // Normalize the data using a whitening transformation
        this.trainInput = new SimpleMatrix(X);
        this.testInput = new SimpleMatrix(tX);
        this.trainOutput = new SimpleMatrix(Y);
        this.testOutput = new SimpleMatrix(tY);
        var nbHiddenLayers = hiddenLayers.length;
        int[] dimensions = new int[nbHiddenLayers + 2];
        dimensions[0] = X[0].length;
        System.arraycopy(hiddenLayers, 0, dimensions, 1, hiddenLayers.length);
        dimensions[nbHiddenLayers + 1] = Y[0].length;
        this.dimensions = dimensions;
        this.allW = null;
    }

    public void normalize() {
        this.trainInput = MatrixNNUtils.normalize(this.trainInput);
        this.testInput = MatrixNNUtils.normalize(this.testInput);
    }

    public void train(int numEpoch, double learningRate) {
        allW = MatrixNNUtils.train(trainInput, trainOutput, dimensions, numEpoch, learningRate);
    }

    public double test() {
        // Check if the model has been trained
        if(allW == null) throw new TxnRevertException();
        int nbSame = 0;
        int totalSamples = testInput.numRows();
        for(int i = 0; i < totalSamples; i++) {
            int prediction = argMax(nn(testInput.rows(i, i + 1).transpose(), allW));
            int expected = argMax(testOutput.rows(i, i + 1));
            if(prediction == expected) nbSame++;
        }
        return (double) nbSame / (double) totalSamples;
    }
}