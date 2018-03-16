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

% avaliação de estados
minimax(E,max,Valor,Jogada) :-
  findall(E2, sucessor(E,max,E2),LS),
  max_value(LS,Valor,Jogada).
  
  
