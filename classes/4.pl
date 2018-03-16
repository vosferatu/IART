sucessor( b(X,Y), B(4,Y), C) :- X < 4,
                                C is 4-X.
                                
sucessor( b(X,Y), b(4,Y1), C) :-


                                 C is 4-X.
                                 
h( b(X,Y), H) :- estado-final( b(Xf,Yf)),   //(sobrestima alguns casos)
                 H is name(abs(Xf-X)) + abs(Yf-Y).
         
astar([(F, G, [E|Path]) | R], [E|Path]) :- _estado_final(E).

astar([(F, G, [E|Path]) | R], S) :- findall( (F2, G2, [E2|[E|Path]]),
                                             ( successor(E,E2,C),
                                                h(E2,H2), G2 is G+C,
                                                F2 is G2+H2), LS),
                                                
                                    append(R,LS,L2),
                                    sort(L2,L2Ond),
                                    astar(L2Ond, S).
                                    
solve_astar(S) :- estado_inicial(Ei),
                  h(Ei,Hi),
                  astar([Hi,0,[Ei])], S).

/*--------------------------------------------------------------------------------*/

estado_inicial((10,max)).

estado_final((0,max),1).
estado_final((0,min),1).

successor((N,max),max,(N1,min)) :- ....

minimax(E, max, P, Valor, JOgada) :-
              findall(E2, sucessor(E,max,E2),LS),
              P1 is P-1,
              max_value(LS, P1, Valor, Jogada).
              
minimax(E, _, _, Valor, _) :- estado_final(E, Valor).

minimax(E, _, 0, Valor, _):- avalia(E, Valor).

max_value([E], P, Valor, E) :- minimax(E, P, min, Valor,_).

max_value([E1|Es], P, MV, ME) :-
              minimax(E1, min, P, V1, _),
              max_value(Es, P, V2, E2),
              (V1>V2, !, MV=V1, ME=E1;
               MV=V2, ME=E2).
      
min_value(...).      

avalia( (N,max), V) :-
        1 is N mod 4,!, V=0;
        V=1.
