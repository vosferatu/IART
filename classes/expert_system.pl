% 7.1

% a)

:- op(800, xfy, e).
:- op(810, xfy, ou).
:- op(950, xfx, entao).
:- op(850, fx, se).
:- op(750, xfx, com).

% R1
se motor=nao e bateria=ma entao problema=bateria com fc=1.
% R2
se luz=fraca entao bateria=ma com fc=0.8.
% R3
se radio=fraco entao bateria=ma com fc=0.8.
% R4
se luz=boa e radio=bom entao bateria=boa com fc=0.8.
% R5
se motor=sim e cheiro_gas=sim entao problema=encharcado com fc=0.8.
% R6
se motor=nao e bateria=boa e indicador_gas=vazio entao problema=sem_gasolina com fc=0.9.
% R7
se motor=nao e bateria=boa e indicador_gas=baixo entao problema=sem_gasolina com fc=0.3.
% R8
se motor=nao e cheiro_gas=nao e ruido_motor=nao_ritmado e bateria=boa entao problema=motor_gripado com fc=0.7.
% R9
se motor=nao e cheiro_gas=nao e bateria=boa entao problema=carburador_entupido com fc=0.9.
% R10
se motor=nao e bateria=boa entao problema=velas_estragadas com fc=0.8.

% b)

:- dynamic
    fact/3.
% fact(A, V, FC).

% c)

questionable(motor, 'O motor funciona?', [sim,nao]).
questionable(luz, 'Como estao as luzes?', [fraca,razoavel,boa]).
questionable(radio, 'Como esta o radio?', [fraco,razoavel,bom]).
questionable(cheiro_gas, 'Sente cheiro a gasolina?', [sim,nao]).
questionable(indicador_gas, 'Como esta o indicador de gasolina?', [vazio,baixo,meio,cheio]).
questionable(ruido_motor, 'Que ruido faz o motor?', [ritmado,nao_ritmado]).

start :-
    retractall(fact(_, _, _)),
    check(problema, V, FC),
    write('Problem = '), write(V), nl, write('Certainty: '), write(FC).

check(A, V, FC) :-
    fact(A, V, FC),
    !.
check(A, V, _) :-
    fact(A, NV, _),
    V \= NV,
    !,
    fail.
check(A, V, FC) :-
    questionable(A, Question, ListAnswers),
    repeat,
    write(Question), write(' '), write(ListAnswers), nl,
    read(Answer),
    member(Answer, ListAnswers),
    repeat,
    write('Certainty? [0-1]'), nl,
    read(FC),
    FC =< 1, FC >= 0,
    assertz(fact(A, Answer, FC)),
    !,
    V = Answer.
check(A, V, FC) :-
    deduction(A, V, FC).

deduction(A, V, _) :-
    se Proposition entao A = V com fc=FCRule,
    prove(Proposition, FCProposition),
    FCConclusion is FCProposition * FCRule,
    update(A, V, FCConclusion),
    fail.
deduction(A, V, FC):-
    fact(A, V, FC).

prove(A=V, FC) :-
    check(A, V, FC).
prove(A=V e Proposition, FC) :-
    check(A, V, FC1),
    prove(Proposition, FC2),
    FC is min(FC1, FC2).
prove(A=V ou Proposition, FC) :-
    check(A, V, FC1),
    prove(Proposition, FC2),
    FC is max(FC1, FC2).

update(A, V, FC1) :-
    (retract(fact(A, V, FC2)),
    !,
    FC is FC1 + FC2 * (1 - FC1);
    FC is FC1),
    assertz(fact(A, V, FC)).
