                  Tema #3 Image processing with MPI
                        Nastasoiu Florina 335CA

                    Part 1. The filters

    Pentru prima parte am populat structura imagine cu: culoare, latime,
lungime, valoarea maxima si 4 matrici de unsigned char (una folosita in
cazul imaginilor alb-negru, celelate 3 folosite in cazul imaginilor rgb,
fiecare pentru o culoare).
    Functia readData() citeste randurile fisierului consecutiv, folosind 
fread() si populeaza fie prima matrice, fie celelalte trei, in functie
de valoarea P5/P6 de pe primul rand.
    Functia writeData() scrie in fisier fiecare rand din fiecare matrice.
Poate fi una singura(alb-negru) sau 3 matrice(RGB), caz in care se vor 
scrie intercalat, respectand ordinea initiala.
    Functia apply() aplica filtrul pe intreaga imagine. Se inmultesc ele-
ment cu element matricea imagine si matricea filtrului ales. Se proce-
deaza similar in cazul matricei color, pentru fiecare culoare.

                                                                                                                  
