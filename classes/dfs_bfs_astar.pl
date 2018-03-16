% 2.1
:-use_module(library(lists)).

%estado inicial
estado_inicial(b(0,0)).

%estado final
estado_final(b(2,0)).

%transições entre estados
sucessor(b(X,Y), b(4,Y)) :- X<4.
sucessor(b(X,Y), b(X,3)) :- Y<3.
sucessor(b(X,Y), b(0,Y)) :- X>0.
sucessor(b(X,Y), b(X,0)) :- Y>0.
sucessor(b(X,Y), b(4,Y1)) :-
            X+Y>=4,
            X<4,
            Y1 is Y-(4-X).
sucessor(b(X,Y), b(X1,3)) :-
            X+Y>=3,
            Y<3,
            X1 is X-(3-Y).
sucessor(b(X,Y), b(X1,0)) :-
            X+Y<4,
            Y>0,
            X1 is X+Y.
sucessor(b(X,Y), b(0,Y1)) :-
            X+Y<3,
            X>0,
            Y1 is X+Y.

dfs(E,_, [E]):- estado_final(E).

dfs(E,V, [E|R]):-
        sucessor(E,E2),
        \+ member(E2,V),
        dfs(E2,[E2|V],R).

solve_dfs(S):-
    estado_inicial(Ei),
    dfs(Ei,[Ei], S).

count_sol(N):-
    findall(X,solve_dfs(X), L),
    length(L,N).

bfs([[E|Path]|_], [E|Path]) :- estado_final(E).

bfs([[E|Path]|R], S):-
        findall([E2|[E|Path]], sucessor(E,E2), LS),
        append(R,LS, L2),
        bfs(L2, S).


solve_bfs(S):-
    estado_inicial(Ei),
    bfs([[Ei]], L),
    reverse(L,S).


% funcao heuristica - minimizar agua transferida

sucessorC(b(X,Y), b(4,Y), C) :-
    X < 4,
    C is 4 - X.
sucessorC(b(X,Y), b(X,3), C) :-
    Y < 3,
    C is 3 - Y.
sucessorC(b(X,Y), b(0,Y), C) :-
    X > 0,
    C is X.
sucessorC(b(X,Y), b(X,0), C) :-
    Y > 0,
    C is Y.
sucessorC(b(X,Y), b(4,Y1), C) :-
    X + Y >= 4,
    X < 4,
    Y1 is Y - (4 - X),
    C is 4 - X.
sucessorC(b(X,Y), b(X1,3), C) :-
    X + Y >= 3,
    Y < 3,
    X1 is X - (3 - Y),
    C is 3 - Y.
sucessorC(b(X,Y), b(X1,0), C) :-
    X + Y < 4,
    Y > 0,
    X1 is X + Y,
    C is Y.
sucessorC(b(X,Y), b(0,Y1), C) :-
    X + Y < 3,
    X > 0,
    Y1 is X + Y,
    C is X.

heuristic(b(X,Y), H) :-
    estado_final(b(Xf, Yf)),
    H is max(abs(Xf - X), abs(Yf - Y)).

astar([(F, _G, [E|Path])|_R], (F, [E|Path])) :-
    estado_final(E).
astar([(_F, G, [E|Path])|R], S) :-
    findall((F2, G2, [E2|[E|Path]]), (sucessorC(E, E2, C), heuristic(E2, H2), G2 is G + C, F2 is G2 + H2), LS),
    append(R, LS, L2sort),
    sort(L2sort, L2),
    astar(L2, S).

solve_astar(S) :-
    estado_inicial(Ei),
    heuristic(Ei, Hi),
    astar([(Hi, 0, [Ei])], (F, RS)),
    reverse(RS, SS),
    S = (F, SS).
