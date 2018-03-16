sucessor( b(X,Y), B(4,Y), C) :- X < 4,
                                C is 4-X.
                                
sucessor( b(X,Y), b(4,Y1), c) :-


                                 C is 4-X.
                                 
h( b(X,Y), H) :- estado-final( b(Xf,Yf)),   //(sobrestima alguns casos)
                 H is abs(Xf-X) + abs(Yf-Y).
                 
