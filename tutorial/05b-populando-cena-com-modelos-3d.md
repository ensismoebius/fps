# Populando a cena com modelos 3D externos

## Objetivo

Adicionar objetos de cenário (uma caixa, um barril, uma barreira, um pilar,
uma escada, uma luminária) e uma arma à cena, modelados fora do Godot em um
programa de modelagem 3D e trazidos para o jogo prontos, com colisão fiel
ao formato de cada um e propriedades físicas (atrito, elasticidade, massa)
condizentes com o material de cada objeto. Este não é um
tópico numerado como os outros: nada aqui usa C++ — é modelagem 3D e
montagem de cena — mas ele documenta uma peça real do projeto
(`game/main.tscn` ganhou um nó `Props` inteiro) que os tópicos numerados
não cobrem. Por isso ele fica entre os tópicos
[05](05-montando-a-cena.md) e [06](06-lendo-erros-do-compilador.md), sem
número de tópico próprio, do mesmo jeito que o tópico
[07b](07b-build-alternativo-com-cmake.md) documenta o CMake sem ser um
tópico numerado.

## De onde vêm os modelos

Os modelos deste projeto foram construídos no Blender (um programa de
modelagem 3D gratuito e de código aberto) a partir de formas geométricas
simples — cubos, cilindros — e depois exportados no formato **glTF**
binário (extensão `.glb`), o formato padrão da indústria para transportar
malhas 3D, materiais e hierarquia de objetos entre programas diferentes. O
Godot sabe importar `.glb` nativamente, sem plugin nenhum: basta o arquivo
existir dentro da pasta do projeto.

Isso não depende de ferramenta nenhuma além do próprio Blender — qualquer
pessoa pode abrir o Blender, modelar algo, escolher **File → Export → glTF
2.0 (.glb/.gltf)** e obter o mesmo tipo de arquivo que este tópico descreve.

## Passo 1 — A convenção de "base na origem"

Antes de exportar qualquer objeto, vale adotar uma convenção simples: modelar
cada objeto de modo que a base dele fique exatamente na origem local (altura
zero), com o objeto crescendo para cima a partir daí — exatamente a mesma
ideia já usada no tópico [05](05-montando-a-cena.md) para a cápsula do
jogador e a caixa do chão, só que agora aplicada a modelos vindos de fora do
Godot.

A vantagem é que, ao instanciar o modelo em uma cena, basta posicionar o nó
pai na altura desejada do mundo — nenhuma conta extra de "subir meio metro
para compensar o centro do objeto" é necessária, porque essa conta já foi
resolvida no momento da modelagem.

## Passo 2 — Um detalhe que confunde muita gente: Z para cima vira Y para cima

Programas de modelagem 3D como o Blender usam o eixo **Z** como "para cima".
O Godot (como a maioria dos motores de jogos, e como o próprio formato
glTF) usa o eixo **Y** como "para cima". Ao exportar para `.glb`, o Blender
faz essa conversão de eixos automaticamente — ninguém precisa girar nada à
mão — mas o resultado é que uma dimensão que era "altura" (eixo Z) no
programa de modelagem chega ao Godot como "altura" no eixo Y, e o que era
"profundidade" (eixo Y, geralmente o eixo para onde o objeto olha) chega
como eixo Z.

Isso importa na prática porque as formas de colisão (Passo 4) precisam ser
declaradas já em espaço do Godot. Um objeto modelado com 2 metros de
comprimento, 0.3 metro de espessura e 1 metro de altura vira, no Godot,
`Vector3(2, 1, 0.3)` — comprimento continua em X, mas altura (que era Z) foi
para Y, e espessura (que era Y) foi para Z. Esse é exatamente o caso da
barreira de concreto deste projeto (Passo 4).

## Passo 3 — Como o Godot enxerga um `.glb` importado

Assim que um arquivo `.glb` é colocado dentro da pasta do projeto, o Godot o
importa automaticamente na próxima varredura do sistema de arquivos,
gerando um arquivo irmão `<nome>.glb.import` com os metadados da
importação:

```ini
[remap]

importer="scene"
importer_version=1
type="PackedScene"
uid="uid://dbcmo8vuy6trb"
path="res://.godot/imported/prop_crate.glb-45abbb7959516bffd8690f4501823990.scn"

[deps]

source_file="res://assets/models/prop_crate.glb"
dest_files=["res://.godot/imported/prop_crate.glb-45abbb7959516bffd8690f4501823990.scn"]
```

Duas coisas para reparar aqui:

- `type="PackedScene"` — um modelo importado não vira uma malha solta, vira
  uma **cena inteira** (`PackedScene`), com sua própria hierarquia de nós.
  Um modelo com várias partes (como a arma do Passo 5) chega ao Godot já
  organizado em nós filhos, do jeito que foi montado no programa de
  modelagem.
- `uid="uid://dbcmo8vuy6trb"` — o Godot atribui a cada recurso importado um
  identificador único e estável (*UID*), que não muda mesmo que o arquivo
  seja renomeado ou movido de pasta depois. É esse UID, e não o caminho de
  texto, que as cenas usam para localizar o recurso de verdade — o caminho
  (`path`) serve só como pista legível para humanos.

Toda referência a um modelo externo dentro de uma cena `.tscn` começa com um
bloco `ext_resource` no topo do arquivo, feito de exatamente esses dois
dados (UID e caminho):

