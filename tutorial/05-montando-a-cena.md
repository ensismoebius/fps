# Tópico 4 — Montando a cena no editor

## Objetivo

Construir a cena `main.tscn` no editor do Godot — o chão, a luz, o jogador e
sua câmera — e mapear as teclas WASD e espaço para as ações que o código em
C++ já espera (`move_forward`, `move_back`, `move_left`, `move_right`,
`jump`).

Diferente dos tópicos anteriores, aqui o trabalho principal acontece no
editor gráfico do Godot, não em um editor de texto. Os arquivos mostrados
abaixo (`main.tscn` e a seção `[input]` de `project.godot`) são o resultado
salvo dessas ações — você não precisa digitá-los à mão, mas vale entender o
que cada parte significa, porque são arquivos de texto comuns que também
podem ser versionados e revisados, como qualquer código.

## Passo 1 — Mapear as ações de input

Abra **Project > Project Settings > Input Map** e crie cinco ações, cada uma
associada a uma tecla física:

| Ação | Tecla |
|---|---|
| `move_forward` | W |
| `move_back` | S |
| `move_left` | A |
| `move_right` | D |
| `jump` | Barra de espaço |

Essas strings (`"move_forward"`, `"jump"`, etc.) são exatamente os nomes que
o código de `Player::_physics_process` já usa para ler o teclado — se você
digitar um nome diferente aqui, o C++ simplesmente não vai reagir a nenhuma
tecla, sem gerar erro nenhum (porque, do ponto de vista do `Input`, é só uma
ação que nunca é ativada).

O resultado é salvo automaticamente na seção `[input]` de `project.godot`:

```ini
[input]

move_forward={
"deadzone": 0.5,
"events": [Object(InputEventKey,"resource_local_to_scene":false,"resource_name":"","device":-1,"window_id":0,"alt_pressed":false,"shift_pressed":false,"ctrl_pressed":false,"meta_pressed":false,"pressed":false,"keycode":0,"physical_keycode":87,"key_label":0,"unicode":0,"location":0,"echo":false,"script":null)
]
}
move_back={
"deadzone": 0.5,
"events": [Object(InputEventKey,"resource_local_to_scene":false,"resource_name":"","device":-1,"window_id":0,"alt_pressed":false,"shift_pressed":false,"ctrl_pressed":false,"meta_pressed":false,"pressed":false,"keycode":0,"physical_keycode":83,"key_label":0,"unicode":0,"location":0,"echo":false,"script":null)
]
}
move_left={
"deadzone": 0.5,
"events": [Object(InputEventKey,"resource_local_to_scene":false,"resource_name":"","device":-1,"window_id":0,"alt_pressed":false,"shift_pressed":false,"ctrl_pressed":false,"meta_pressed":false,"pressed":false,"keycode":0,"physical_keycode":65,"key_label":0,"unicode":0,"location":0,"echo":false,"script":null)
]
}
move_right={
"deadzone": 0.5,
"events": [Object(InputEventKey,"resource_local_to_scene":false,"resource_name":"","device":-1,"window_id":0,"alt_pressed":false,"shift_pressed":false,"ctrl_pressed":false,"meta_pressed":false,"pressed":false,"keycode":0,"physical_keycode":68,"key_label":0,"unicode":0,"location":0,"echo":false,"script":null)
]
}
jump={
"deadzone": 0.5,
"events": [Object(InputEventKey,"resource_local_to_scene":false,"resource_name":"","device":-1,"window_id":0,"alt_pressed":false,"shift_pressed":false,"ctrl_pressed":false,"meta_pressed":false,"pressed":false,"keycode":0,"physical_keycode":32,"key_label":0,"unicode":0,"location":0,"echo":false,"script":null)
]
}
```

Os números em `physical_keycode` (87, 83, 65, 68, 32) são os códigos das
teclas W, S, A, D e espaço, respectivamente — o editor os gera ao gravar a
tecla que você aperta; você nunca precisa descobri-los de cabeça.
`physical_keycode`, especificamente (em vez de só `keycode`), identifica a
tecla pela posição física no teclado, não pelo caractere que ela produz — o
que importa para um jogo, já que "W" deve significar "andar para frente"
mesmo em um teclado com layout diferente do QWERTY.

## Passo 2 — Construir a árvore de nós

Crie uma nova cena 3D (`main.tscn`) com esta estrutura:

