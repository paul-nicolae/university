import logging
from logging.handlers import RotatingFileHandler
import time
from threading import RLock
import unittest


class Marketplace:
    def __init__(self, queue_size_per_producer):
        self.logger = logging.getLogger('logger_marketplace')
        self.logger.setLevel(logging.INFO)
        self.handler = RotatingFileHandler(
            "marketplace.log", maxBytes=1024 * 512, backupCount=5)
        self.formatter = logging.Formatter(
            '%(asctime)s, %(levelname)s, %(message)s')
        self.handler.setFormatter(self.formatter)
        self.formatter.converter = time.gmtime
        self.logger.addHandler(self.handler)

        self.queue_size_per_producer = queue_size_per_producer

        self.id_producer = 0
        # distribuitor = { 'producer_id' -> { 'products_lock' -> [products] } }
        self.distribuitor = {}

        self.id_cart = 0
        # cart = { 'cart_id' -> [(producer_id, product)] }
        self.cart = {}

        self.register_prod_lock = RLock()
        self.register_cart = RLock()
        self.verify_queue_size_lock = RLock()

    def register_producer(self):
        self.logger.info("we are registering a new producer")

        # avoiding race condition over multiple producers
        with self.register_prod_lock:
            self.id_producer += 1
            self.distribuitor[self.id_producer] = {}

            products_lock = RLock()

            # for every producer, every list of products should have its own
            # lock in order to prevent race condition
            self.distribuitor[self.id_producer][products_lock] = []

            self.logger.info("a producer has been registered")

            return self.id_producer

    def publish(self, producer_id, product):
        self.logger.info("%d is publishing the product %s",
                         producer_id, product)

        # avoiding race condition when verifing queue isn't full
        with self.verify_queue_size_lock:
            for lock, products in self.distribuitor[producer_id].items():
                if len(products) >= self.queue_size_per_producer:
                    self.logger.info(
                        "queue is full, producer %d should wait", producer_id)
                    return False

                # append is a thread safe method
                self.distribuitor[producer_id][lock].append(product)

        self.logger.info(
            "producer %d has been published the product %s", producer_id, product)

        return True

    def new_cart(self):
        self.logger.info("we are creating a new cart")

        # avoiding race condition over multiple consumers
        with self.register_cart:
            self.id_cart += 1
            self.cart[self.id_cart] = []

            self.logger.info("a cart has been created")

            return self.id_cart

    def add_to_cart(self, cart_id, product):
        self.logger.info(
            "consumer is trying to add the product %s into cart %d", product, cart_id)

        for producer_id, dict in self.distribuitor.items():
            for lock, products in dict.items():
                if product in products:
                    # remove the desired product from the marketplace
                    # for lists, remove method isn't thread safe
                    with lock:
                        self.distribuitor[producer_id][lock].remove(product)

                    # add product to cart
                    self.cart[cart_id].append((producer_id, product))

                    self.logger.info(
                        "the product %s has been added to cart %d", product, cart_id)

                    return True

        self.logger.error(
            "product %s not available, the consumer should wait", product)
        return False

    def remove_from_cart(self, cart_id, product):
        self.logger.info(
            "consumer is trying to remove the product %s from cart %d", product, cart_id)

        # looking in cart for the product that must be removed
        for (producer_id, prod) in self.cart[cart_id]:
            if str(prod) == str(product):
                # remove the product from the cart
                self.cart[cart_id].remove((producer_id, prod))

                # add the product back in the marketplace
                for lock, products in self.distribuitor[producer_id].items():
                    # for lists, append method is thread safe
                    self.distribuitor[producer_id][lock].append(prod)

                self.logger.info(
                    "the product %s has been remove from card %d", product, cart_id)

                return True

        # the product doesn't exists in the cart
        self.logger.error(
            "the product %s doesn't exist in card %d", product, cart_id)
        return False

    def place_order(self, cart_id):
        self.logger.info("consumer is placing the order from card %d", cart_id)

        products = []

        for (producer_id, product) in self.cart[cart_id]:
            products.append(product)

        self.logger.info("the order from cart %d has been placed", cart_id)

        return products

