import java.io.*;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicInteger;

public class Product implements Runnable{
    private BufferedReader br;
    private BufferedWriter bwProducts;
    private String orderId;
    private BufferedWriter bwOrders;
    private AtomicInteger productsToBeProcessed;
    private String orderOutputLine;

    public Product(String orderId, BufferedReader br, BufferedWriter bwProducts,
                   BufferedWriter bwOrders, AtomicInteger productsToBeProcessed, String orderOutputLine) {
        this.orderId = orderId;
        this.br = br;
        this.bwProducts = bwProducts;
        this.bwOrders = bwOrders;
        this.productsToBeProcessed = productsToBeProcessed;
        this.orderOutputLine = orderOutputLine;
    }

    public void run() {
        try {
            String line;
            while ((line = br.readLine()) != null) {
                String[] parts = line.split(",");
                if (Objects.equals(parts[0], orderId)) {
                    bwProducts.write(line + ",shipped" + "\n");

                    synchronized (productsToBeProcessed) {
                        productsToBeProcessed.getAndDecrement();
                        if (productsToBeProcessed.get() == 0) {
                            // write in orders_out.txt
                            bwOrders.write(orderOutputLine);
                        }
                    }
                    break;
                }
            }
        } catch (IOException e1) {
            e1.printStackTrace();
        }
    }
}