```
Main (Node3D)
├── Floor (StaticBody3D)
│   ├── CollisionShape3D  (forma: BoxShape3D, tamanho 20x1x20)
│   └── MeshInstance3D    (malha: BoxMesh, mesmo tamanho)
├── DirectionalLight3D
└── Player (Player)                      ← o tipo custom que registramos em C++
    ├── CollisionShape3D  (forma: CapsuleShape3D, raio 0.4, altura 1.8)
    ├── MeshInstance3D    (malha: CapsuleMesh, mesmas medidas)
    └── CameraPivot (Node3D)             ← elevado 0.7 no eixo Y, altura dos olhos
        └── Camera3D                     ← "Current" marcado como ativo
```

Pontos que merecem explicação:

- `StaticBody3D` é um corpo físico que não se move nunca — perfeito para o
  chão, que só precisa participar de colisões, nunca reagir a elas.
- Todo corpo físico no Godot precisa de um `CollisionShape3D` para saber
  contra o que colidir, e separadamente um `MeshInstance3D` para saber o que
  desenhar — as duas coisas são independentes de propósito (você poderia ter
  uma forma de colisão simples, tipo uma caixa, e uma malha visual bem mais
  detalhada por cima).
- O nó `Player` na cena precisa ser criado com o tipo `Player` — o mesmo nome
  que registramos com `ClassDB::register_class<Player>()` no tópico
  [02](02-primeira-classe-gdextension.md). Se a extensão ainda não tiver sido
  compilada com sucesso, `Player` não vai aparecer na lista de tipos de nó do
  editor.
- O nome `CameraPivot`, exatamente esse, precisa bater com a string que
  `Player::_ready()` procura via `get_node_or_null(NodePath("CameraPivot"))`
  (tópico [04](04-camera-e-mouse.md)) — é uma correspondência por texto, não
  por tipo, então um erro de digitação aqui faz a busca falhar
  silenciosamente (o ponteiro `camera_pivot` fica nulo, e a inclinação
  vertical da câmera simplesmente para de funcionar).
- `Camera3D` precisa ter a propriedade `Current` marcada — é o que diz ao
  Godot qual câmera, entre possivelmente várias na cena, deve ser usada para
  renderizar a tela.

O resultado salvo, em texto:

```ini
[gd_scene load_steps=5 format=3]

[sub_resource type="CapsuleShape3D" id="CapsuleShape3D_player"]
radius = 0.4
height = 1.8

[sub_resource type="CapsuleMesh" id="CapsuleMesh_player"]
radius = 0.4
height = 1.8

[sub_resource type="BoxShape3D" id="BoxShape3D_floor"]
size = Vector3(20, 1, 20)

[sub_resource type="BoxMesh" id="BoxMesh_floor"]
size = Vector3(20, 1, 20)

[node name="Main" type="Node3D"]

[node name="Floor" type="StaticBody3D" parent="."]

[node name="CollisionShape3D" type="CollisionShape3D" parent="Floor"]
shape = SubResource("BoxShape3D_floor")

[node name="MeshInstance3D" type="MeshInstance3D" parent="Floor"]
mesh = SubResource("BoxMesh_floor")

[node name="DirectionalLight3D" type="DirectionalLight3D" parent="."]
transform = Transform3D(0.707107, -0.5, 0.5, 0, 0.707107, 0.707107, -0.707107, -0.5, 0.5, 0, 5, 0)
shadow_enabled = true

[node name="Player" type="Player" parent="."]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0)

[node name="CollisionShape3D" type="CollisionShape3D" parent="Player"]
shape = SubResource("CapsuleShape3D_player")

[node name="MeshInstance3D" type="MeshInstance3D" parent="Player"]
layers = 2
mesh = SubResource("CapsuleMesh_player")

[node name="CameraPivot" type="Node3D" parent="Player"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.7, 0)

[node name="Camera3D" type="Camera3D" parent="Player/CameraPivot"]
cull_mask = 1048573
current = true
```

