

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
