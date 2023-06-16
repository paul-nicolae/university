from threading import Thread
from time import sleep

class Producer(Thread):
    def __init__(self, products, marketplace, republish_wait_time, **kwargs):
        Thread.__init__(self, **kwargs)
        self.products = products
        self.marketplace = marketplace
        self.republish_wait_time = republish_wait_time

    def run(self):
        producer_id = self.marketplace.register_producer()
        
        while True:
            for product in self.products:
                published_products = 0
                while published_products < product[1]:
                    if self.marketplace.publish(producer_id, product[0]):
                        published_products += 1
                        sleep(product[2])
                    else:
                        sleep(self.republish_wait_time)