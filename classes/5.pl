:-use_module(library(lists)).
:-use_module(library(between)).


slots(4).
disciplinas(12).
disciplina(1,[1,2,3,4,5]). % Os alunos 1,2,3,4,5 estão inscritos à disciplina 1
disciplina(2,[6,7,8,9]).
disciplina(3,[10,11,12]).
disciplina(4,[1,2,3,4]).
disciplina(5,[5,6,7,8]).
disciplina(6,[9,10,11,12]).
disciplina(7,[1,2,3,5]).
disciplina(8,[6,7,8]).
disciplina(9,[4,9,10,11,12]).
disciplina(10,[1,2,4,5]).
disciplina(11,[3,6,7,8]).
disciplina(12,[9,10,11,12]).

incompat(D1,D2,NA) :-
  disciplina(D1,LA1),
  disciplina(D2,LA2),
  findall(A,(member(A,LA1), member(A,LA2)),LA12),
  length(LA12,NA).
 
 %  d1 d2 d3
 % [1, 2, 3, 3,..] = L
 
 f_aval(L,V) :-
 findall(N,
		(nth1(D1,L,Slot),
		nth1(D2,L,Slot),
		D1 < D2, %antes, D1 \= D2, < para eliminar simetrias
		incompat(D1,D2,N)),  
		LIncomp),
		sumlist(LIncomp,V).
		
best(X,Y) :- X<Y.

	
neighbor(L,L2) :-
	slots(NSlots),
	nth1(D,L,Slot),
	between(1,NSlots,NovoSlot),
	NovoSlot \= Slot,
	D1 is D-1, length(Prefix,D1),
	append(Prefix,[Slot|Suffix],L),
	append(Prefix,[NovoSlot|Suffix],L2).
		
		
%parte de uma soluçao do problema e transita-se sempre que possivel a uma soluçao vizinha (soluçao a partir da atual a partir de um modificador) e assume-se essa soluçao caso seja melhor
%quando isso deixar de ser possivel (todos os vizinhos forem piores que a soluçao atual) paro. min/max local 		
hillclimbing(S, LocalOpt) :-
	f_aval(S,V1),
	neighbor(S,S2),
	f_aval(S2,V2),
	best(V2,V1),
	!, write(S2:V2),nl, %compromisso: se num espaço de estados se atingir otimo local, nao voltar para tras
	hillclimbing(S2,LocalOpt).
	
hillclimbing(S,S).

	

	
  
