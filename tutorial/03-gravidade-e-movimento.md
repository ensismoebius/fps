# Tópico 2 — Gravidade e movimento

## Objetivo

Preencher `_physics_process` para que o `Player` caia por gravidade, ande com
WASD e pule — a lógica central de um jogador de FPS.

## Passo 1 — Ler a velocidade atual

`CharacterBody3D`, a classe da qual `Player` herda, já guarda um vetor de
velocidade internamente e sabe aplicá-lo ao mover o corpo e resolver colisões
com o cenário. Nós não movemos o nó diretamente — em vez disso, ajustamos
essa velocidade e pedimos ao próprio `CharacterBody3D` que se mova de acordo
com ela.

```cpp
void Player::_physics_process(double p_delta) {
	Vector3 velocity = get_velocity();
}
```

`Vector3` é o tipo do Godot para um vetor com três componentes (`x`, `y`,
`z`) — aqui representa velocidade em metros por segundo em cada eixo.
`p_delta` é o tempo, em segundos, desde o último passo de física; multiplicar
uma taxa por `p_delta` é como se converte "por segundo" em "neste instante".

## Passo 2 — Aplicar gravidade

```cpp
	if (!is_on_floor()) {
		velocity.y -= gravity * p_delta;
	}
```

`is_on_floor()` também vem de `CharacterBody3D`: ele sabe, a partir da última
chamada de movimento, se o corpo está encostado em uma superfície considerada
chão. Só aplicamos gravidade quando o jogador está no ar — no chão, a
gravidade sendo zerada é o que impede o personagem de afundar no piso a cada
quadro.

## Passo 3 — Pular

```cpp
	if (Input::get_singleton()->is_action_just_pressed("jump") && is_on_floor()) {
		velocity.y = jump_velocity;
	}
```

`Input` é um **singleton** do Godot — uma única instância global, acessada
por `get_singleton()`, que centraliza o estado de teclado, mouse e
controles. `is_action_just_pressed("jump")` pergunta se a ação chamada
`jump` foi pressionada neste quadro específico (e não nos anteriores) — é
assim que evitamos pular repetidamente enquanto a tecla continua pressionada.
Note que `"jump"` é uma string: ela precisa corresponder exatamente a uma
ação definida no mapa de entrada do projeto, o que faremos no tópico
[05-montando-a-cena.md](05-montando-a-cena.md).

## Passo 4 — Movimento horizontal com WASD

```cpp
	Vector2 input_dir = Input::get_singleton()->get_vector("move_left", "move_right", "move_forward", "move_back");
	Vector3 direction = get_transform().basis.xform(Vector3(input_dir.x, 0, input_dir.y)).normalized();

	if (direction.length_squared() > 0.0) {
		velocity.x = direction.x * speed;
		velocity.z = direction.z * speed;
	} else {
		velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)speed);
		velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)speed);
	}
```

Passo a passo:

- `Input::get_vector(...)` combina quatro ações (esquerda, direita, frente,
  trás) em um único `Vector2` já normalizado — se você apertar duas teclas
  ao mesmo tempo (por exemplo frente e direita), o vetor resultante tem
  comprimento no máximo 1, então andar na diagonal não é mais rápido que
  andar reto.
- `get_transform().basis` é a orientação atual do nó no espaço (para onde ele
  está virado). `basis.xform(v)` transforma o vetor `v`, que está definido em
  relação ao próprio jogador ("frente dele", "direita dele"), para o espaço
  do mundo — é isso que faz "andar para frente" significar "para onde o
  jogador está olhando", e não sempre "eixo Z do mundo". Um `Basis` no
  godot-cpp não sobrecarrega o operador `*` para multiplicar por um
  `Vector3` diretamente; `xform` é o método correto para essa transformação
  (voltamos a esse detalhe no tópico
  [06-lendo-erros-do-compilador.md](06-lendo-erros-do-compilador.md), porque
  foi um erro real ao escrever este código pela primeira vez).
