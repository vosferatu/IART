% grau de impureza do conjunto inicial
%
% E(S) = sum(i=1 -> n)  - pi * log2(pi)
%

% Gain(S,A) = E(S) - sum(j=1 -> |A|) pj * E(Sj)
%                   |                          |
%                               E/Q/A)
% entropia de subconjuntos (outros parâmetros)

% o que queremos saber e se emigra ou nao

% Idade  Nacionalidade  Situação Familiar  Emigra EUA

% jovem   alemã          solteiro             sim

% media   francesa       solteiro             sim

% media   alemã          solteiro             sim

% jovem   italiana       solteiro             não

% media   alemã          casado               não

% media   italiana       solteiro             não

% media   italiana       casado               não

% jovem   alemã          casado               não


% -3/8*log2(3/8) + - 5/8*log2(5/8) = 0.954
% neste caso E(S) = 0.954, a entropia é alta


% Gain(emigra/idade) = 3/8*[-1/3*log2(1/3) + -2/3*log2(2/3)] +
%                     5/8*[-2/5*log2(2/5) + -3/5*log2(3/5)]
%                   = 0.951

% Gain(emigra/nacionalidade) = 4/8*[-2/4*log2(2/4) + -2/4*log2(2/4)] +
%                     1/8*[-1/1*log2(1/1) + -1/1*log2(1/1)] (1/8*0) -> CONJUNTO PURO +
                      3/8*[0] -> nenhum dos italianos imigra, conjunto puro
%                   = 0.5 -> metade do conjunto

% Gain(emigra/sit.Familiar) = 5/8*[-2/5*log2(2/5) + -3/5*log2(3/5)] +
%                     3/8*[0] -> nenhum dos casados imigra, conjunto puto
%                   = 0.607



% nacionalidade alemã -> entropia 1, maior do que a entropia do conjunto

%                             nacionalidade

%              alemã               francesa             italiana

%        Situação Familiar          [sim]                  [não]

%      solteiro       casado

%       [sim]          [não]


% S alemã

%  E(emigra/idade) = 2/4 *[-1/2*log2(1/2) + - 1/2 *log2(1/2)] +
%                            2/4 *[-1/2*log2(1/2) + - 1/2 *log2(1/2)]
%                  = 1 -> não conclusivo

%  E(emigra/sit.Familiar) = 2/4 *[0] +
%                            2/4 *[0]
%                         = 0


% OUTRAS FERRAMENTAS DO C4.5
% INFORMAÇÃO DE SEPARAÇÃO
% RAZÃO DO GANHO
