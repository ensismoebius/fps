# Tópico 1 — Primeira classe reconhecida pelo Godot

## Objetivo

Entender o que é uma GDExtension, montar o esqueleto de build com SCons e
registrar uma primeira classe C++ chamada `Player`, confirmando que o Godot
consegue enxergá-la.

## O que é uma GDExtension

O Godot é escrito em C++. Uma **GDExtension** é uma biblioteca compartilhada
(`.so` no Linux) que você compila separadamente e que o Godot carrega em
tempo de execução, como se fosse um plugin nativo. Isso é diferente de um
*módulo customizado*, que exigiria recompilar o próprio Godot toda vez que
você mudasse uma linha — com GDExtension, você recompila só a sua biblioteca,
o que é muito mais rápido.

Para escrever essa biblioteca sem reimplementar manualmente toda a
comunicação com o engine, usamos o **godot-cpp**: um conjunto de cabeçalhos e
código C++ que espelha a API do Godot (as classes `Node`, `CharacterBody3D`,
`Input`, etc.) e sabe como conversar com o engine por baixo dos panos. Ele
precisa ser buscado como parte do projeto.

## Passo 1 — Trazer o godot-cpp para o projeto

Como o `godot-cpp` é um projeto próprio, versionado em seu próprio
repositório Git, ele entra no nosso projeto como um **submódulo Git** — uma
referência a um commit específico de outro repositório, guardada dentro do
nosso:

```bash
git init
git submodule add https://github.com/godotengine/godot-cpp gdextension/godot-cpp
```

O `godot-cpp` mantém, dentro da pasta `gdextension/`, arquivos JSON com o
nome `extension_api-4-N.json` — um por versão do Godot. Esse arquivo descreve
toda a API do engine (quais classes existem, quais métodos elas têm) e é a
partir dele que o `godot-cpp` gera automaticamente os cabeçalhos C++
correspondentes. Isso importa porque, mais adiante, vamos precisar dizer
explicitamente qual desses arquivos usar — o que corresponde à versão do
Godot instalada (`4.7`, no nosso caso).

## Passo 2 — O script de build (`SConstruct`)

Crie `gdextension/SConstruct`:

```python
#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    library_path = "../game/bin/libfps.{}.{}.framework/libfps.{}.{}".format(
        env["platform"], env["target"], env["platform"], env["target"]
    )
else:
    library_path = "../game/bin/libfps{}{}".format(env["suffix"], env["SHLIBSUFFIX"])

library = env.SharedLibrary(library_path, source=sources)

Default(library)
```

`SConstruct` é o nome de arquivo que o SCons procura automaticamente quando
você roda `scons` em uma pasta — cumpre o mesmo papel que um `Makefile` para
o `make`. As partes importantes:

- `SConscript("godot-cpp/SConstruct")` reaproveita o script de build que já
  vem dentro do `godot-cpp`, que sabe como compilar os próprios bindings.
- `Glob("src/*.cpp")` pega automaticamente todo arquivo `.cpp` dentro de
  `src/` — não precisamos listar arquivo por arquivo.
- `env.SharedLibrary(...)` é o comando que efetivamente produz o arquivo
  `.so`, já com um nome que muda de acordo com a plataforma (`linux`,
  `windows`, `macos`) e o tipo de build (`template_debug`,
  `template_release`).
- O resultado é escrito diretamente em `../game/bin/`, a pasta do projeto
  Godot — assim o engine já encontra a biblioteca no lugar certo, sem
  precisarmos copiar nada manualmente depois de compilar.

## Passo 3 — Declarar a classe `Player`

Toda classe C++ exposta ao Godot precisa herdar de uma classe já existente do
engine (não é possível criar um tipo totalmente novo do zero) e usar a macro
`GDCLASS`, que gera o código de integração necessário. Crie
`gdextension/src/player.h`:

