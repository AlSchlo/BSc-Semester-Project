package benchmarking.neural_network;

import java.io.*;

/**
 * Utility class to load MNIST data
 */
public final class MNISTLoader {

    private final static int NB_CLASSES = 10;

    public static double[][] readImages(String filePath, int divideFactor) {
        try (var dataInputStream = new DataInputStream(new BufferedInputStream(new FileInputStream(filePath)))) {

            dataInputStream.readInt(); // Magic value
            var nbSamples = dataInputStream.readInt() / divideFactor;
            var nbRows = dataInputStream.readInt();
            var nbColumns = dataInputStream.readInt();
            double[][] data = new double[nbSamples][nbRows * nbColumns];

            for (int i = 0; i < nbSamples; i++) {
                for (int r = 0; r < nbRows; r++) {
                    for (int c = 0; c < nbColumns; c++) {
                        data[i][(r * nbColumns) + c] = dataInputStream.readUnsignedByte();
                    }
                }
            }

            return data;
        } catch (IOException e) {
            e.printStackTrace();
            throw new UncheckedIOException(e);
        }
    }

    public static double[][] readLabels(String filePath, int divideFactor) {
        try (var dataInputStream = new DataInputStream(new BufferedInputStream(new FileInputStream(filePath)))) {

            dataInputStream.readInt(); // Magic value
            var nbSamples = dataInputStream.readInt() / divideFactor;
            double[][] data = new double[nbSamples][NB_CLASSES];

            for (int i = 0; i < nbSamples; i++) {
               var label= dataInputStream.readUnsignedByte();
               for(int c = 0; c < NB_CLASSES; c++) {
                   if(c == label) {
                       data[i][c] = 1;
                   } else data[i][c] = 0;
               }
            }

            return data;
        } catch (IOException e) {
            e.printStackTrace();
            throw new UncheckedIOException(e);
        }
    }
}