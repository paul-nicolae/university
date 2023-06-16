from threading import Thread, current_thread
from time import sleep

class Consumer(Thread):
    def __init__(self, carts, marketplace, retry_wait_time, **kwargs):
        Thread.__init__(self, **kwargs)
        self.carts = carts
        self.marketplace = marketplace
        self.retry_wait_time = retry_wait_time

    def run(self):
        for cart in self.carts:
            cart_id = self.marketplace.new_cart()

            for ops in cart:
                processed_products = 0
                while processed_products < ops['quantity']:
                    if ops['type'] == 'add':
                        if self.marketplace.add_to_cart(cart_id, ops['product']):
                            processed_products += 1
                        else:
                            sleep(self.retry_wait_time)
                    else:
                        self.marketplace.remove_from_cart(cart_id, ops['product'])
                        processed_products += 1

            products = self.marketplace.place_order(cart_id)
            for product in products:
                print(current_thread().name + " bought " + str(product))
        
