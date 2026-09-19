# Preparando o ambiente

## Objetivo

Instalar e confirmar as ferramentas necessárias antes de escrever qualquer
código: o editor Godot, o compilador C++ e o sistema de build que conecta os
dois.

## Por que várias ferramentas, e não só o Godot?

O Godot sozinho já roda jogos escritos na linguagem própria dele, a GDScript.
Nós vamos, em vez disso, escrever a lógica do jogador em C++ e entregar esse
código ao Godot na forma de uma **GDExtension**: uma biblioteca compartilhada
(`.so` no Linux, `.dll` no Windows) que o engine carrega em tempo de execução.
Isso exige duas peças que o Godot não traz embutidas:

- Um **compilador C++** (`g++`), que transforma o código-fonte que você
  escreve em instruções que o processador entende.
- O **SCons**, o sistema de build usado pelo próprio Godot para compilar
  ligações entre C++ e o engine (o pacote `godot-cpp`, que veremos no próximo
  tópico). SCons é parecido com o `make`, mas os arquivos de configuração são
  escritos em Python.

## Passo 1 — Instalar Godot e SCons

Em uma distribuição baseada em Arch Linux:

```bash
sudo pacman -S --needed godot scons
```

Se você usa outra distribuição, troque pelo gerenciador de pacotes
correspondente (`apt`, `dnf`, etc.) ou baixe o Godot diretamente em
[godotengine.org](https://godotengine.org/download).

## Passo 2 — Confirmar as versões instaladas

```bash
godot --version
scons --version
g++ --version
```

Neste projeto, as versões usadas foram Godot `4.7.2`, SCons `4.11.1` e o
`g++` do sistema. Anote a versão do Godot: ela importa mais adiante, porque a
GDExtension precisa declarar com qual versão do engine ela é compatível.

## Passo 3 — Criar a estrutura de pastas do projeto

Um projeto que mistura C++ e Godot costuma separar as duas coisas em pastas
irmãs: uma para o projeto Godot (cenas, recursos, configuração) e outra para o
código-fonte C++ (que é compilado antes de o Godot conseguir usá-lo).

```bash
mkdir -p game/bin gdextension/src
```

- `game/` é o projeto Godot propriamente dito — é essa pasta que você abre no
  editor.
- `game/bin/` vai guardar a biblioteca compilada (o resultado do C++) e o
  arquivo de configuração que diz ao Godot como carregá-la.
- `gdextension/` guarda o código-fonte C++ e o script de build. Nada dentro
  dela é aberto diretamente pelo editor do Godot.

Com isso, o ambiente está pronto. O próximo tópico não usa o Godot ainda —
antes de conectar C++ ao engine, vale a pena entender o ciclo básico de
compilar e rodar um programa C++ sozinho.