```ini
[ext_resource type="PackedScene" uid="uid://dbcmo8vuy6trb" path="res://assets/models/prop_crate.glb" id="ExtCrate"]
[ext_resource type="PackedScene" uid="uid://t7af7dra0h6b" path="res://assets/models/prop_barrel.glb" id="ExtBarrel"]
[ext_resource type="PackedScene" uid="uid://dsyi3qjghfril" path="res://assets/models/prop_barrier.glb" id="ExtBarrier"]
[ext_resource type="PackedScene" uid="uid://cqvyr1cjk4r1n" path="res://assets/models/prop_pillar.glb" id="ExtPillar"]
[ext_resource type="PackedScene" uid="uid://cdxeu416trnte" path="res://assets/models/prop_stairs.glb" id="ExtStairs"]
[ext_resource type="PackedScene" uid="uid://dkmbu7k4sxd0i" path="res://assets/models/prop_light_fixture.glb" id="ExtLightFixture"]
[ext_resource type="PackedScene" uid="uid://cbbtuyj4ypxr2" path="res://assets/models/weapon_pistol.glb" id="ExtPistol"]
```

`id="ExtCrate"` (e os demais) é só um apelido local, válido dentro deste
arquivo `.tscn`, para não repetir o UID inteiro toda vez que o recurso for
usado mais abaixo.

## Passo 4 — Instanciando um modelo com colisão

Um modelo importado, sozinho, é só aparência — nenhuma malha vinda de um
`.glb` vem com física embutida. Para que o jogador colida com a caixa em vez
de atravessá-la, ela precisa de um `StaticBody3D` (o mesmo tipo de nó já
usado para o chão no tópico [05](05-montando-a-cena.md)) com uma forma de
colisão desenhada à mão, do mesmo jeito que a forma de colisão do chão e do
jogador foram desenhadas à mão:

```ini
[node name="Crate01" type="StaticBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 0.5, -3)

[node name="Model" parent="Props/Crate01" instance=ExtResource("ExtCrate")]

[node name="CollisionShape3D" type="CollisionShape3D" parent="Props/Crate01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5, 0)
shape = SubResource("BoxShape3D_crate")
```

Repare na diferença entre os dois nós filhos: `Model` não tem `transform`
nenhum declarado (fica na origem local do `Crate01`) porque, seguindo o
Passo 1, o próprio modelo já traz embutido o deslocamento de meio metro para
cima — é assim que a base da caixa acaba exatamente na altura Y=0.5 do
mundo, que é a altura do topo do chão (o chão tem 1 metro de espessura,
centrado na origem, então o topo dele fica em Y=0.5, não em Y=0). Já o
`CollisionShape3D` precisa desse deslocamento (`0, 0.5, 0`) declarado
manualmente, porque uma forma de colisão nunca é derivada automaticamente da
malha visual — ela é sempre um objeto geométrico simples (caixa, cilindro,
cápsula) que alguém desenha para aproximar o formato real.

O mesmo padrão se repete para o barril e o pilar (cilindros) e para a
barreira (uma caixa, com a conversão de eixos do Passo 2 já aplicada):

```ini
[sub_resource type="BoxShape3D" id="BoxShape3D_crate"]
size = Vector3(1, 1, 1)

[sub_resource type="CylinderShape3D" id="CylinderShape3D_barrel"]
radius = 0.4
height = 1.2

[sub_resource type="BoxShape3D" id="BoxShape3D_barrier"]
size = Vector3(2, 1, 0.3)

[sub_resource type="CylinderShape3D" id="CylinderShape3D_pillar"]
radius = 0.3
height = 3.0
```

A escada, a luminária e a arma seguem exatamente o mesmo padrão — uma
`BoxShape3D` ou `CylinderShape3D` por peça, com o tamanho copiado direto das
dimensões reais daquela peça (o Passo 5 mostra como confirmar esses números
sem depender de contas de cabeça):

```ini
[sub_resource type="BoxShape3D" id="BoxShape3D_stairs_step1"]
size = Vector3(1.2, 0.2, 1.0)

[sub_resource type="BoxShape3D" id="BoxShape3D_stairs_step2"]
size = Vector3(0.8, 0.4, 1.0)

[sub_resource type="BoxShape3D" id="BoxShape3D_stairs_step3"]
size = Vector3(0.4, 0.6, 1.0)

[sub_resource type="BoxShape3D" id="BoxShape3D_lightfixture"]
size = Vector3(0.6, 0.1, 0.6)

[sub_resource type="BoxShape3D" id="BoxShape3D_pistol_grip"]
size = Vector3(0.06, 0.18, 0.09)

[sub_resource type="BoxShape3D" id="BoxShape3D_pistol_frame"]
size = Vector3(0.05, 0.06, 0.18)

[sub_resource type="BoxShape3D" id="BoxShape3D_pistol_slide"]
size = Vector3(0.044, 0.04, 0.22)

[sub_resource type="CylinderShape3D" id="CylinderShape3D_pistol_barrel"]
radius = 0.012
height = 0.06

[sub_resource type="BoxShape3D" id="BoxShape3D_pistol_sight"]
size = Vector3(0.008, 0.012, 0.016)

[sub_resource type="BoxShape3D" id="BoxShape3D_pistol_triggerguard"]
size = Vector3(0.04, 0.012, 0.06)
```

A caixa, o barril, a barreira e o pilar são cada um uma única forma
geométrica simples — por isso uma única `CollisionShape3D` basta. A escada
e a arma não são: são várias formas coladas, e cada parte que se projeta
para fora precisa de sua própria `CollisionShape3D`, todas dentro do mesmo
`StaticBody3D`, ou o jogador atravessaria a parte que não tem colisão.

Para a escada, cada degrau vira uma caixa de colisão do tamanho exato
daquele degrau:

