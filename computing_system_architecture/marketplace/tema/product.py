from dataclasses import dataclass


@dataclass(init=True, repr=True, order=False, frozen=True)
class Product:
    name: str
    price: int


@dataclass(init=True, repr=True, order=False, frozen=True)
class Tea(Product):
    type: str


@dataclass(init=True, repr=True, order=False, frozen=True)
class Coffee(Product):
    acidity: str
    roast_level: str