```cpp
#ifndef FPS_PLAYER_H
#define FPS_PLAYER_H

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot {

class Player : public CharacterBody3D {
	GDCLASS(Player, CharacterBody3D)

private:
	double speed = 6.0;
	double jump_velocity = 4.5;
	double gravity = 9.8;
	double mouse_sensitivity = 0.003;

	Node3D *camera_pivot = nullptr;

protected:
	static void _bind_methods();

public:
	Player();
	~Player();

	void _ready() override;
	void _physics_process(double p_delta) override;
	void _unhandled_input(const Ref<InputEvent> &p_event) override;

	void set_speed(double p_speed);
	double get_speed() const;

	void set_jump_velocity(double p_jump_velocity);
	double get_jump_velocity() const;

	void set_mouse_sensitivity(double p_sensitivity);
	double get_mouse_sensitivity() const;
};

} // namespace godot

#endif // FPS_PLAYER_H
```

Alguns pontos específicos de C++ e do Godot que vale explicar:

- `#ifndef FPS_PLAYER_H` / `#define FPS_PLAYER_H` / `#endif` é um **guarda de
  inclusão**: evita que o conteúdo do arquivo seja colado duas vezes caso
  algo o inclua mais de uma vez indiretamente. Todo cabeçalho `.h` em C++
  deveria ter um.
- `CharacterBody3D` é a classe do Godot para corpos que se movem por conta
  própria e colidem com o cenário (em contraste com um corpo físico
  simulado livremente). É a base natural para um jogador.
- `GDCLASS(Player, CharacterBody3D)` registra, em tempo de compilação, que
  `Player` herda de `CharacterBody3D` para efeitos do sistema de reflexão do
  Godot — é o que permite ao editor listar `Player` como um tipo de nó
  disponível.
- `void _ready() override;` e as outras funções com `override` são **métodos
  virtuais**: o Godot os chama automaticamente em momentos específicos do
  ciclo de vida do nó (`_ready` uma vez, quando o nó entra na cena;
  `_physics_process` a cada passo de física; `_unhandled_input` quando um
  evento de entrada não foi consumido por nenhuma interface). A palavra
  `override` não é obrigatória, mas comunica a quem lê que a função está
  substituindo um comportamento herdado, e faz o compilador avisar se você
  errar a assinatura.
- Os campos `speed`, `jump_velocity`, `gravity` e `mouse_sensitivity` guardam
  o estado do jogador; vamos usá-los a partir do tópico
  [03-gravidade-e-movimento.md](03-gravidade-e-movimento.md).

Crie também `gdextension/src/player.cpp`, por enquanto só com os métodos que
o cabeçalho promete, todos vazios (vamos preenchê-los nos próximos tópicos):

```cpp
#include "player.h"

using namespace godot;

Player::Player() {}

Player::~Player() {}

void Player::_bind_methods() {
}

void Player::_ready() {
}

void Player::_physics_process(double p_delta) {
}

void Player::_unhandled_input(const Ref<InputEvent> &p_event) {
}

void Player::set_speed(double p_speed) { speed = p_speed; }
double Player::get_speed() const { return speed; }

void Player::set_jump_velocity(double p_jump_velocity) { jump_velocity = p_jump_velocity; }
double Player::get_jump_velocity() const { return jump_velocity; }

void Player::set_mouse_sensitivity(double p_sensitivity) { mouse_sensitivity = p_sensitivity; }
double Player::get_mouse_sensitivity() const { return mouse_sensitivity; }
```

Em C++, a separação entre `.h` e `.cpp` é uma convenção comum: o `.h`
declara *o que existe* (a interface), o `.cpp` implementa *como funciona*.
Isso permite que outros arquivos incluam apenas o `.h` para saber como usar
`Player`, sem precisar reprocessar a implementação inteira toda vez.

## Passo 4 — Registrar a classe no engine

Ter a classe declarada em C++ não é suficiente — o Godot só reconhece tipos
que foram explicitamente registrados. Isso acontece em um par de arquivos
próprios, chamados por convenção `register_types`. Crie
`gdextension/src/register_types.h`:

```cpp
#ifndef FPS_REGISTER_TYPES_H
#define FPS_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

void initialize_fps_module(godot::ModuleInitializationLevel p_level);
void uninitialize_fps_module(godot::ModuleInitializationLevel p_level);

#endif // FPS_REGISTER_TYPES_H
```