```ini
[node name="Stairs01" type="StaticBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5, -6)

[node name="Model" parent="Props/Stairs01" instance=ExtResource("ExtStairs")]

[node name="CollisionShape3D_Step1" type="CollisionShape3D" parent="Props/Stairs01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0.6, 0.1, 0)
shape = SubResource("BoxShape3D_stairs_step1")

[node name="CollisionShape3D_Step2" type="CollisionShape3D" parent="Props/Stairs01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0.8, 0.2, 0)
shape = SubResource("BoxShape3D_stairs_step2")

[node name="CollisionShape3D_Step3" type="CollisionShape3D" parent="Props/Stairs01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 1.0, 0.3, 0)
shape = SubResource("BoxShape3D_stairs_step3")
```

Repare que cada deslocamento (`0.6, 0.1, 0`; `0.8, 0.2, 0`; `1.0, 0.3, 0`) é
exatamente a posição de cada degrau na malha visual — não é uma caixa
grande e aproximada por cima dos três degraus, é uma colisão por degrau,
subindo junto com a geometria. O jogador ainda não consegue *subir* a
escada sozinho (isso exigiria lógica própria de "degrau" no `Player`, um
problema à parte), mas ele já não atravessa nenhum dos três blocos.

A luminária de teto é uma única caixa achatada, então recebe uma única
`CollisionShape3D`, sem deslocamento (o modelo dela foi construído com a
origem já no centro da própria caixa, ao contrário dos outros — outra
lembrança de que a convenção do Passo 1 é uma escolha de quem modela, não
uma regra imposta pelo Godot):

```ini
[node name="LightFixture01" type="StaticBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 2, 3.5, 0)

[node name="Model" parent="Props/LightFixture01" instance=ExtResource("ExtLightFixture")]

[node name="CollisionShape3D" type="CollisionShape3D" parent="Props/LightFixture01"]
shape = SubResource("BoxShape3D_lightfixture")
```

## Passo 5 — A arma: seis formas e uma pegadinha de sinal

A arma (`weapon_pistol.glb`) é o modelo mais complexo do projeto: seis
partes (cabo, armação, ferrolho, cano, mira, guarda-mato) dentro de um nó
vazio que serve só de raiz. Dar a ela uma colisão que "reflete sua forma" —
e não só uma caixa grosseira em volta de tudo — significa uma
`CollisionShape3D` por parte:

```ini
[node name="Pistol01" type="StaticBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 1.2, 0.5, 1.2)

[node name="Model" parent="Props/Pistol01" instance=ExtResource("ExtPistol")]

[node name="CollisionShape3D_Grip" type="CollisionShape3D" parent="Props/Pistol01"]
transform = Transform3D(1, 0, 0, 0, 0.985585, 0.169182, 0, -0.169182, 0.985585, 0, -0.09, 0.06)
shape = SubResource("BoxShape3D_pistol_grip")

[node name="CollisionShape3D_Frame" type="CollisionShape3D" parent="Props/Pistol01"]
shape = SubResource("BoxShape3D_pistol_frame")

[node name="CollisionShape3D_Slide" type="CollisionShape3D" parent="Props/Pistol01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.035, -0.01)
shape = SubResource("BoxShape3D_pistol_slide")

[node name="CollisionShape3D_Barrel" type="CollisionShape3D" parent="Props/Pistol01"]
transform = Transform3D(1, 0, 0, 0, 0, 1, 0, -1, 0, 0, 0.035, -0.14)
shape = SubResource("CylinderShape3D_pistol_barrel")

[node name="CollisionShape3D_Sight" type="CollisionShape3D" parent="Props/Pistol01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.052, 0.03)
shape = SubResource("BoxShape3D_pistol_sight")

[node name="CollisionShape3D_TriggerGuard" type="CollisionShape3D" parent="Props/Pistol01"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, -0.03, 0.005)
shape = SubResource("BoxShape3D_pistol_triggerguard")
```

A primeira versão desta lista de colisões estava errada, e o erro é
instrutivo. O Passo 2 já avisou que a altura (Blender Z) vira Y no Godot —
mas ficou faltando dizer o que acontece com a profundidade (Blender Y). A
resposta, descoberta só quando esta arma expôs o problema, é: **Blender Y
vira Godot -Z, não +Z** — o sinal também inverte, não só o rótulo do eixo.
Nenhum dos seis modelos anteriores (caixa, barril, barreira, pilar, escada,
luminária) tinha peças deslocadas ao longo da profundidade em relação umas
às outras — todas ficavam centradas nesse eixo — então nenhuma delas
conseguiria ter revelado esse detalhe. A arma, com seis partes em posições
diferentes de profundidade (o cabo para trás, o cano para a frente), foi o
primeiro modelo com peças suficientes para expor o problema.

O jeito de descobrir isso não foi adivinhar — foi carregar a própria cena
já exportada dentro do Godot e perguntar a ela onde cada malha realmente
está, com um script descartável rodado via linha de comando:

```bash
godot --headless --script /tmp/inspect_pistol.gd
```

onde o script carrega `main.tscn`, instancia-o, percorre os nós dentro de
`Props/Pistol01/Model` e imprime a posição acumulada de cada
`MeshInstance3D`. Isso é mais confiável do que recalcular à mão a partir
dos parâmetros originais do Blender, porque não depende de ninguém lembrar
corretamente uma convenção de sinal — pergunta direto para o motor que vai
rodar o jogo de verdade.

Essa mesma investigação também revelou uma segunda armadilha: pedir ao
Godot o ângulo de rotação de uma peça já pronta (`Basis.get_euler()`) deu um
valor visivelmente errado para o cabo da arma, porque a transformação
combinava rotação **e** escala não uniforme, e `get_euler()` assume uma
rotação pura. A forma correta de extrair só a rotação foi normalizar as
colunas da matriz (dividir cada eixo pelo próprio comprimento) antes de ler
o ângulo — outro lembrete de que ferramentas de introspecção têm premissas,
e vale checar se elas realmente se aplicam ao que está sendo perguntado.

