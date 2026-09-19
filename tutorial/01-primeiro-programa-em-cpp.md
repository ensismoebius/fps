# Primeiro programa em C++

## Objetivo

Entender o ciclo editar → compilar → executar em C++, fora do Godot, antes de
lidar com a complexidade extra que o engine adiciona.

## Compilador, não intérprete

Se você já programou em Python, JavaScript ou outra linguagem interpretada,
está acostumado a rodar o código diretamente: `python script.py` lê o arquivo
e o executa linha por linha, na hora. C++ não funciona assim. Antes de rodar,
todo o código precisa passar por um **compilador**, que traduz o texto que
você escreveu para instruções binárias específicas do seu processador,
produzindo um arquivo executável separado. Só depois desse passo o programa
roda — e ele roda sem precisar do compilador presente, porque já virou
código de máquina.

Essa é a razão de existir um passo de "build" em qualquer projeto C++: é
literalmente a etapa que transforma texto em programa.

## Passo 1 — Escrever o programa

Crie um arquivo `ola.cpp`:

```cpp
#include <iostream>

int main() {
    std::cout << "Ola, FPS!" << std::endl;
    return 0;
}
```

Três coisas específicas de C++ aparecem aqui:

- `#include <iostream>` traz as declarações necessárias para escrever no
  terminal. Em C++, nada fica disponível por padrão — cada recurso da
  biblioteca padrão (entrada e saída, texto, contêineres) precisa ser
  explicitamente incluído no topo do arquivo. É o equivalente a um `import`.
- `int main()` é o ponto de entrada do programa. Todo executável C++ precisa
  de exatamente uma função chamada `main`; é por ela que a execução começa.
  O `int` na frente é o tipo do valor que a função devolve ao sistema
  operacional (0 costuma significar "terminou sem erro").
- `std::cout << "..."` envia texto para a saída padrão. O `std::` na frente
  diz que `cout` vem do *namespace* (um agrupamento de nomes) da biblioteca
  padrão — evita que o nome `cout` colida com algo que você mesmo definiu.

## Passo 2 — Compilar

```bash
g++ ola.cpp -o ola
```

`g++` é o compilador. `-o ola` diz a ele para nomear o arquivo executável
gerado como `ola` (sem essa opção, o padrão no Linux seria `a.out`). Se não
houver erro nenhum, esse comando não imprime nada — no mundo Unix, silêncio
geralmente significa sucesso.

## Passo 3 — Executar

```bash
./ola
```

Saída esperada:

```
Ola, FPS!
```

O `./` na frente é necessário porque o diretório atual normalmente não está
na lista de lugares onde o sistema procura programas para rodar — sem ele, o
shell não encontraria o `ola` que você acabou de compilar.

## Por que isso importa para o Godot

Quando formos construir a GDExtension, o SCons vai fazer exatamente esses dois
passos — compilar e depois produzir uma biblioteca — só que automatizados e
com muito mais arquivos envolvidos (o código-fonte do projeto mais os
cabeçalhos do `godot-cpp`). Entender que "compilar" é uma etapa concreta, que
pode ter sucesso ou falhar com uma mensagem de erro específica, é a base para
conseguir ler os erros que vamos encontrar mais adiante.

## Exercícios

**Fácil.** Altere `ola.cpp` para imprimir seu próprio nome em vez de
"Ola, FPS!". Recompile e rode.

**Médio.** Crie uma função `saudacao` que recebe um nome (`std::string`) e
devolve uma frase de saudação, e chame essa função a partir de `main`.
Lembre-se de incluir `<string>`.

**Difícil.** Apague o `;` do final da linha `return 0;` e tente compilar de
novo. Leia a mensagem de erro do `g++` com atenção — ela vai apontar a linha
errada (normalmente a linha seguinte, não a que falta o `;`). Guarde essa
observação: erros de compilador em C++ frequentemente apontam para onde o
compilador *percebeu* o problema, não necessariamente para onde ele
*começou*. Vamos voltar a esse assunto no tópico
[06-lendo-erros-do-compilador.md](06-lendo-erros-do-compilador.md).

## Perguntas para fixação

1. Por que `./ola` funciona mas apenas `ola` (sem o `./`) geralmente não
   funciona no Linux?
2. Se você apagar a linha `#include <iostream>` e tentar compilar, o que
   você espera que aconteça? Teste e compare com sua previsão.