E `gdextension/src/register_types.cpp`:

```cpp
#include "register_types.h"

#include "player.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_fps_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Player>();
}

void uninitialize_fps_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT fps_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_fps_module);
	init_obj.register_terminator(uninitialize_fps_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
```

Duas ideias novas aqui:

- `ClassDB::register_class<Player>()` é a chamada que efetivamente avisa o
  Godot "esta classe existe e pode ser usada em cenas". Sem ela, `Player`
  seria só um tipo C++ comum, invisível para o editor.
  `MODULE_INITIALIZATION_LEVEL_SCENE` diz *em qual fase* da inicialização do
  engine isso deve acontecer — nós registramos na fase de cena, que é a fase
  correta para nós (`Node`) e derivados.
- `extern "C"` diz ao compilador para não aplicar o *name mangling* do C++
  (a prática de codificar tipos de parâmetros no nome da função, o que
  permite sobrecarga de funções) a essa função específica. Isso é necessário
  porque o Godot vai procurar essa função pelo nome exato `fps_library_init`
  dentro da biblioteca `.so` — se o nome estivesse "decorado" pelo C++, o
  Godot não o encontraria. Esse nome (`fps_library_init`) é o **ponto de
  entrada** da extensão, e vamos referenciá-lo de novo no tópico
  [07-compilando-e-executando.md](07-compilando-e-executando.md), no arquivo
  de configuração que diz ao Godot qual função chamar.

## Passo 5 — Compilar pela primeira vez

```bash
cd gdextension
scons platform=linux target=template_debug api_version=4.7 -j$(nproc)
```

- `platform=linux` e `target=template_debug` dizem qual biblioteca produzir
  (versão de depuração, para a plataforma atual).
- `api_version=4.7` seleciona qual dos arquivos
  `extension_api-4-N.json` do `godot-cpp` usar — precisa bater com a versão
  do Godot instalada (veja `godot --version`).
- `-j$(nproc)` usa todos os núcleos do processador disponíveis para acelerar
  a compilação — a primeira vez compila também o `godot-cpp` inteiro, o que
  demora bem mais que as compilações seguintes.

Se tudo estiver correto, o final da saída é parecido com:

```
Linking Shared Library .../game/bin/libfps.linux.template_debug.x86_64.so ...
scons: done building targets.
```

Se aparecer um erro, não se preocupe — o tópico
[06-lendo-erros-do-compilador.md](06-lendo-erros-do-compilador.md) foi escrito
justamente a partir de erros reais que aconteceram ao construir este mesmo
projeto.

## Exercícios

**Fácil.** Rode `scons --clean` dentro de `gdextension/` e depois compile de
novo. Observe que a primeira compilação depois de um `--clean` volta a
recompilar o `godot-cpp` inteiro, e não só o seu código.

**Médio.** Adicione um novo campo privado a `Player`, por exemplo
`double turn_speed = 2.0;`, junto com um par `set_turn_speed` /
`get_turn_speed`, seguindo o mesmo padrão de `speed`. Recompile e confirme
que não há erro (mesmo sem ainda usar o campo em lugar nenhum).

**Difícil.** Troque `MODULE_INITIALIZATION_LEVEL_SCENE` por
`MODULE_INITIALIZATION_LEVEL_CORE` na chamada de `register_class` dentro de
`initialize_fps_module` (deixe o `if` como está, comparando ainda com
`MODULE_INITIALIZATION_LEVEL_SCENE` — ou seja, o registro nunca vai
acontecer). Recompile, tente rodar o projeto no Godot (você vai precisar dos
tópicos [05](05-montando-a-cena.md) e [07](07-compilando-e-executando.md)
prontos) e observe o que muda. Depois desfaça a alteração.

## Perguntas para fixação

1. Por que a macro `GDCLASS` recebe dois nomes (`Player` e
   `CharacterBody3D`), e não só um?
2. O que aconteceria se `fps_library_init` não estivesse dentro de um bloco
   `extern "C"`? Em que momento esse problema apareceria — na compilação, ou
   só quando o Godot tentasse carregar a biblioteca?