## Passo 6 — Conferindo a colisão de verdade

Depois de escrever várias formas de colisão à mão, "parece certo" não é
verificação — é só uma primeira impressão. Duas ferramentas confirmam de
verdade:

**Visualmente**, o próprio Godot sabe desenhar as formas de colisão por
cima da cena, sem precisar editar nada, com uma opção de linha de comando:

```bash
godot --path . --debug-collisions
```

**Numericamente**, é possível escrever um script Godot descartável que
carrega a cena, calcula a caixa delimitadora (*AABB*, *axis-aligned bounding
box*) da malha visual de cada objeto e a compara com a caixa delimitadora
da colisão correspondente — a mesma técnica usada para descobrir e
confirmar a correção do Passo 5. Um objeto com colisão fiel ao formato deve
ter as duas caixas praticamente idênticas; qualquer diferença grande aponta
uma forma de colisão mal posicionada, mal dimensionada, ou (como
aconteceu aqui) um eixo com o sinal trocado.

## Passo 7 — Um bug real: por que os objetos apareceram pretos

Depois de posicionar os sete modelos e rodar o jogo pela primeira vez,
alguns deles apareceram como silhuetas completamente pretas — mesmo
tendo cores e materiais definidos corretamente no Blender. A caixa de
madeira, por exemplo, aparecia com uma face marrom (a face voltada para a
luz) e o resto do objeto em preto total.

A causa não tinha nada a ver com os modelos em si: a cena só tinha uma
única `DirectionalLight3D` (o "sol" da cena, já presente desde o tópico
[05](05-montando-a-cena.md)) e nenhuma luz ambiente. Uma face cujo lado de
fora aponta para longe da única luz que existe na cena não recebe luz
nenhuma — e sem luz ambiente para preencher esse vazio, o resultado é preto
absoluto, não um cinza escuro. Isso sempre foi verdade nesta cena, só que
não aparecia antes: com pouca geometria (só o chão e a cápsula do jogador),
era raro alguma face relevante ficar de costas para a única luz. Com sete
objetos novos em posições e orientações variadas, virou inevitável.

A correção foi adicionar um nó `WorldEnvironment` com um recurso
`Environment` configurado para fornecer uma luz ambiente constante, que
ilumina minimamente toda superfície da cena, não importa a direção que ela
aponte:

```ini
[sub_resource type="Environment" id="Environment_main"]
background_mode = 0
background_color = Color(0.05, 0.06, 0.08, 1)
ambient_light_source = 2
ambient_light_color = Color(0.55, 0.6, 0.68, 1)
ambient_light_energy = 0.8
```

```ini
[node name="WorldEnvironment" type="WorldEnvironment" parent="."]
environment = SubResource("Environment_main")
```

- `background_mode = 0` corresponde à constante `BG_CLEAR_COLOR` — o fundo
  da cena (o que aparece atrás de tudo, como um céu) é simplesmente a cor
  sólida `background_color`, sem depender de um céu procedural.
- `ambient_light_source = 2` corresponde a `AMBIENT_SOURCE_COLOR` — a luz
  ambiente vem de uma cor fixa (`ambient_light_color`), não de uma imagem de
  céu. Os outros valores possíveis são `0` (`AMBIENT_SOURCE_BG`, a luz
  ambiente usa a própria cor de fundo), `1` (`AMBIENT_SOURCE_DISABLED`, sem
  luz ambiente nenhuma — o que a cena tinha antes desta correção, por
  omissão) e `3` (`AMBIENT_SOURCE_SKY`, a luz ambiente vem de um céu
  procedural, o que exigiria configurar também um recurso `Sky`).

Esses valores numéricos não são arbitrários — foram conferidos rodando um
pequeno script dentro do próprio Godot (`Environment.AMBIENT_SOURCE_COLOR`,
etc., impresso via `print()`), em vez de assumidos de memória. Se uma versão
futura do Godot renumerar essas constantes, o número certo a usar é sempre o
que o próprio motor instalado reporta, não o que está escrito aqui.

Este bug é primo do bug do tópico [05](05-montando-a-cena.md) sobre a
câmera enxergando o próprio corpo do jogador: os dois são casos de "o
renderizador só mostra exatamente aquilo que foi configurado para mostrar" —
lá, era preciso excluir uma camada explicitamente; aqui, era preciso incluir
uma fonte de luz explicitamente. Nenhum dos dois é um defeito do motor —
são lacunas de configuração que só aparecem quando a cena cresce o
suficiente para expô-las.

## Passo 8 — Propriedades físicas: material e massa

Até aqui, cada objeto tinha uma forma de colisão, mas todos se comportavam
fisicamente do mesmo jeito — o Godot usa valores padrão de atrito e
elasticidade quando nada é dito. Faz sentido que uma caixa de madeira
deslize diferente de um barril de metal, então cada corpo físico da cena
ganhou um recurso `PhysicsMaterial`, escolhido por categoria de material,
exatamente como os materiais visuais do Blender (`Prop_Wood`,
`Prop_Concrete`, `Prop_Metal_Red`) já eram compartilhados por categoria:

```ini
[sub_resource type="PhysicsMaterial" id="PhysicsMaterial_Concrete"]
friction = 0.8
bounce = 0.0

[sub_resource type="PhysicsMaterial" id="PhysicsMaterial_Wood"]
friction = 0.6
bounce = 0.0

[sub_resource type="PhysicsMaterial" id="PhysicsMaterial_Metal"]
friction = 0.4
bounce = 0.05
```

