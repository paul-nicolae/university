import java.io.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Tema2 {
    public static Integer nrThreads;

    public static void main(String[] args) {
        String directoryInput = args[0];
        nrThreads = Integer.valueOf(args[1]);

        ExecutorService threadPool = Executors.newFixedThreadPool(nrThreads);

        try {
            // input and output files for orders
            Path path = Paths.get(directoryInput, "orders.txt");
            File inputFileOrders = path.toFile();
            BufferedReader brOrders = new BufferedReader(new FileReader(inputFileOrders));

            FileWriter outputFileOrders = new FileWriter(new File(".", "orders_out.txt"));
            BufferedWriter bwOrders = new BufferedWriter(outputFileOrders);

            // output file for products
            path = Paths.get(directoryInput, "order_products.txt");
            File inputFileProducts = path.toFile();

            FileWriter outputFileProducts = new FileWriter(new File(".", "order_products_out.txt"));
            BufferedWriter bwProducts = new BufferedWriter(outputFileProducts);

            // create tasks
            List<Thread> threads = new ArrayList<>();

            // start order threads
            for (int i = 0; i < nrThreads; i++) {
                Order task = new Order(brOrders, bwOrders, bwProducts, inputFileProducts, threadPool);
                Thread t = new Thread(task);
                t.start();
                threads.add(t);
            }

            // wait for order threads to finish
            for (int i = 0; i < nrThreads; i++) {
                threads.get(i).join();
            }

            threadPool.shutdown();
            threadPool.awaitTermination(Integer.MAX_VALUE, TimeUnit.SECONDS);

            bwProducts.flush();
            bwProducts.close();
            bwOrders.flush();
            bwOrders.close();

        } catch (InterruptedException | IOException e1) {
            e1.printStackTrace();
        }
    }
}