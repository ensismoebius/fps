# Tópico 5 — Lendo erros do compilador

## Objetivo

Aprender a ler mensagens de erro do `g++`, usando três erros reais que
aconteceram durante a construção deste mesmo projeto. Erros de compilador em
C++ costumam ser longos e, à primeira vista, assustadores — mas quase sempre
têm uma estrutura previsível, e aprender a estrutura é mais útil do que
decorar soluções.

## Erro 1 — Sobrecarga ambígua

Ao escrever pela primeira vez a desaceleração do jogador (tópico
[03](03-gravidade-e-movimento.md)), o código era:

```cpp
velocity.x = Math::move_toward(velocity.x, 0.0, speed);
```

E o `g++` recusou a compilar, com uma mensagem como:

```
src/player.cpp:58:47: warning: ISO C++ says that these are ambiguous, even though the worst conversion for the first is better than the worst conversion for the second:
   58 |                 velocity.x = Math::move_toward(velocity.x, 0.0, speed);
      |                              ~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~
godot-cpp/include/godot_cpp/core/math.hpp:643:18: note: candidate 1: 'constexpr double godot::Math::move_toward(double, double, double)'
godot-cpp/include/godot_cpp/core/math.hpp:646:17: note: candidate 2: 'constexpr float godot::Math::move_toward(float, float, float)'
```

**Como ler isso:** `move_toward` existe em duas versões — uma para `double`,
outra para `float` (isso se chama **sobrecarga de função**: mesmo nome,
parâmetros de tipos diferentes). `velocity.x` é do tipo `real_t` (que no
Godot pode ser `float`), `0.0` é um literal `double`, e `speed` é um `double`
— uma mistura de tipos entre os três argumentos. O compilador não consegue
decidir sozinho qual das duas versões você quer, porque nenhuma das duas bate
exatamente com a combinação de tipos que você passou, e mais de uma
conversão implícita seria igualmente "razoável".

**A correção:** tornar os tipos explícitos, em vez de deixar o compilador
adivinhar:

```cpp
velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)speed);
```

`(real_t)valor` é um **cast explícito**: converte `valor` para o tipo
`real_t` antes de passá-lo para a função, eliminando a ambiguidade porque
agora os três argumentos batem exatamente com uma única das duas
sobrecargas.

**Lição geral:** quando o `g++` reclama de "ambiguous" e lista "candidate 1",
"candidate 2" (às vezes muitos mais), o problema quase sempre é uma mistura
de tipos numéricos diferentes (`int`, `float`, `double`) em uma chamada de
função que tem múltiplas versões. A solução é sempre a mesma: tornar os
tipos dos argumentos explícitos e consistentes.

## Erro 2 — Operador que não existe para aquele tipo

Ainda no movimento do jogador, a primeira tentativa de transformar a direção
de entrada para o espaço do mundo foi:

```cpp
Vector3 direction = (get_transform().basis * Vector3(input_dir.x, 0, input_dir.y)).normalized();
```

Erro:

```
src/player.cpp:52:52: error: no match for 'operator*' (operand types are 'godot::Basis' and 'godot::Vector3')
```

**Como ler isso:** `no match for 'operator*'` significa, literalmente, "não
existe uma versão do operador `*` que aceite estes dois tipos juntos". Em
C++, operadores como `+`, `*`, `==` não funcionam automaticamente entre tipos
definidos por bibliotecas — eles só existem se alguém explicitamente os
definiu (isso se chama **sobrecarga de operadores**). O `godot-cpp` define
`operator*` para `Basis` multiplicado por outro `Basis`, ou por um número,
mas não para `Basis` multiplicado diretamente por um `Vector3`.

**A correção:** usar o método que o próprio `Basis` oferece para essa
operação, `xform` (de "transform"):

```cpp
Vector3 direction = get_transform().basis.xform(Vector3(input_dir.x, 0, input_dir.y)).normalized();
```

**Lição geral:** "no match for 'operator*'" (ou `+`, `==`, etc.) não é um
erro de sintaxe — é o compilador dizendo que a operação que parece óbvia
matematicamente simplesmente não foi implementada para esses tipos
específicos. A solução é procurar, na documentação da classe, qual método
nomeado faz o que você queria (frequentemente `xform`, `dot`, `cross`, ou
similar, no caso de matemática vetorial do Godot).