`friction` (0 a 1, geralmente) controla o quanto uma superfície resiste a
deslizar; `bounce` (0 a 1) controla quanto de velocidade um impacto
devolve — 0 significa "não quica nada". Concreto recebe atrito alto e
nenhum quique; metal recebe atrito mais baixo e um pouco de quique (imagine
o barril tombando e ricocheteando de leve); madeira fica no meio. Cada
`StaticBody3D` referencia o material da sua categoria:

```ini
[node name="Barrier01" type="StaticBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 0.5, 2)
physics_material_override = SubResource("PhysicsMaterial_Concrete")
metadata/mass = 0.0
metadata/material_kind = "concreto"
```

Repare também nas duas linhas `metadata/...`. `mass` (massa) não é uma
propriedade que o Godot usa para nada num `StaticBody3D` — corpos estáticos
são, por definição, imóveis, então a física deles nunca precisa saber
"quanto pesam". Ainda assim, documentar uma massa plausível é útil: é a
mesma informação que uma futura funcionalidade (pegar a arma, calcular o
peso de itens carregados, uma explosão que empurra objetos próximos com
força proporcional à massa) vai precisar, e é mais fácil registrar esse
número agora, junto do resto da descrição física do objeto, do que
redescobrir depois qual valor faz sentido para "um barril de metal".

Adotamos uma convenção simples para essa massa informativa:
**objetos estruturais do cenário (chão, barreira, pilar, escada, luminária)
recebem massa `0.0`**, sinalizando "isso é parte fixa do nível, nunca vai
virar um objeto dinâmico" — o mesmo `0` que mecanismos de física como o
Bullet e o PhysX usam para marcar um corpo como estático/infinito. **Objetos
que poderiam plausivelmente virar itens dinâmicos no futuro (caixa, barril,
arma) recebem uma massa realista**, estimada a partir do que aquele objeto
seria na vida real (uma pistola real pesa perto de 0.9 kg; uma caixa de
madeira vazia, uns 18 kg).

> **Atualização (Passo 9):** quando este passo foi escrito, `mass` era pura
> informação — nenhum código lia esse número. Isso não é mais verdade para a
> caixa, o barril e a pistola: no Passo 9 eles viram `RigidBody3D` de
> verdade, e `metadata/mass` é *promovido* a uma propriedade nativa `mass`
> que passa a mexer diretamente com a física. `Floor`, `Player`, `Barrier01`,
> `Pillar01`, `Stairs01` e `LightFixture01` continuam exatamente como
> descritos aqui — `mass` neles continua sendo só documentação.

Como essas propriedades são gravadas com `set_meta()`/`metadata/...`, e não
com uma propriedade nomeada de verdade como `friction`, qualquer nó aceita
metadata — inclusive o `Player`. E é aí que aparece uma limitação real do
motor: um `CharacterBody3D` (a classe da qual `Player` herda, tópico
[02](02-primeira-classe-gdextension.md)) **não tem** a propriedade
`physics_material_override` — só `StaticBody3D`, `RigidBody3D` e
`AnimatableBody3D` têm. Isso não é uma omissão do projeto, é uma decisão de
design do Godot: um `CharacterBody3D` é *cinemático* — ele se move porque o
código dele (`Player::_physics_process`, tópico
[03](03-gravidade-e-movimento.md)) manda, não porque o motor de física
simula atrito e colisão automaticos nele. Por isso o `Player` guarda
`friction`, `bounce`, `mass` e `material_kind` inteiramente como metadata,
sem nenhum `physics_material_override`:

```ini
[node name="Player" type="Player" parent="."]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0)
metadata/mass = 80.0
metadata/material_kind = "humano"
metadata/friction = 1.0
metadata/bounce = 0.0
```

A forma de descobrir que `CharacterBody3D` não tem essa propriedade não foi
ler a documentação de cabo a cabo — foi perguntar diretamente ao motor,
instanciando um `CharacterBody3D` vazio e listando suas propriedades
(`Object.get_property_list()`) à procura de qualquer coisa com
"physics_material" no nome. A primeira tentativa de atribuir a propriedade
a um nó `Player` dentro de um script de verificação falhou com
`Invalid access to property or key 'physics_material_override'` — o
próprio erro é a resposta, mas só depois de ser reproduzido e investigado,
não assumido de antemão.

## Passo 9 — Fazendo o jogo realmente usar essas propriedades

O Passo 8 deixou uma lacuna: `friction`, `bounce` e `mass` existiam como
dados na cena, mas nada os *lia*. Para dois `RigidBody3D` colidindo entre si
(ou contra um `StaticBody3D`), o Godot resolve atrito e quique sozinho —
mas o `Player` é um `CharacterBody3D` cinemático (Passo 8 já explicou por
quê), e `Player::_physics_process` (tópico
[03](03-gravidade-e-movimento.md)) sempre usou uma velocidade fixa, sem
olhar para nenhum `PhysicsMaterial`. Este passo fecha essa lacuna em três
frentes: atrito muda a tração do jogador, quique muda o pouso, e massa faz
objetos leves saírem voando enquanto objetos pesados mal se mexem.

### 9.1 — De metadata a propriedade nativa: promovendo três props a `RigidBody3D`

Metadata (`get_meta("mass")`) nunca vira física de verdade sozinha — é só um
número guardado no nó. Para que a massa da caixa, do barril e da pistola
*importem de fato*, eles precisam virar corpos dinâmicos, com uma
propriedade `mass` nativa que o motor de física realmente usa. `Barrier01`,
`Pillar01`, `Stairs01` e `LightFixture01` continuam `StaticBody3D` — são
estrutura fixa do nível (a luminária, por exemplo, fica presa no teto: não
faz sentido ela cair).

