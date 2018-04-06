:- use_module(library(lists)).

% 2.1

%estado inicial
estado_inicial(b(0,0)).

%estado final
estado_final(b(2,0)).

%transições entre estados
sucessor(b(X,Y), b(X,3)) :- Y<3.
sucessor(b(X,Y), b(4,Y)) :- X<4.
sucessor(b(X,Y), b(X,0)) :- Y>0.
sucessor(b(X,Y), b(0,Y)) :- X>0.
sucessor(b(X,Y), b(X1,3)) :-
			X+Y>=3,
			Y<3,
			X1 is X-(3-Y).
sucessor(b(X,Y), b(4,Y1)) :-
			X+Y>=4,
			X<4,
			Y1 is Y-(4-X).
sucessor(b(X,Y), b(0,Y1)) :-
			X+Y<3,
			X>0,
			Y1 is X+Y.
sucessor(b(X,Y), b(X1,0)) :-
			X+Y<4,
			Y>0,
			X1 is X+Y.
			
dfs(E,_, [E]) :- estado_final(E).

dfs(E,V,[E|R]) :- sucessor(E,E2),
                 \+ member(E2,V),
                 dfs(E2,[E2|V],R).

solve_dfs(S) :- estado_inicial(Ei),
                dfs(Ei,[Ei],S).
				
				
				
bfs([[E|Path]|_],[E|Path]) :- estado_final(E).

bfs([[E|Path]|R], S) :- findall([E2|[E|Path]],sucessor(E,E2),LS),
           append(R,LS,L2),
           bfs(L2,S).

solve_bfs(S) :- estado_inicial(Ei), 
                bfs([[Ei]],S).