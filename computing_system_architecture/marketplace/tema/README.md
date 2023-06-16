Nicolae Paul
333CB

    Tema prezinta o abordare Multi Producer - Multi Consumer pentru un Marketplace
de ceaiuri si cafele.
    Avem doua clase copil (Producer si Consumer) mostenite din clasa parinte
Thread care consuma in mod concurent date din Marketplace.

    In subrutina Producerului publicam in mod constant produsele din lista de
produse primata ca parametru, avand dupa fiecare actiune un timp de asteptat.
    Cu ajutorul variabilei contor published_products respectam publicarea unui
produs de un numar maxim de ori corespunzator cu cantitatea primita ca parametru.
Dupa fiecare publicare reusita, asteptam un timp corespunzator fiecarui produs.
    Daca coada producatorului s-a umplut, sau daca un produs a fost deja
publicat de un numar maxim de ori si se incerca o alta pubicare a acestuia,
se asteapta republish_wait_time pana la o viitoare incercare de publicare.

    In subrutina Consumer, avem un comportament asemanator ca la Producer, dar
adaptat pe structura de date carts primita ca parametru.
    Pentru fiecare cart din carts, vom crea un cart nou, iar pentru fiecare
operatie corespunzatoare din cartul curent, apelam metoda potrivita din
Marketplace, consumand astfel produse de acolo.
    La fiecare operatie in parte vom contoriza numarul de produse procesate
(add or remove) pentru a respecta campul quantity al cartului.
    Daca incercam sa adaugam in cart un produs care nu exista in Marketplace,
vom astepta retry_wait_time pana cand vom reincerca sa luam acel produs din
stocul disponibil.
    Pentru fiecare cos in parte, vom face si printarea produselor din acesta
la apelul metodei place_order din Marketplace.

    In clasa Marketplace avem toata logica concurenta a aplicatiei, unde sunt
scrise toate structurile de date si metodele folosite de threaduri.
    Ca si campuri ale clasei, vom folosi urmatoarele:

    - id_producer, pt contorizarea producatorilor
    - id_cart, pt contorizarea carucioarelor
    - distribuitor, un dictionar unde sunt stocate produsele publicate de
producator, care sunt si available. Cheia este idul unui producator, iar valoarea
este un alt dictionar, care are ca si cheie un lock si ca si valoare lista de
produse publicare de producatorul cu idul retinut ulterior. Folosim un lock
specific pentru fiecare lista de produse ale unui producator pentru a evita
race condition in cazul in care mai multi consumatori vor sa cumpere produsele
aceluiasi producator.
    - cart, unde stocam produsele dintr-un carucior. Este un dictionar unde cheia
este idul caruciorului, iar valoarea este lista produselor din carucior. Pentru
fiecare produs din cos, preferam sa retinem si idul producatorului care a publicat
acel produs, pentru usurinta adaptarii structurii distribuitor la fiecare operatie
de add si remove facuta de consumator.
    - register_prod_lock, pt evitarea race conditionului la fiecare noua
inregistrarea a unui producator
    - register_cart, pt evitarea race conditionului la fiecare noua inregistrarea
a unui consumator
    - verify_queue_size_lock, pt evitarea race conditionului in momentul in care
verificam daca coada producatorului curent este plina

    Odata clare structurile folosite in toate metodele din Marketplace, voi
explica pe scurt ce face fiecare metoda:

    - register_producer = inregistreaza initializarea fiecarui nou producator,
initializand in acelasi timp si dictionarul distribuitor unde o sa fie publicate
toate produsele available pentru consumatori

    - publish = daca coada producatorului curent este plina, returneaza False.
Astfel, publica produsul primit ca parametru adaugandul in coada (adaugandul
in lista de produse corespunzatoare idului producatorului)

    - new_cart = inregistreaza initializarea fiecarui nou carucior, initializand
in acelasi timp si dictionarul cart unde o sa fie stocate produsele (si idul
producatorului pentru fiecare produs in parte) din cosul corespunzator cart_idului.

    - add_to_cart = cautam printe produsele disponibile produsul primit ca
parametru. In cazul in care este gasit, in scoate din distribuitor pentru a nu mai
fi availabe pentru alti consumatori, si il adaugam in cosul curent. In cazul in
care produsul nu este gasit, returnam False.

    - remove_from_cart = cautam in cos produsul care urmeaza sa fie eliminat.
Daca il gasim, il eliminam, urmand sa-l adaugam inapoi in distribuitor pentru
a fi disponibil pentru alti consumatori. Pe dictionarul cart nu este nevoie sa
folosim in niciun context lock deoarece fiecare consumator are cosuri unice si
din cosuri.
    Daca nu gasim in cos produsul primit ca parametru, returnam False.

    - place_order = returnam lista de produse din cosul corespunzator idului
primit ca paramentru.

    
    Intregul enunt al temei este implementat.
    
    Nu am avut dificultati in intelegerea si implementarea temei.

    Ce am invatat nou pe parcursul temei a fost lucrul cu logger. Foarte util
pentru partea de debugging.
