% 8

% 8.1

%%%
% gramática
%

determinante(s-m) --> [o].
determinante(p-m) --> [os].
determinante(s-f) --> [a].

preposicao(_) --> [de].
preposicao(s-f) --> [da].

nome(p-m, rapazes) --> [rapazes].
nome(s-m, rapaz) --> [rapaz].
nome(s-m, rui) --> [rui].
nome(s-m, luis) --> [luis].
nome(s-f, rita) --> [rita].
nome(s-f, ana) --> [ana].
nome(s-f, maria) --> [maria].
nome(s-m, elefante) --> [elefante].
nome(p-m, caes) --> [caes].
nome(p-m, gatos) --> [gatos].
nome(s-m, cao) --> [cao].
nome(s-m, gato) --> [gato].
nome(s-m, futebol) --> [futebol].
nome(p-m, morangos) --> [morangos].
nome(p-m, amendoins) --> [amendoins].
nome(p-m, bolachas) --> [bolachas].
nome(p-m, humanos) --> [humanos].
nome(p-f, pessoas) --> [pessoas].

verbo(s,jogar,s) --> [joga], {humano(s)}.
verbo(p,jogar) --> [jogam].
verbo(s,gostar) --> [gosta].
verbo(p,gostar) --> [gostam].
verbo(s,comer) --> [come].
verbo(p,comer) --> [comem].
verbo(p,ser) --> [sao].

%%%
% base de dados
%

humano(rapaz).
humano(rui).
humano(maria).
humano(rita).
humano(ana).
humano(luis).
humano(humano).
humano([]).
humano([H|T]) :- humano(H), humano(T).

jogar(rapaz, futebol).
jogar(rui, futebol).
jogar(pokemon, futebol).

gostar(luis, morango).
gostar(rita, morango).
gostar(ana, morango).
gostar(rui, maria).
gostar(cao, bolacha).
gostar(gato, bolacha).

comer(elefante, amendoim).

ser(rui, rapaz).
ser(X, humano) :- humano(X).


% 8.2

%%%
% gramática
%

pron_inter(_-_) --> [quem].
pron_inter(p-_) --> [quais].
pron_inter(p-m) --> [quantos].
pron_inter(p-f) --> [quantas].

pronome(_) --> [que].


frase(A,S,Ob) --> sn(N,S), sv(N,A,Ob,S).

sn(N,S) --> determinante(N-G ), nome(N-G,S).
sn(N,S) --> nome(N-_,S).

sv(N, gostar, Ob,S) --> verbo(N, gostar,S),{!}, preposicao(N1-G1), nome(N1-G1,Ob).
sv(N,A,Ob,S) --> verbo(N,A,S), sn(_, Ob).

concorda_frase(A,S,Ob) :-
  P =.. [A,S,Ob],
  (P,!,write(concorda);
  write(discorda)).
