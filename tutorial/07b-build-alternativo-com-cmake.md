# Build alternativo com CMake

## Objetivo

Configurar o CMake como uma segunda forma de compilar a mesma extensão, lado
a lado com o SCons do tópico [02](02-primeira-classe-gdextension.md), e
resolver de vez os avisos de "arquivo não encontrado" que o editor de texto
provavelmente mostrou em `player.h`/`player.cpp` desde o início deste
tutorial.

Este não é um tópico numerado como os anteriores: ele não adiciona nenhuma
funcionalidade nova ao jogo (o `Player` continua exatamente igual), apenas
uma ferramenta de build alternativa. Por isso ele fica entre os tópicos
[07](07-compilando-e-executando.md) e
[08](08-conclusao-proximos-passos.md), sem número de tópico próprio.

## Por que ter dois sistemas de build para a mesma coisa

O `godot-cpp` (tópico [02](02-primeira-classe-gdextension.md)) já vem
preparado para ser compilado tanto por SCons quanto por CMake — os dois são
mantidos oficialmente, lado a lado, dentro do próprio repositório do
`godot-cpp` (você pode conferir: `gdextension/godot-cpp/SConstruct` e
`gdextension/godot-cpp/CMakeLists.txt` existem os dois). O motivo prático de
adicionar CMake aqui não é trocar de ferramenta — é ganho de suporte de
editor: a maioria dos editores de código com autocompletar para C++ (a
extensão C/C++ do VS Code, `clangd`, CLion) entende automaticamente um
arquivo chamado `compile_commands.json`, que lista, para cada arquivo `.cpp`
do projeto, exatamente quais `#include` e flags foram usados para compilá-lo.
O CMake sabe gerar esse arquivo com uma única opção; o SCons deste projeto,
não.

Se você acompanhou os tópicos anteriores editando os arquivos `.h`/`.cpp` em
um editor com esse tipo de autocompletar, é provável que tenha visto erros
como `'godot_cpp/classes/character_body3d.hpp' file not found` diretamente no
editor, mesmo depois de o `scons` compilar com sucesso sem erro nenhum. Isso
acontecia porque o editor, sem um `compile_commands.json`, não tinha como
saber que os cabeçalhos do `godot-cpp` ficam em
`gdextension/godot-cpp/gen/include` e `gdextension/godot-cpp/include` — ele
simplesmente não sabia onde procurar. O `scons` sempre soube (o `SConstruct`
do tópico [02](02-primeira-classe-gdextension.md) informa isso com
`env.Append(CPPPATH=["src/"])`, e o `godot-cpp/SConstruct` informa o resto),
mas essa informação nunca saiu de dentro do processo do SCons. Com CMake e
`compile_commands.json`, ela passa a ficar disponível para qualquer
ferramenta externa que queira lê-la.

## Passo 1 — Escrever `gdextension/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.10)

project(fps_gdextension LANGUAGES CXX)

# Default API version to match the installed Godot editor (4.7), without
# overriding an explicit -DGODOTCPP_API_VERSION=... passed on the command
# line. This mirrors how godot-cpp's own CMakeLists.txt sets a default for
# its integration-testing target.
set(GODOTCPP_DEFAULT_API_VERSION "4.7")

add_subdirectory(godot-cpp)

add_library(fps SHARED
	src/register_types.cpp
	src/player.cpp
)

target_include_directories(fps PRIVATE src)
target_link_libraries(fps PRIVATE godot-cpp)

get_target_property(GODOTCPP_SUFFIX godot-cpp GODOTCPP_SUFFIX)