Um `.tscn` é um formato de texto próprio do Godot: a seção `[gd_scene]` no
topo declara quantos `load_steps` (recursos + nós) existem; cada
`[sub_resource]` é um recurso reutilizável identificado por um `id` (aqui,
as formas e malhas); e cada `[node]` declara um nó da árvore, com `parent`
indicando onde ele se encaixa (`"."` significa "filho direto da raiz da
cena").

## Passo 3 — Evitar que a câmera veja o próprio corpo do jogador

Rode o jogo neste ponto (adiantando um pouco o tópico
[07](07-compilando-e-executando.md)) e você vai notar um problema clássico de
câmera em primeira pessoa: a tela fica parcialmente tomada por uma forma
preta. A causa é geométrica, não um bug de física — `CameraPivot` fica a 0.7
de altura, e a cápsula visual do jogador tem 1.8 de altura e 0.4 de raio
centrada na origem; nessa posição, a câmera está *dentro* do volume da
própria cápsula. Ela acaba enxergando o lado de dentro da malha do próprio
personagem.

A solução padrão no Godot não é mover a câmera para fora do corpo (isso
mudaria o ponto de vista) — é dizer explicitamente à câmera para não
desenhar aquele objeto específico, usando **camadas de renderização**
(*render layers*). Toda `VisualInstance3D` (o que inclui `MeshInstance3D`)
pertence a uma ou mais camadas numeradas de 1 a 20, e toda `Camera3D` só
desenha objetos cujas camadas estejam no seu `cull_mask`. Por padrão, tudo
está na camada 1, e toda câmera enxerga todas as camadas — por isso o
problema só aparece quando a câmera acaba dentro de um objeto que ela mesma
deveria evitar ver.

Ajuste dois nós:

- No `MeshInstance3D` do `Player`, mude a camada de `1` (padrão) só para a
  `2`, marcando a camada 2 e desmarcando a 1 na propriedade **Layers**, no
  inspetor, dentro de **VisualInstance3D > Layers**.
- No `Camera3D`, em **Cull Mask**, desmarque a camada 2, deixando todas as
  outras marcadas.

O resultado salvo:

```ini
[node name="MeshInstance3D" type="MeshInstance3D" parent="Player"]
layers = 2
mesh = SubResource("CapsuleMesh_player")
```

```ini
[node name="Camera3D" type="Camera3D" parent="Player/CameraPivot"]
cull_mask = 1048573
current = true
```

`layers = 2` é uma máscara de bits: o valor `2` corresponde a "somente a
camada 2 está marcada" (a camada 1 tem valor 1, a camada 2 tem valor 2, a
camada 3 tem valor 4, e assim por diante, dobrando a cada camada — o mesmo
esquema de `physical_keycode` como número, só que aqui cada bit é uma
camada). `cull_mask = 1048573` é o valor padrão de uma câmera nova
(`1048575`, todas as 20 camadas) menos `2` — ou seja, "todas as camadas,
exceto a 2". A colisão física (`CollisionShape3D`) não é afetada por nada
disso: camadas de renderização e camadas de física são sistemas
completamente separados no Godot, apesar do nome parecido.

Com essa mudança, a malha do jogador continua existindo normalmente (você a
veria, por exemplo, em um espelho ou em uma câmera de terceira pessoa que
não excluísse a camada 2) — só a câmera em primeira pessoa do próprio
jogador deixa de desenhá-la.

## Passo 4 — Definir a cena principal

Em **Project > Project Settings > Application > Run**, defina `main.tscn`
como a cena principal — o que o Godot deve abrir ao rodar o projeto. Isso
grava a linha `run/main_scene="res://main.tscn"` em `project.godot`.

## Exercícios

**Fácil.** Ajuste a posição inicial do `Player` (o `transform` do nó) para
começar em outro canto do chão.

**Médio.** Adicione uma segunda plataforma (`StaticBody3D` com seu próprio
`CollisionShape3D` e `MeshInstance3D`) em outra posição, de tamanho e altura
diferentes do chão principal, para testar o pulo do tópico
[03](03-gravidade-e-movimento.md).

**Difícil.** Adicione uma ação nova, `crouch`, mapeada para a tecla Ctrl, e
uma propriedade `crouch_height_scale` no `Player` (seguindo o padrão de
propriedades do tópico [02](02-primeira-classe-gdextension.md)). Não é
necessário implementar o efeito visual do agachar ainda — o objetivo deste
exercício é só a prática de adicionar uma ação de input nova e uma
propriedade nova ao `Player`, ponta a ponta.

**Extra.** Reverta temporariamente o Passo 3 (volte `layers` do
`MeshInstance3D` do jogador para `1`, o padrão, e remova o `cull_mask` do
`Camera3D`) e rode o jogo de novo, para ver com os próprios olhos o problema
que a camada de renderização resolve. Depois reaplique a correção.

## Perguntas para fixação

1. Por que a forma de colisão (`CollisionShape3D`) e a malha visual
   (`MeshInstance3D`) do jogador são dois nós separados, em vez de uma única
   propriedade no `StaticBody3D`/`CharacterBody3D`?
2. Se você renomear o nó `CameraPivot` para `PivotDaCamera` na árvore da
   cena, mas esquecer de atualizar a string em `player.cpp`, o que acontece
   ao rodar o jogo? O programa trava, mostra um erro no console, ou falha
   silenciosamente?
3. O Passo 3 resolve o problema mudando a *camada de renderização* da malha,
   em vez de mover a posição da câmera para fora do corpo do jogador. Por que
   mover a câmera seria uma solução pior, mesmo que também "funcionasse"?