```ini
[node name="Crate01" type="RigidBody3D" parent="Props"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 0.5, -3)
mass = 18.0
physics_material_override = SubResource("PhysicsMaterial_Wood")
metadata/material_kind = "madeira"
```

Note que `metadata/mass = 18.0` sumiu — virou o `mass = 18.0` de cima, que
agora é uma propriedade real de `RigidBody3D`, não mais um metadata inerte.
`Barrel01` e `Pistol01` seguem o mesmo padrão (35.0 kg e 0.9 kg). A pistola
também ganhou um pequeno ajuste de posição, de `y = 0.5` para `y = 0.6`: as
seis formas de colisão dela (Passo 5) ficam levemente abaixo da origem do
corpo, e um `RigidBody3D` que já nasce penetrando o chão dá um pequeno
"solavanco" ao acordar, então subimos a origem o suficiente para eliminar
essa sobreposição inicial.

### 9.2 — Atrito muda a tração

Depois de `move_and_slide()`, `get_slide_collision(i)` devolve cada contato
daquele passo. Se o contato tiver normal apontando para cima (`normal.y >=
0.7`, ou seja, é chão e não parede), lemos o `physics_material_override` de
quem colidiu — funciona tanto para `StaticBody3D` (o chão) quanto para
`RigidBody3D` (as props que viraram dinâmicas no 9.1):

```cpp
current_floor_friction = 1.0;
current_floor_bounce = 0.0;

for (int i = 0; i < get_slide_collision_count(); i++) {
	Ref<KinematicCollision3D> collision = get_slide_collision(i);
	if (collision->get_normal().y < 0.7) {
		continue; // A wall or ceiling hit, not the floor.
	}

	Ref<PhysicsMaterial> material;
	Object *collider_obj = collision->get_collider();
	StaticBody3D *static_collider = Object::cast_to<StaticBody3D>(collider_obj);
	if (static_collider != nullptr) {
		material = static_collider->get_physics_material_override();
	} else {
		RigidBody3D *rigid_collider = Object::cast_to<RigidBody3D>(collider_obj);
		if (rigid_collider != nullptr) {
			material = rigid_collider->get_physics_material_override();
		}
	}

	if (material.is_valid()) {
		current_floor_friction = material->get_friction();
		current_floor_bounce = material->get_bounce();
	}
	break;
}
```

Esse valor só fica pronto *depois* de um `move_and_slide()`, então ele é
usado no tick **seguinte** — um atraso de 1/60 de segundo, imperceptível.
`current_floor_friction` entra direto na conta que decide o quanto a
velocidade horizontal muda por tick:

```cpp
real_t traction = (real_t)speed * (real_t)Math::clamp(current_floor_friction, 0.15, 1.0);

if (direction.length_squared() > 0.0) {
	velocity.x = Math::move_toward(velocity.x, direction.x * (real_t)speed, traction);
	velocity.z = Math::move_toward(velocity.z, direction.z * (real_t)speed, traction);
} else {
	velocity.x = Math::move_toward(velocity.x, (real_t)0.0, traction);
	velocity.z = Math::move_toward(velocity.z, (real_t)0.0, traction);
}
```

Antes, o código chamava `Math::move_toward(..., speed)` incondicionalmente:
com `speed = 6.0` e um "passo" de até `6.0` por tick, a velocidade sempre
alcançava o alvo (parado ou em movimento) em um único tick — atrito não
existia. Agora, `traction` é o próprio `speed` multiplicado pelo atrito da
superfície: no concreto (`0.8`) o jogador ganha e perde velocidade quase
instantaneamente; no metal (`0.4`) o mesmo `move_toward` avança bem menos
por tick, e leva vários ticks para acelerar ou frear — a sensação de "piso
escorregadio". O `clamp(..., 0.15, 1.0)` existe para nunca deixar a tração
chegar a zero (o que travaria o jogador por completo caso um material tenha
`friction = 0`).

### 9.3 — Quique no pouso

Pousar numa superfície com `bounce > 0` devolve parte da velocidade de queda
para cima. A dificuldade é o *momento exato*: a amostragem do 9.2 só fica
disponível depois do `move_and_slide()` daquele mesmo tick — e é exatamente
nesse tick que a aterrissagem acontece. Se a checagem de quique rodasse
antes da amostragem (foi o que aconteceu na primeira versão deste código),
ela sempre leria o valor do tick anterior, que — vindo de um jogador no ar,
sem nenhum contato — é sempre `0.0`. Resultado: **nenhum pouso jamais
quicava**, porque o quique real só ficava disponível tarde demais, um tick
depois de já ter sido necessário.

```cpp
real_t impact_velocity_y = velocity.y;

set_velocity(velocity);
move_and_slide();

// (amostragem do 9.2 acontece aqui, antes da checagem abaixo)

if (is_on_floor() && !was_on_floor && current_floor_bounce > 0.0 && impact_velocity_y < -1.0) {
	Vector3 bounced_velocity = get_velocity();
	bounced_velocity.y = -impact_velocity_y * (real_t)current_floor_bounce;
	set_velocity(bounced_velocity);
}
```

`was_on_floor` guarda `is_on_floor()` do início da função, antes de
`move_and_slide()` mexer em qualquer coisa — então `is_on_floor() &&
!was_on_floor` é exatamente "acabei de pousar neste tick". A verificação foi
feita com um script `--headless --script` que solta o jogador de queda livre
e imprime a velocidade vertical tick a tick (veja o Exercício de
verificação abaixo); antes da correção de ordem, a sequência era
`vel.y=-3.10 → vel.y=0.0` no pouso (nenhum quique); depois da correção,
`vel.y=-3.10 → vel.y=0.163` — exatamente `3.10 × 0.05`, o coeficiente de
quique do metal.

