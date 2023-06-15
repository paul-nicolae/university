Nicolae Paul
333CB
                             Tema 2 - APD

    In componenta proiectului avem 3 clase:
        - Tema2 -> clasa main unde citim si prelucram inputul si pornim si
    threadurile.
        - Order -> clasa threadului order care contine subrutina threadului care
    se ocupa de procesarea comenzilor si de pornirea threadurile de products.
        - Product -> clasa threadului product care contine subrutina threadului
    care se ocupa cu citirea si procesarea produselor.

    In Tema2.java, dupa ce iau calea directorului si numarul de threadsuri per
nivel din linia de comanda, voi instantia un BufferedReader si un BufferedWriter
pe care le voi folosi pe tot parcursul temei pentru procesarea (citirea si
scrierea) datelor din fisiere. Am ales sa folosesc BufferedReader si BufferedWriter
deoarece sunt synchronized, deci citirea si scrirea in fisiere o sa fie thread safe.
Toate orders threadurile o sa foloseasca neaparat acelasi BufferedReader si
BufferedWriter, deci le voi pasa in constructorul clasei Orders, ca fiecare
nou thread pornit sa foloseasca buffere comune.
    In acelasi timp, pasez fiecarui thread de order un thread pool pentru a ne
asigura ca in nivelul 2 de threaduri nu depasim nrThreads active.
    Pornesc nrThreads de order threads si la final fisierului Tema2.java ii zic
main threadului sa astepte executia tuturor orderurilor inainte sa inchida
BufferedWriterii.

    In Order.java avem subrutina threadului de order care se ocupa atat cu
citirea datelor din orders.txt si cu pornirea threadurile de Products.
    Folosim un atomicInteger pe care il initializam cu numarul de produse pe care
le are comanda de procesat pentru a ne asigura in logica threadului de Product
ca o comanda este shipped doar dupa procesarea tuturor produselor din comanda curenta.
    Cat timp mai avem comenzi de procesat, threadul curent va citi comanda si
va porni nrProducts de Products (numarul maxim de threaduri permis per nivel)
daca comanda curenta are produse de procesat. Fiecare nou thread Product
este adaugat in thread pool, acesta din urma asigurandu-ne executia concomitenta
a fix nrProducts threaduri.
    Fiecarui product thread curent ii pasez si idul comenzii din care face
parte ca se ne asiguram de acuratetea produselor scrise in order_products_out.txt.

    In Products.java avem subrutina threadului product care citeste produse din
order_products.txt si le scrie in order_products_out.txt doar daca produsul citit
corespunde idului comenzii primit in constructor. De fiecare data cand gasim un
produs cu un id al unei comenzi valid, dam break ca sa ne asiguram ca threadul
Product curent proceseaza doar un singur produs.
    Tot aici scriem si in orders_out daca o comanda a fost shipped dupa ce nu
mai ramane niciun produs de procesat.
