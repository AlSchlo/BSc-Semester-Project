package benchmarking.neural_network;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.RunnerException;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import src.neural_network.ClassificationContract;

import java.util.concurrent.TimeUnit;

import static benchmarking.neural_network.MNISTLoader.readImages;
import static benchmarking.neural_network.MNISTLoader.readLabels;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Warmup(iterations = 1, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(2)
@State(Scope.Benchmark)
public class Simulation {

    // Benchmark and tests on the MNIST data set. Serves as proof of proper implementation.
    // However, the benchmarking is done on dummy data that can be easily generated across platforms
    // without having to import anything (also, the MNIST data set is ridiculously big to process on the EVM)
    /*@State(Scope.Benchmark)
    public enum MnistContract {
        TRAINING_100(new int[]{50}, false),
        PRE_TRAINED_100(new int[]{50}, true);

        public final ClassificationContract contract;

        private final static int DIVIDE_FACTOR = 10;
        public final static int NB_EPOCH = 2;
        public final static double LEARNING_RATE = 0.001;

        MnistContract(int[] dimensions, boolean trained) {
            this.contract = new ClassificationContract(
                    readImages("src/main/resources/MNIST/train-images.idx3-ubyte", DIVIDE_FACTOR),
                    readLabels("src/main/resources/MNIST/train-labels.idx1-ubyte", DIVIDE_FACTOR),
                    readImages("src/main/resources/MNIST/t10k-images.idx3-ubyte", DIVIDE_FACTOR),
                    readLabels("src/main/resources/MNIST/t10k-labels.idx1-ubyte", DIVIDE_FACTOR),
                    dimensions);
            this.contract.normalize();
            if (trained) {
                this.contract.train(NB_EPOCH, LEARNING_RATE);
            }
        }
    }

    @Benchmark
    public void mnistTrain(Blackhole bh) {
        var contract = MnistContract.TRAINING_100.contract;
        contract.train(MnistContract.NB_EPOCH, MnistContract.LEARNING_RATE);
        bh.consume(contract);
    }

    @Benchmark
    public void mnistVerify(Blackhole bh) {
        var contract = MnistContract.PRE_TRAINED_100.contract;
        contract.test();
        bh.consume(contract);
    }*/

    @State(Scope.Benchmark)
    public enum Contract {
        NORMALIZE(false, false),
        TRAIN(true, false),
        TEST(true, true);

        public final ClassificationContract contract;

        private final static int NB_TRAIN_SAMPLES = 100;
        private final static int NB_FEATURES = 5;
        private final static int NB_TEST_SAMPLES = 2;
        private final static int NB_CLASSES = 2;

        public final static int NB_EPOCH = 1;
        public final static double LEARNING_RATE = 0.01;

        private final static int NB_LAYERS = 1;
        private final static int LAYER_SIZE = 3;


        Contract(boolean normalized, boolean trained) {
            this.contract = new ClassificationContract(generateData(NB_TRAIN_SAMPLES, NB_FEATURES),
                    generateData(NB_TRAIN_SAMPLES, NB_CLASSES), generateData(NB_TEST_SAMPLES, NB_FEATURES),
                    generateData(NB_TEST_SAMPLES, NB_CLASSES), generateLayers());
            if (normalized) {
                this.contract.normalize();
                if (trained) {
                    this.contract.train(NB_EPOCH, LEARNING_RATE);
                }
            }
        }

        private double[][] generateData(int rows, int cols) {
            double[][] data = new double[rows][cols];
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    data[i][j] = ((i * cols) + j) % 1000;
                }
            }
            return data;
        }

        private int[] generateLayers() {
            int[] layers = new int[NB_LAYERS];
            for (int i = 0; i < NB_LAYERS; i++) {
                layers[i] = LAYER_SIZE;
            }
            return layers;
        }
    }

    @Benchmark
    public void normalize(Blackhole bh) {
        var contract = Contract.NORMALIZE.contract;
        contract.normalize();
        bh.consume(contract);
    }

    @Benchmark
    public void train(Blackhole bh) {
        var contract = Contract.TRAIN.contract;
        contract.train(Contract.NB_EPOCH, Contract.LEARNING_RATE);
        bh.consume(contract);
    }

    @Benchmark
    public void test(Blackhole bh) {
        var contract = Contract.TEST.contract;
        contract.test();
        bh.consume(contract);
    }

    public static void main(String[] args) throws RunnerException {

        Options opt = new OptionsBuilder()
                .include(Simulation.class.getSimpleName())
                .build();

        new Runner(opt).run();
    }
}