### 9.4 — Massa decide quem se move

`move_and_slide()` nunca empurra os `RigidBody3D` contra os quais o jogador
esbarra — ele é cinemático, então "empurrar" precisa ser código escrito à
mão:

```cpp
Vector3 horizontal_velocity = Vector3(velocity.x, 0.0, velocity.z);
real_t horizontal_speed = horizontal_velocity.length();
double mass_factor = player_mass / 80.0;

for (int i = 0; i < get_slide_collision_count(); i++) {
	Ref<KinematicCollision3D> collision = get_slide_collision(i);
	RigidBody3D *rigid_collider = Object::cast_to<RigidBody3D>(collision->get_collider());
	if (rigid_collider == nullptr) {
		continue;
	}

	Vector3 push_direction = -collision->get_normal();
	push_direction.y = 0.0;
	if (push_direction.length_squared() > 0.0) {
		rigid_collider->apply_central_force(push_direction.normalized() * (real_t)push_force * (real_t)mass_factor * MAX(horizontal_speed, (real_t)1.0));
	}
}
```

`player_mass` vem de `get_meta("mass")` do próprio `Player`, lido uma vez em
`_ready()` — a metadata de massa do Passo 8, que até aqui não fazia nada,
agora decide o quanto o jogador empurra. `mass_factor` é `1.0` para os
80 kg padrão; se algum dia essa metadata mudar, a força do empurrão muda
junto, sem tocar em nenhum código.

A parte mais importante deste bloco é **`apply_central_force`, não
`apply_central_impulse`**. Um impulso é um chute instantâneo de velocidade;
uma força é acumulada pelo motor de física e integrada ao longo do próprio
passo de física, sendo zerada automaticamente a cada tick. A primeira versão
deste código usava impulso — e como o jogador fica em contato com a caixa
por muitos ticks seguidos (encostado nela, andando para frente), cada um
dos 60 ticks por segundo aplicava um chute *inteiro* de velocidade. O
resultado, medido num script de verificação: uma caixa de 18 kg percorreu
**42 metros em 1.5 segundo** de contato — o equivalente a levar 90 socos
completos em sequência, um por tick. Trocar para `apply_central_force`
resolveu, porque força é uma grandeza "por segundo": o motor já divide o
efeito pela duração do tick internamente.

Por fim, note que a força **não depende da massa do alvo na fórmula acima**
— e é assim que deve ser. `apply_central_force()`/`apply_central_impulse()`
recebem uma força ou impulso em unidades absolutas; é o próprio motor de
física que, ao integrar o movimento, divide pela massa de cada corpo
(`aceleração = força / massa`) para chegar na mudança de velocidade. Aplicar
a mesma força a uma caixa de 18 kg e a um barril de 35 kg já basta para o
barril acelerar menos — a massa nativa promovida no 9.1 é o que faz essa
conta funcionar sem nenhuma divisão explícita no código do `Player`.

Medido com um script `--headless --script` que posiciona o jogador ao lado
de cada prop e segura "andar para frente" por 90 ticks (1.5 s):

| Prop | Massa | Distância percorrida |
|---|---|---|
| Crate01 (madeira) | 18 kg | ~6.3 m |
| Barrel01 (metal) | 35 kg | ~1.7 m |
| Pistol01 (metal) | 0.9 kg | ~76 m |

O barril, duas vezes mais pesado que a caixa, anda bem menos que a metade da
distância dela para a mesma força — e a pistola, quase 40× mais leve que a
caixa, sai literalmente voando. Esse espalhamento é o próprio objetivo da
massa nativa: sem escrever nenhuma regra de "se leve então..." no `Player`,
a física já entrega essa diferença de comportamento de graça.

## Exercícios

**Fácil.** Mude `ambient_light_energy` de `0.8` para `0.15` e depois para
`2.5`, rodando o jogo entre uma mudança e outra. Descreva o que acontece com
as faces das caixas e do pilar que ficam de costas para a
`DirectionalLight3D` em cada um dos dois casos.

**Médio.** Modele um novo objeto de cenário no Blender (por exemplo, um
banco: um bloco para o assento e dois blocos para as pernas), seguindo a
convenção do Passo 1 (base na origem), exporte como `.glb` para
`game/assets/models/`, e instancie-o em `main.tscn` como um novo
`StaticBody3D` dentro de `Props`, com uma `CollisionShape3D` que aproxime
sua forma.

**Difícil.** Em vez de luz ambiente constante, tente resolver o mesmo
problema de outra forma: remova (ou reduza bastante) o
`ambient_light_energy`, e em vez disso adicione uma segunda
`DirectionalLight3D` apontando em uma direção aproximadamente oposta à
primeira, com `light_energy` bem mais baixo que o "sol" principal, agindo
como luz de preenchimento. Compare visualmente as duas técnicas — luz
ambiente constante versus uma segunda luz direcional fraca — e descreva uma
diferença perceptível entre elas (dica: pense em como cada uma trata
sombras).

**Extra (verificação).** Rode `godot --path . --debug-collisions` e
confirme visualmente que as seis formas de colisão da arma (Passo 5) se
encaixam nas seis partes visuais dela, sem sobrar nem faltar em nenhuma
ponta. Depois, escreva (ou adapte o do Passo 5) um script
`--headless --script` que calcule a caixa delimitadora da malha de um dos
objetos simples (por exemplo `Barrel01`) e a compare com a caixa da
colisão correspondente — as duas devem sair idênticas.

**Extra (o bug de propósito).** Troque `0, 0.035, -0.14` por
`0, 0.035, 0.14` no `CollisionShape3D_Barrel` da arma (desfazendo a correção
do Passo 5) e rode `--debug-collisions` de novo. Descreva onde a colisão do
cano aparece agora em relação ao cano de verdade. Desfaça a mudança depois.

