import java.io.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class Order implements Runnable {
    private BufferedReader brOrders;
    private BufferedWriter bwOrders;
    private BufferedWriter bwProducts;
    private File inputFileProducts;
    private ExecutorService pool;

    public Order(BufferedReader brOrders, BufferedWriter bwOrders, BufferedWriter bwProducts,
                 File inputFileProducts, ExecutorService pool) {
        this.brOrders = brOrders;
        this.bwOrders = bwOrders;
        this.bwProducts = bwProducts;
        this.inputFileProducts = inputFileProducts;
        this.pool = pool;
    }

    public void run() {
        try {
            String line;
            while ((line = brOrders.readLine()) != null) {
                String[] parts = line.split(",");
                Integer nrProducts = Integer.valueOf(parts[1]);

                String outputLine = line + ",shipped" + "\n";

                if (nrProducts != 0) {
                    BufferedReader brProducts = new BufferedReader(new FileReader(inputFileProducts));

                    AtomicInteger productsToBeProcessed = new AtomicInteger(nrProducts);

                    // start product threads
                    for (int i = 0; i < nrProducts; i++) {
                        Product task = new Product(parts[0], brProducts, bwProducts, bwOrders,
                                                    productsToBeProcessed, outputLine);
                        Thread t = new Thread(task);
                        pool.submit(t);
                    }
                }
            }
        } catch (IOException e1) {
            e1.printStackTrace();
        }
    }
}