- Quando há entrada (`direction.length_squared() > 0.0`), a velocidade
  horizontal vai direto para `speed` na direção desejada. Quando não há
  entrada nenhuma, `Math::move_toward` desacelera suavemente até zero, em vez
  de travar o personagem instantaneamente — o `(real_t)` na frente dos
  literais `0.0` e `speed` existe porque `velocity.x` é do tipo `real_t` (que
  pode ser `float` ou `double` dependendo de como o Godot foi compilado), e
  sem o cast o compilador não consegue decidir sozinho qual das duas
  sobrecargas de `move_toward` (uma para `float`, outra para `double`) usar.

## Passo 5 — Aplicar e mover

```cpp
	set_velocity(velocity);
	move_and_slide();
}
```

`set_velocity` grava o vetor calculado de volta no `CharacterBody3D`, e
`move_and_slide()` é quem efetivamente movimenta o nó de acordo com essa
velocidade, resolvendo colisões com o cenário (por isso "slide": ao esbarrar
em uma parede em ângulo, o personagem desliza ao longo dela em vez de parar
seco).

## Código completo do passo

```cpp
void Player::_physics_process(double p_delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Vector3 velocity = get_velocity();

	if (!is_on_floor()) {
		velocity.y -= gravity * p_delta;
	}

	if (Input::get_singleton()->is_action_just_pressed("jump") && is_on_floor()) {
		velocity.y = jump_velocity;
	}

	Vector2 input_dir = Input::get_singleton()->get_vector("move_left", "move_right", "move_forward", "move_back");
	Vector3 direction = get_transform().basis.xform(Vector3(input_dir.x, 0, input_dir.y)).normalized();

	if (direction.length_squared() > 0.0) {
		velocity.x = direction.x * speed;
		velocity.z = direction.z * speed;
	} else {
		velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)speed);
		velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)speed);
	}

	set_velocity(velocity);
	move_and_slide();
}
```

A checagem `Engine::get_singleton()->is_editor_hint()` no início evita que
essa lógica rode enquanto você está apenas editando a cena no editor do
Godot (e não jogando) — sem ela, o personagem cairia e se moveria mesmo
dentro do editor, o que atrapalharia a edição. Ela exige incluir
`<godot_cpp/classes/engine.hpp>` no topo de `player.cpp`.

Lembre-se também de adicionar, no topo de `player.cpp`:

```cpp
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
```

`math.hpp` traz `Math::move_toward`. Recompile com o mesmo comando do tópico
anterior para confirmar que está tudo certo:

```bash
cd gdextension
scons platform=linux target=template_debug api_version=4.7 -j$(nproc)
```

## Exercícios

**Fácil.** Mude o valor inicial de `speed` de `6.0` para `12.0` e recompile.
(Você ainda vai precisar do tópico [07](07-compilando-e-executando.md) para
efetivamente jogar e sentir a diferença.)

**Médio.** Adicione um campo `double sprint_multiplier = 2.0;` e, dentro de
`_physics_process`, multiplique `speed` por esse valor quando a ação
`"sprint"` estiver pressionada (`Input::get_singleton()->is_action_pressed("sprint")`
— note que é `is_action_pressed`, não `is_action_just_pressed`, porque
correr deve continuar enquanto a tecla estiver segurada, não só no instante
em que foi apertada).

**Difícil.** O pulo atual é de altura fixa, não importa por quanto tempo a
tecla é segurada. Pesquise na documentação do Godot como detectar quando uma
ação é solta (`is_action_just_released`) e implemente um "pulo variável":
solte o jogador mais cedo do pico se a tecla de pulo for solta enquanto ele
ainda está subindo (dica: reduza `velocity.y` quando isso acontecer, apenas
se `velocity.y > 0`).

## Perguntas para fixação

1. Por que a gravidade é aplicada com `-=` em vez de `=`? O que aconteceria
   se fosse `velocity.y = -gravity * p_delta` em vez de `velocity.y -=
   gravity * p_delta`?
2. `is_action_just_pressed` e `is_action_pressed` parecem fazer a mesma
   coisa. Descreva um cenário de jogo onde usar o errado causaria um bug
   perceptível.