**Fácil (propriedades físicas).** Crie um `PhysicsMaterial_Glass` com
`friction = 0.2` e `bounce = 0.3` (vidro é liso e quica mais que metal ou
concreto) e aplique-o à luminária, no lugar de `PhysicsMaterial_Metal`.
Justifique, em uma frase, se essa escolha faz mais sentido para o
*case* (invólucro) da luminária ou para uma versão futura em que ela
tivesse uma cúpula de vidro visível separada.

**Médio (propriedades físicas).** Escreva um script `--headless --script`
que percorra todos os filhos de `Props`, leia `physics_material_override.friction`
e `get_meta("material_kind")` de cada um, e para a massa use `get_meta("mass")`
se o nó a tiver (ainda é o caso de `Barrier01`, `Pillar01`, `Stairs01` e
`LightFixture01`) ou a propriedade nativa `mass` caso contrário (o caso de
`Crate01`, `Barrel01` e `Pistol01`, depois do Passo 9). Imprima uma tabela e
confirme que nenhum objeto ficou sem essas três informações, seja como
metadata ou como propriedade nativa.

**Difícil (propriedades físicas).** Tente adicionar
`physics_material_override` diretamente ao nó `Player` no `main.tscn` e
rode `godot --headless --path . --quit`. Compare o resultado com o que
aconteceu ao tentar o mesmo a partir de um script GDScript (que gerou o
erro `Invalid access to property or key`). As duas tentativas falham do
mesmo jeito? Em qual das duas o Godot avisa mais cedo que a propriedade não
existe?

**Médio (Passo 9).** Em `Player::_physics_process`, troque o
`Math::clamp(current_floor_friction, 0.15, 1.0)` por
`Math::clamp(current_floor_friction, 0.0, 1.0)` (removendo o piso de
`0.15`), recompile e ponha o jogador para pousar sobre uma superfície
hipotética com `friction = 0.0`. O que acontece com o controle do
personagem, e por que o piso de `0.15` existe?

**Difícil (Passo 9 — reproduza o bug de propósito).** Troque
`apply_central_force` de volta para `apply_central_impulse` na função de
empurrão (9.4), recompile, e escreva um script `--headless --script` que
posicione o jogador do lado da caixa (`Crate01`), segure "andar para
frente" por 90 ticks, e imprima a distância percorrida pela caixa. Compare
com a tabela do 9.4. Desfaça a mudança depois.

**Extra (Passo 9 — verificação do quique).** Escreva um script
`--headless --script` que solte o jogador de uma altura (por exemplo `y =
5`) diretamente acima de `LightFixture01` (`bounce = 0.05`) e imprima
`get_velocity().y` a cada tick de física ao redor do pouso. Confirme que a
velocidade vertical vira positiva por um instante, e calcule se o valor
bate com `impacto × 0.05`.

## Perguntas para fixação

1. Por que o problema de faces pretas não apareceu em nenhum momento
   anterior do tutorial, mesmo a cena já tendo um chão e um jogador antes
   deste tópico?
2. Qual é a diferença prática entre o nó `Model` (a instância do `.glb`) não
   ter `transform` declarado dentro de `Crate01`, e o `CollisionShape3D`
   precisar de um `transform` com deslocamento `(0, 0.5, 0)` explícito? Por
   que essas duas coisas, que representam o "mesmo objeto" visualmente, são
   tratadas de formas tão diferentes pelo Godot?
3. Por que nenhum dos seis primeiros modelos (caixa, barril, barreira,
   pilar, escada, luminária) conseguiria ter revelado, sozinho, que a
   profundidade do Blender vira `-Z` (e não `+Z`) no Godot? O que
   especificamente a arma tem que os outros seis não têm?
4. `Basis.get_euler()` devolveu um ângulo visivelmente errado para o cabo da
   arma. Isso significa que o método tem um bug? Por que ou por que não —
   e o que ele exige do chamador para devolver um resultado confiável?
5. Por que faz sentido `mass = 0.0` para o pilar e a barreira continuarem
   como metadata (Passo 8), enquanto a massa do barril precisou virar uma
   propriedade nativa de `RigidBody3D` (Passo 9) para ter algum efeito? Uma
   massa maior sempre precisa ser uma propriedade nativa para "importar", ou
   depende do que o objeto faz na cena?
6. O `Player` guarda `friction` e `bounce` como metadata, não como
   `physics_material_override`. Se um dia `CharacterBody3D` ganhasse essa
   propriedade em uma versão futura do Godot, o que mudaria no
   comportamento do personagem — o `Player::_physics_process` (tópico
   [03](03-gravidade-e-movimento.md)) passaria a usar esses valores
   automaticamente, ou ainda seria preciso ler `get_meta("friction")` à mão
   dentro do código C++? (Dica: pense se `physics_material_override` faria
   o motor aplicar sozinho a tração customizada do 9.2 e o empurrão do 9.4,
   ou só o atrito/quique entre corpos que o motor já resolve automaticamente
   hoje.)
7. No 9.3, a checagem de quique lê `current_floor_bounce` logo depois da
   amostragem de superfície, na mesma função e no mesmo tick. Por que não
   bastava simplesmente mover a amostragem para o `_ready()` e rodá-la uma
   vez só, guardando o material do "chão" permanentemente?
8. A força de empurrão do 9.4 usa `MAX(horizontal_speed, 1.0)` em vez de só
   `horizontal_speed`. O que aconteceria — em termos concretos, com números
   — se o jogador encostasse na caixa parado (sem se mover), sem esse `MAX`?