set(GAME_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../game/bin")

set_target_properties(fps PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
	PREFIX "lib"
	OUTPUT_NAME "fps${GODOTCPP_SUFFIX}"
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
	RUNTIME_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
)
```

Comparando com o `SConstruct` do tópico
[02](02-primeira-classe-gdextension.md), cada peça tem uma correspondência
direta:

- `project(fps_gdextension LANGUAGES CXX)` é o equivalente a declarar, no
  topo do arquivo, que este é um projeto C++. Todo `CMakeLists.txt` de nível
  mais alto precisa de um `project()` antes de qualquer outro comando.
- `set(GODOTCPP_DEFAULT_API_VERSION "4.7")` cumpre o mesmo papel que
  `api_version=4.7` no comando do SCons — diz qual dos arquivos
  `extension_api-4-N.json` usar. A diferença é que aqui é só um *valor
  padrão*: se alguém rodar o CMake passando explicitamente
  `-DGODOTCPP_API_VERSION=4.6`, esse valor tem prioridade. É o mesmo
  princípio de "valor padrão que pode ser sobrescrito" que muitas
  bibliotecas C++ usam para parâmetros de configuração.
- `add_subdirectory(godot-cpp)` é o equivalente a
  `SConscript("godot-cpp/SConstruct")` — reaproveita o build do `godot-cpp`,
  que por sua vez expõe um alvo (*target*) chamado `godot-cpp` para o resto
  do projeto usar.
- `add_library(fps SHARED ...)` declara explicitamente os dois arquivos-fonte
  do projeto (diferente do `Glob("src/*.cpp")` do SCons, que pega qualquer
  `.cpp` automaticamente). Listar os arquivos à mão é a prática mais comum em
  projetos CMake, porque evita uma armadilha conhecida da ferramenta: o CMake
  só reconfigura automaticamente quando os arquivos `CMakeLists.txt` mudam —
  se você criasse um `.cpp` novo e ele fosse pego só por um `Glob`, o CMake
  não perceberia esse arquivo novo até que alguém rodasse a configuração de
  novo manualmente.
- `target_link_libraries(fps PRIVATE godot-cpp)` conecta a nossa biblioteca
  ao `godot-cpp`, herdando automaticamente seus diretórios de cabeçalho — não
  precisamos repetir onde ficam `include/` e `gen/include`.
- `get_target_property(GODOTCPP_SUFFIX godot-cpp GODOTCPP_SUFFIX)` lê, de
  volta, o mesmo sufixo de nome de arquivo que o `godot-cpp` calculou para si
  mesmo (algo como `.linux.template_debug.x86_64`), para que o nome do nosso
  `.so` siga exatamente o mesmo padrão — e, por isso, o arquivo
  `fps.gdextension` do tópico [07](07-compilando-e-executando.md) não precisa
  de nenhuma alteração: os dois sistemas de build produzem arquivos com o
  nome idêntico.
- `LIBRARY_OUTPUT_DIRECTORY`/`RUNTIME_OUTPUT_DIRECTORY` apontam para
  `../game/bin`, o mesmo destino usado pelo SCons.

## Passo 2 — Configurar e compilar

CMake separa "configurar" (ler o `CMakeLists.txt` e decidir como compilar) de
"compilar" (efetivamente rodar o compilador) em dois comandos distintos — ao
contrário do SCons, que faz as duas coisas em uma única chamada de `scons`.

```bash
cd gdextension
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j$(nproc)
```

- `-S .` indica onde está o `CMakeLists.txt` fonte; `-B build` diz para
  colocar todos os arquivos gerados (inclusive os `.o` intermediários) dentro
  de uma pasta `build/`, separada do código-fonte — isso é chamado de *build
  fora da árvore* (*out-of-tree build*) e é a prática recomendada com CMake,
  porque apagar a pasta `build/` inteira sempre volta o projeto a um estado
  limpo, sem risco de apagar código-fonte por engano.
- `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` é a opção que gera o
  `compile_commands.json` mencionado na introdução, dentro de `build/`.
- O segundo comando, `cmake --build build`, é quem de fato invoca o
  compilador — internamente, o CMake gerou arquivos de Makefile (ou Ninja, se
  disponível) dentro de `build/`, e este comando só os executa.

A primeira configuração e build demoram tanto quanto a primeira compilação
via SCons (o `godot-cpp` inteiro precisa ser compilado uma vez); builds
seguintes, depois de mudar só `player.cpp` ou `register_types.cpp`, são
rápidos.

## Passo 3 — Conectar o editor ao `compile_commands.json`

A maioria dos editores com suporte a `clangd` (a extensão C/C++ do VS Code
com o modo IntelliSense baseado em `clangd`, CLion, Neovim com `clangd`)
procura automaticamente por um arquivo `compile_commands.json` na raiz do
projeto ou em uma pasta chamada `build/`. Como o nosso já está em
`gdextension/build/compile_commands.json`, muitos editores o encontram sem
configuração adicional. Se o seu não encontrar, um link simbólico na raiz de
`gdextension/` resolve na maioria dos casos:

```bash
cd gdextension
ln -sf build/compile_commands.json compile_commands.json
```

A partir daqui, abrir `player.h` ou `player.cpp` no editor deveria parar de
mostrar erros de "arquivo não encontrado" para os cabeçalhos do `godot-cpp` —
o editor agora sabe exatamente quais diretórios de inclusão o compilador
usou de verdade.

## Os dois builds não conflitam

SCons e CMake escrevem no mesmo arquivo final
(`game/bin/libfps.linux.template_debug.x86_64.so`), porque fizemos os dois
sistemas seguirem a mesma convenção de nome. Isso significa que você pode
usar qualquer um dos dois no dia a dia — quem compilou por último é a versão
que o Godot vai carregar da próxima vez que o jogo rodar. Nenhum dos dois
precisa saber que o outro existe.

A pasta `gdextension/build/` (gerada pelo CMake) e o link simbólico
`compile_commands.json` são artefatos de build, no mesmo sentido que os
arquivos `.os` do SCons ou o `.so` final — não fazem parte do código-fonte e
não devem ser versionados no Git.

## Exercícios

**Fácil.** Rode `rm -rf build` dentro de `gdextension/` e refaça os dois
comandos do Passo 2, confirmando que o projeto volta a compilar do zero sem
erro.

**Médio.** Configure uma segunda pasta de build para uma versão de
distribuição, sem apagar a primeira: `cmake -S . -B build-release
-DCMAKE_BUILD_TYPE=Release -DGODOTCPP_TARGET=template_release`, depois
`cmake --build build-release -j$(nproc)`. Confirme com `ls game/bin/` que
agora existem os dois arquivos `.so` (debug e release) lado a lado.

**Difícil.** Abra `gdextension/godot-cpp/CMakeLists.txt` e procure pela opção
`GODOTCPP_PRECISION`. Pesquise o que ela controla (dica: relacione com o tipo
`real_t`, mencionado no tópico [03](03-gravidade-e-movimento.md) a respeito
do cast em `Math::move_toward`) e explique, em poucas frases, por que mudar
essa opção exigiria recompilar o `godot-cpp` inteiro, não só `player.cpp`.

## Perguntas para fixação

1. Por que o CMake separa "configurar" e "compilar" em dois comandos, e o
   SCons faz tudo em um só? Isso é uma limitação, ou uma escolha de design
   com alguma vantagem?
2. O `compile_commands.json` é gerado a partir de uma compilação real, não
   escrito à mão. O que aconteceria com as sugestões do editor se você
   editasse esse arquivo manualmente para adicionar um `#include` que não
   existe de verdade no projeto?
