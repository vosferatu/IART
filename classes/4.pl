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