# new class for testing Marketplace methods


class TestMarketplace(unittest.TestCase):
    def setUp(self):
        self.marketplace = Marketplace(5)

    def test_register_producer(self):
        producer1 = self.marketplace.register_producer()
        self.assertEqual(producer1, 1)

        producer2 = self.marketplace.register_producer()
        self.assertNotEqual(producer2, 100)

        producer3 = self.marketplace.register_producer()
        self.assertEqual(producer3, 3)

    def test_publish(self):
        producer_id = self.marketplace.register_producer()
        products = ["Coffee(name='ethiopia', price=500, acidity=30, roast_level='medium')",
                    "Coffee(name='indonezia', price=1, acidity=1, roast_level='medium')",
                    "Coffee(name='ethiopia', price=10, acidity=2, roast_level='medium')",
                    "Tea(name='musetel', price=2, type='romanesc')",
                    "Tea(name='menta', price=3, type='clasic')",
                    "Tea(name='english breakfast', price=4, type='black')"]

        self.assertTrue(self.marketplace.publish(producer_id, products[0]))
        self.assertTrue(self.marketplace.publish(producer_id, products[1]))
        self.assertTrue(self.marketplace.publish(producer_id, products[2]))
        self.assertTrue(self.marketplace.publish(producer_id, products[3]))
        self.assertTrue(self.marketplace.publish(producer_id, products[4]))
        self.assertFalse(self.marketplace.publish(producer_id, products[5]))

    def test_new_cart(self):
        cart1 = self.marketplace.new_cart()
        self.assertEqual(cart1, 1)

        cart2 = self.marketplace.new_cart()
        self.assertNotEqual(cart2, 173)

    def test_add_to_cart(self):
        producer1_id = self.marketplace.register_producer()
        producer2_id = self.marketplace.register_producer()

        products = ["Tea(name='musetel', price=2, type='romanesc')",
                    "Tea(name='menta', price=3, type='clasic')",
                    "Coffee(name='ethiopia', price=500, acidity=30, roast_level='medium')"]

        self.assertTrue(self.marketplace.publish(producer1_id, products[0]))
        self.assertTrue(self.marketplace.publish(producer1_id, products[1]))

        self.assertTrue(self.marketplace.publish(producer2_id, products[2]))

        cart_id = self.marketplace.new_cart()
        self.assertTrue(self.marketplace.add_to_cart(cart_id, products[0]))
        self.assertFalse(self.marketplace.add_to_cart(cart_id, products[0]))
        self.assertTrue(self.marketplace.add_to_cart(cart_id, products[2]))

    def test_remove_from_cart(self):
        producer1_id = self.marketplace.register_producer()
        producer2_id = self.marketplace.register_producer()

        products = ["Tea(name='musetel', price=2, type='romanesc')",
                    "Tea(name='menta', price=3, type='clasic')",
                    "Coffee(name='ethiopia', price=500, acidity=30, roast_level='medium')"]

        self.assertTrue(self.marketplace.publish(producer1_id, products[0]))
        self.assertTrue(self.marketplace.publish(producer1_id, products[1]))

        self.assertTrue(self.marketplace.publish(producer2_id, products[2]))

        cart_id = self.marketplace.new_cart()
        self.assertTrue(self.marketplace.add_to_cart(cart_id, products[0]))
        self.assertTrue(self.marketplace.add_to_cart(cart_id, products[2]))

        self.assertTrue(
            self.marketplace.remove_from_cart(cart_id, products[0]))
        self.assertFalse(
            self.marketplace.remove_from_cart(cart_id, products[0]))
        self.assertFalse(
            self.marketplace.remove_from_cart(cart_id, products[1]))