## Erro 3 — Tipo incompleto

Ao adicionar `_unhandled_input` à classe `Player`, a compilação falhou com
uma cadeia longa de mensagens, terminando em:

```
godot-cpp/include/godot_cpp/classes/ref.hpp:65:49: error: invalid use of incomplete type 'class godot::InputEvent'
   65 |                                 if (!reference->init_ref()) {
godot-cpp/gen/include/godot_cpp/classes/node.hpp:53:7: note: forward declaration of 'class godot::InputEvent'
   53 | class InputEvent;
```

**Como ler isso:** o compilador distingue entre uma **declaração adiantada**
(*forward declaration*, `class InputEvent;`, que só diz "este tipo existe em
algum lugar", sem descrever seus membros) e a **definição completa** do
tipo (o conteúdo real da classe, obtido incluindo o cabeçalho correto). Um
outro arquivo do `godot-cpp` (`node.hpp`) só precisava saber que
`InputEvent` existe, então usa a declaração adiantada. Mas o nosso
`player.h` usa `Ref<InputEvent>` como parâmetro de uma função — e para
gerenciar a referência contada (o `init_ref()` do erro), o compilador precisa
conhecer o tipo completo, não só o nome.

**A correção:** incluir o cabeçalho que define `InputEvent` por completo, em
`player.h`:

```cpp
#include <godot_cpp/classes/input_event.hpp>
```

**Lição geral:** "incomplete type" quase sempre significa "faltou um
`#include`" — o compilador já sabe que o tipo existe (por isso não é um erro
de "tipo desconhecido"), mas só tem a versão resumida dele, insuficiente para
o que você está tentando fazer (chamar um método, acessar um campo, criar
uma instância). A pista de qual cabeçalho falta geralmente está no próprio
nome do tipo incompleto — neste caso, `godot::InputEvent` apontava
diretamente para `godot_cpp/classes/input_event.hpp`.

## Estratégia geral para ler um erro grande do C++

1. Ignore, na primeira leitura, blocos de "note: candidate ..." repetidos
   muitas vezes — eles listam alternativas que o compilador considerou e
   descartou; raramente são o ponto principal.
2. Procure a primeira linha que começa com `error:` (não `warning:` nem
   `note:`) — é normalmente ali que está o problema real; erros depois dela
   costumam ser consequência do primeiro, não problemas independentes.
3. Leia o nome do arquivo e número de linha logo antes do `error:` — é onde
   o compilador *percebeu* o problema, que pode ser um pouco depois de onde
   ele realmente começou (como no exercício do tópico
   [01](01-primeiro-programa-em-cpp.md), sobre o `;` faltando).
4. Se a mensagem menciona um tipo específico (`godot::InputEvent`,
   `godot::Basis`), esse é quase sempre o ponto de partida para pesquisar a
   documentação ou o cabeçalho correspondente.

## Exercícios

**Fácil.** Reproduza o erro 1 de propósito: troque de volta
`(real_t)0.0, (real_t)speed` por `0.0, speed` em `player.cpp` e recompile.
Confirme que a mensagem de erro é a mesma descrita aqui. Desfaça a mudança.

**Médio.** Remova a linha `#include <godot_cpp/classes/input_event.hpp>` de
`player.h` e recompile. Compare a mensagem de erro obtida com a descrita no
"Erro 3" acima — é exatamente igual, parecida, ou diferente? Anote qualquer
diferença e tente explicar por quê. Restaure a linha depois.

**Difícil.** Escreva uma função separada (fora do projeto, em um arquivo
`.cpp` isolado, sem depender do Godot) que provoque deliberadamente um erro
de "no match for 'operator*'" — por exemplo, definindo duas `struct` simples
sem operador `*` entre elas e tentando multiplicá-las. Compare a estrutura
da mensagem de erro do `g++` com a do Erro 2 acima.

## Perguntas para fixação

1. Qual é a diferença prática entre uma declaração adiantada
   (`class InputEvent;`) e um `#include` do cabeçalho completo? Por que a
   primeira é mais rápida de compilar, mas nem sempre suficiente?
2. Dos três erros descritos aqui, qual deles também aconteceria em uma
   linguagem com tipagem dinâmica (como Python ou JavaScript), e qual é
   específico de uma linguagem com tipos verificados em tempo de
   compilação, como C++?
