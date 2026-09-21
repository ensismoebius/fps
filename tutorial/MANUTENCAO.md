# Mapa de manutenção do tutorial

Este tutorial descreve o código deste repositório *exatamente como ele existe
hoje*. Ele não é uma cópia independente do projeto — os blocos de código em
cada tópico são trechos literais dos arquivos reais em `gdextension/` e
`game/`. Isso significa que, sempre que o código mudar, o tutorial fica
desatualizado até que alguém o revise. Esta tabela existe para tornar essa
revisão rápida: em vez de reler o tutorial inteiro, procure aqui qual tópico
cobre o arquivo que você acabou de mudar.

## Tabela de rastreamento

| Arquivo de código alterado | Tópico(s) afetado(s) | O que revisar |
|---|---|---|
| `gdextension/SConstruct` | [02-primeira-classe-gdextension.md](02-primeira-classe-gdextension.md) | Passo sobre o script de build |
| `gdextension/src/register_types.h` / `.cpp` | [02-primeira-classe-gdextension.md](02-primeira-classe-gdextension.md) | Passo sobre registrar a classe no `ClassDB` |
| `gdextension/src/player.h` (novas propriedades, novos métodos, nova herança) | [02](02-primeira-classe-gdextension.md), [03](03-gravidade-e-movimento.md), [04](04-camera-e-mouse.md), [05b — Passo 9](05b-populando-cena-com-modelos-3d.md) | Qualquer passo que declara membros ou assinaturas do `Player` (05b cobre especificamente `push_force`, `player_mass`, `current_floor_friction`/`current_floor_bounce`) |
| `gdextension/src/player.cpp` — `_physics_process` | [03-gravidade-e-movimento.md](03-gravidade-e-movimento.md), [05b — Passo 9](05b-populando-cena-com-modelos-3d.md) | O algoritmo de movimento básico (03) e a parte que lê `physics_material_override`/`mass` dos colisores para tração, quique e empurrão de `RigidBody3D` (05b, Passo 9) |
| `gdextension/src/player.cpp` — `_ready`, `_unhandled_input` | [04-camera-e-mouse.md](04-camera-e-mouse.md) | Captura do mouse, busca do `CameraPivot`, rotação da câmera |
| `game/main.tscn` — nós `Main`, `Floor`, `DirectionalLight3D`, `Player` | [05-montando-a-cena.md](05-montando-a-cena.md) | Árvore de nós, formas de colisão, malhas |
| `game/main.tscn` — nó `Props`, `WorldEnvironment`, arquivos em `game/assets/models/` | [05b-populando-cena-com-modelos-3d.md](05b-populando-cena-com-modelos-3d.md) | Lista de `ext_resource`, cada corpo físico (`StaticBody3D` ou, desde o Passo 9, `RigidBody3D` para `Crate01`/`Barrel01`/`Pistol01`) + `CollisionShape3D`, o recurso `Environment`, os `PhysicsMaterial`, a propriedade nativa `mass` dos três `RigidBody3D`, e a metadata `mass`/`material_kind`/`friction`/`bounce` de cada corpo que ainda não tem `mass` nativa (inclusive `Player` e `Floor`) |
| `game/project.godot` — seção `[input]` | [05-montando-a-cena.md](05-montando-a-cena.md) | Tabela de ações de input e teclas |
| `game/bin/fps.gdextension` | [07-compilando-e-executando.md](07-compilando-e-executando.md) | Campos `entry_symbol`, `compatibility_minimum`, caminhos de bibliotecas |
| Comando de build (`scons ...`) ou plataformas suportadas | [07-compilando-e-executando.md](07-compilando-e-executando.md) | Comando exato mostrado no tópico |
| `gdextension/CMakeLists.txt` | [07b-build-alternativo-com-cmake.md](07b-build-alternativo-com-cmake.md) | Todo o tópico — o arquivo é citado por completo |
| Novo arquivo-fonte adicionado a `add_library(fps SHARED ...)` no CMake | [07b-build-alternativo-com-cmake.md](07b-build-alternativo-com-cmake.md) | Lista de arquivos-fonte no Passo 1 (lembre-se: `add_library` lista os arquivos à mão, não usa Glob) |
| Qualquer arquivo novo em `gdextension/src/` | [08-conclusao-proximos-passos.md](08-conclusao-proximos-passos.md) | Árvore de arquivos final |

## Processo ao mudar o código

1. Identifique quais arquivos você alterou (`git diff --stat` ajuda).
2. Consulte a tabela acima para achar o(s) tópico(s) afetado(s).
3. Atualize os blocos de código citados nesses tópicos para que voltem a ser
   uma cópia literal do arquivo real — não apenas uma paráfrase.
4. Se a mudança altera o *comportamento* explicado (não só a sintaxe), releia
   a explicação em prosa do passo e ajuste-a também.
5. Recompile com os dois sistemas de build — `cd gdextension && scons
   platform=linux target=template_debug api_version=4.7` e `cmake --build
   build` (assumindo que a pasta `build/` já foi configurada, veja o tópico
   [07b](07b-build-alternativo-com-cmake.md)) — e rode a verificação do
   [tópico 07](07-compilando-e-executando.md) para confirmar que os passos,
   na ordem escrita, ainda produzem um build funcional. Se você alterou quais
   arquivos `.cpp` existem em `gdextension/src/`, lembre-se de que o SCons os
   pega automaticamente (`Glob`), mas o CMake exige atualizar a lista manual
   em `add_library(fps SHARED ...)`.
6. Se você criou um arquivo de código novo que nenhuma linha da tabela cobre,
   adicione uma linha nova aqui apontando para o tópico que passou a
   descrevê-lo — ou crie um tópico novo, se for um conceito novo, e adicione-o
   também ao índice em [README.md](README.md).

Se um tópico descreve um erro de compilação real (tópico
[06](06-lendo-erros-do-compilador.md)), e uma mudança futura no código faz
esse erro deixar de ser possível (por exemplo, uma versão nova do godot-cpp
que passa a aceitar `Basis * Vector3` diretamente), não apague o tópico — ele
ainda ensina como ler esse tipo de erro. Em vez disso, adicione uma nota
indicando a partir de qual versão o comportamento mudou.

## Nota histórica: o nó `Props` já teve um tópico pendente

`game/main.tscn` ganhou em algum momento um nó `Props` com seis objetos 3D
(caixa, barril, barreira, pilar, escada, luminária) e uma arma
(`weapon_pistol.glb`), modelados no Blender e exportados para
`game/assets/models/`, além de um `WorldEnvironment` com luz ambiente — tudo
isso sem usar C++, então sem tópico correspondente por um tempo. Essa
lacuna foi fechada pelo tópico
[05b-populando-cena-com-modelos-3d.md](05b-populando-cena-com-modelos-3d.md).
Este parágrafo fica como lembrete de que o processo descrito em "Processo ao
mudar o código", acima, é exatamente o que resolveu essa pendência: ela foi
registrada aqui assim que detectada, e resolvida com um tópico novo assim
que houve pedido explícito para atualizar o tutorial.
