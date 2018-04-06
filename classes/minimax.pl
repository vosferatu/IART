:- use_module(library(lists)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Formalização do jogo dos palitos:
% - existem inicialmente 10 palitos sobre a mesa
% - cada jogador pode retirar um, dois ou três palitos na sua vez
% - o objectivo é evitar ficar com o último palito
%%%%%%%%%%

% a representação do estado vai incluir também o jogador a jogar,
% pois neste jogo é importante para efeitos de avaliação do estado

% representação de um estado: (NumeroPalitos, Quemjoga)

estado_inicial((10,max)).

% estado final (ter 0 palitos é bom): só interessa para o minimax simples
estado_final((0,max), 1).
estado_final((0,min), 0).

%muito importante saber quem vai jogar a seguir pois as peças sao comuns aos jogadores

% transições entre estados (as jogadas são as mesmas para os 2 jogadores)
sucessor((N,max), max, (N1,min)) :- N>0, N1 is N-1.
sucessor((N,max), max, (N1,min)) :- N>1, N1 is N-2.
sucessor((N,max), max, (N1,min)) :- N>2, N1 is N-3.
sucessor((N,min), min, (N1,max)) :- N>0, N1 is N-1.
sucessor((N,min), min, (N1,max)) :- N>1, N1 is N-2.
sucessor((N,min), min, (N1,max)) :- N>2, N1 is N-3.


max_value([E],P1,Valor,E) :-
   minimax(E,min,P1,Valor,_).

max_value([E1|Es],P1, MV, ME) :-
  minimax(E1,min,P1,V1,_),
  max_value(Es,P1, V2, E2),
  (V1 > V2, !, MV=V1, ME=E1 ;
  MV=V2, ME=E2).

 % "=" para valores, "is" para expressoes

min_value([E],P1,Valor,E) :-
   minimax(E,max,Valor,_).

min_value([E1|Es], P1, MV, ME) :-
  minimax(E1,max,P1,V1,_),
  min_value(Es,P1,V2, E2),
  (V1 < V2, !, MV=V1, ME=E1 ;
  MV=V2, ME=E2).



% avaliação de estados

minimax(E,_,_,Valor,_) :- estado_final(E,Valor).

minimax(E,_,0,Valor,_) :- avalia(E,Valor).


minimax(E,max,P,Valor,Jogada) :-
  findall(E2, sucessor(E,max,E2),LS),
  P1 is P-1,
  max_value(LS,P1,Valor,Jogada).

minimax(E,min,P,Valor,Jogada) :-
  findall(E2, sucessor(E,min,E2),LS),
  P is P-1,
  min_value(LS,P1,Valor,Jogada).

avalia((N,max),V) :-
  1 is N mod 4, !, V=0;
  V=1.

avalia((N,min),V) :-
  1 is N mod 4, !, V=1;
  V=0.
