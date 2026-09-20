# Populando a cena com modelos 3D externos

## Objetivo

Adicionar objetos de cenário (uma caixa, um barril, uma barreira, um pilar,
uma escada, uma luminária) e uma arma à cena, modelados fora do Godot em um
programa de modelagem 3D e trazidos para o jogo prontos. Este não é um
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
