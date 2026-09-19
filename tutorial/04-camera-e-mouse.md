# Tópico 3 — Câmera em primeira pessoa e controle do mouse

## Objetivo

Fazer o jogador olhar ao redor com o mouse, do jeito característico de um
FPS: mover o mouse para os lados gira o corpo inteiro, mover para cima e para
baixo inclina só a câmera.

## Por que dois eixos de rotação diferentes

Em um FPS, girar a visão para a esquerda e para a direita gira o personagem
inteiro (afinal, ele passa a "olhar" — e portanto andar — em outra direção).
Já inclinar a visão para cima e para baixo não deveria inclinar o corpo
inteiro (imagine o personagem se curvando para trás só porque você olhou
para cima). Por isso a rotação horizontal é aplicada ao próprio nó `Player`,
e a rotação vertical é aplicada a um nó filho separado, só para a câmera —
que vamos chamar de `CameraPivot` (o "eixo da câmera"). Construímos essa
árvore de nós no tópico [05-montando-a-cena.md](05-montando-a-cena.md); por
enquanto, o código deste tópico já assume que um filho com esse nome vai
existir.

## Passo 1 — Encontrar o `CameraPivot` e capturar o mouse

```cpp
void Player::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	camera_pivot = Object::cast_to<Node3D>(get_node_or_null(NodePath("CameraPivot")));
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
}
```

`_ready()` roda uma única vez, quando o nó entra na árvore da cena — o
momento certo para procurar nós vizinhos, porque é quando temos garantia de
que a árvore inteira já foi montada.

- `get_node_or_null(NodePath("CameraPivot"))` procura, entre os filhos
  diretos de `Player`, um nó chamado exatamente `CameraPivot`. A versão
  `_or_null` (em vez de `get_node`, que existe também) devolve um ponteiro
  nulo em vez de travar o programa caso o nó não seja encontrado — mais
  seguro quando queremos checar antes de usar.
- `Object::cast_to<Node3D>(...)` converte o resultado (que vem como um
  `Node` genérico) para o tipo mais específico `Node3D`, que é o que
  precisamos para ler e escrever rotação em seguida. Esse `cast_to` também
  devolve nulo se o nó encontrado não for, de fato, um `Node3D` — por isso
  guardamos o resultado no ponteiro `camera_pivot` e checamos se é nulo antes
  de usá-lo, no passo 2.
- `Input::MOUSE_MODE_CAPTURED` esconde o cursor do sistema e prende o mouse
  dentro da janela do jogo — sem isso, mover o mouse simplesmente moveria um
  cursor de seta na tela, como em um aplicativo comum, em vez de gerar
  eventos de olhar ao redor.

## Passo 2 — Reagir ao movimento do mouse

```cpp
void Player::_unhandled_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseMotion> mouse_motion = p_event;
	if (mouse_motion.is_valid() && Input::get_singleton()->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
		Vector2 relative = mouse_motion->get_relative();

		rotate_y(-relative.x * mouse_sensitivity);

		if (camera_pivot != nullptr) {
			Vector3 pivot_rotation = camera_pivot->get_rotation();
			pivot_rotation.x -= relative.y * mouse_sensitivity;
			pivot_rotation.x = CLAMP(pivot_rotation.x, Math::deg_to_rad(-89.0), Math::deg_to_rad(89.0));
			camera_pivot->set_rotation(pivot_rotation);
		}
	}
}
```

- `_unhandled_input` é chamado para qualquer evento de entrada que nenhuma
  interface gráfica (um botão, um campo de texto) tenha consumido antes —
  ideal para controles de gameplay, que não devem competir com a interface.
- `Ref<T>` é o tipo de referência contada do Godot: em vez de um ponteiro
  comum, `Ref` cuida de liberar o objeto automaticamente quando ninguém mais
  o referencia. `Ref<InputEventMouseMotion> mouse_motion = p_event;` tenta
  converter o evento genérico `p_event` (que pode ser um clique, uma tecla,
  ou movimento de mouse) especificamente para "movimento de mouse".
  `mouse_motion.is_valid()` diz se essa conversão deu certo — ou seja, se
  este evento em particular era mesmo um movimento de mouse.
- `rotate_y(...)` gira o próprio `Player` em torno do eixo vertical. O sinal
  negativo em `-relative.x` inverte a direção para que mover o mouse para a
  direita gire a visão para a direita (sem o sinal, ficaria invertido,
  girando para o lado oposto ao esperado).
- A rotação vertical (`pivot_rotation.x`) é acumulada no `CameraPivot`, não
  no `Player` — por isso lemos e escrevemos `camera_pivot->get_rotation()` /
  `set_rotation(...)`, e não `get_rotation()` do próprio `Player`.
- `CLAMP(valor, mínimo, máximo)` restringe o ângulo vertical entre -89 e 89
  graus (convertidos para radianos com `Math::deg_to_rad`). Sem esse limite,
  seria possível girar a câmera além de olhar reto para cima ou para baixo,
  fazendo a visão virar de cabeça para baixo de forma desorientadora.

## Lembrete de inclusão

Este tópico usa `InputEventMouseMotion`, que precisa de um `#include`
próprio no topo de `player.cpp` (é uma classe diferente de `InputEvent`, que
já está incluída em `player.h`):

```cpp
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
```

Recompile com o comando de sempre para confirmar:

```bash
cd gdextension
scons platform=linux target=template_debug api_version=4.7 -j$(nproc)
```

## Exercícios

**Fácil.** Altere o valor inicial de `mouse_sensitivity` de `0.003` para
`0.01` e observe a diferença depois de recompilar e jogar (tópico
[07](07-compilando-e-executando.md)).

**Médio.** Adicione uma tecla (por exemplo `Escape`) que alterne o mouse
entre `MOUSE_MODE_CAPTURED` e `MOUSE_MODE_VISIBLE`, permitindo "soltar" o
cursor sem fechar o jogo. Você vai precisar de uma nova ação de input (veja o
tópico [05](05-montando-a-cena.md) para como declarar ações) e de um `if`
dentro de `_unhandled_input` que verifique
`Input::get_singleton()->is_action_just_pressed(...)`.

**Difícil.** O limite de -89/89 graus está fixo no código. Exponha-o como uma
propriedade configurável (`max_pitch_degrees`, seguindo o mesmo padrão de
`set_speed`/`get_speed` do tópico
[02](02-primeira-classe-gdextension.md)), e use esse valor no lugar dos
literais `-89.0` e `89.0`.

## Perguntas para fixação

1. Por que a rotação horizontal é aplicada com `rotate_y` no próprio
   `Player`, mas a rotação vertical é aplicada a um nó filho separado?
2. O que `mouse_motion.is_valid()` está realmente checando? Por que não
   basta checar se `p_event` em si não é nulo?
