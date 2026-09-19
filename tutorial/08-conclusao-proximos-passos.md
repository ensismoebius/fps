# Conclusão e próximos passos

## O projeto completo

Depois dos seis tópicos principais, a árvore de arquivos do projeto — sem
contar o submódulo `godot-cpp` e artefatos de build (`.os`, `.so`, `.godot/`)
— é esta:

```
fps/
├── gdextension/
│   ├── SConstruct                  ← Tópico 1: script de build (SCons)
│   ├── godot-cpp/                  ← submódulo Git com os bindings C++ do Godot
│   └── src/
│       ├── register_types.h        ← Tópico 1: declaração das funções de (des)inicialização
│       ├── register_types.cpp      ← Tópico 1: registro do Player no ClassDB
│       ├── player.h                ← Tópico 1: declaração da classe Player
│       └── player.cpp              ← Tópicos 2 e 3: _physics_process, _ready, _unhandled_input
├── game/
│   ├── project.godot                ← Tópico 4: seção [input] com as ações WASD/jump
│   ├── main.tscn                    ← Tópico 4: árvore de nós da cena
│   └── bin/
│       ├── fps.gdextension          ← Tópico 6: aponta o Godot para a biblioteca compilada
│       └── libfps.linux.template_debug.x86_64.so  ← gerado pelo build, não editado à mão
└── tutorial/                        ← este material
```

## Como as peças se conectam, de ponta a ponta

Quando você aperta o botão de play no editor (ou roda `godot --headless
--path . --quit`), isto acontece, na ordem:

1. O Godot lê `project.godot`, descobre que `main.tscn` é a cena principal e
   que existem ações de input chamadas `move_forward`, `jump`, etc.
2. Antes de montar qualquer cena, o Godot escaneia o projeto em busca de
   arquivos `.gdextension` (tópico [07](07-compilando-e-executando.md)) e
   encontra `game/bin/fps.gdextension`.
3. Esse arquivo diz ao Godot para carregar
   `libfps.linux.template_debug.x86_64.so` e chamar a função
   `fps_library_init` dentro dela.
4. `fps_library_init`, definida em `register_types.cpp`, registra
   `initialize_fps_module` como a função a ser chamada durante a
   inicialização — que por sua vez executa
   `ClassDB::register_class<Player>()`. A partir deste ponto, `Player`
   deixa de ser "só" uma classe C++ e passa a existir como um tipo de nó
   que o Godot reconhece.
5. O Godot carrega `main.tscn`, encontra um nó chamado `Player` do tipo
   `Player`, e consegue instanciá-lo de verdade (em vez de criar o
   *placeholder* de que fala o aviso "Cannot get class").
6. A cada quadro de física, o Godot chama `Player::_physics_process`, que lê
   `Input::get_singleton()` para saber quais teclas estão pressionadas
   (tópico [03](03-gravidade-e-movimento.md)) e ajusta a velocidade do
   personagem.
7. Sempre que o mouse se move, o Godot entrega o evento a
   `Player::_unhandled_input`, que gira o personagem e inclina a câmera
   dentro de `CameraPivot` (tópico [04](04-camera-e-mouse.md)).

Nenhuma dessas peças funciona isolada: tire o `.gdextension` e o Godot nunca
carrega a biblioteca; tire o registro no `ClassDB` e `Player` nunca aparece
como tipo de nó; tire as ações de input do `project.godot` e o C++ lê teclas
que nunca são pressionadas. É por isso que o tutorial construiu tudo nessa
ordem — cada tópico depende do anterior já estar funcionando.

## Para onde ir a partir daqui

Ideias de extensão, cada uma exercitando um pedaço diferente do que foi
ensinado:

- **Tiro.** Adicione um `RayCast3D` como filho de `Camera3D` e, em
  `_unhandled_input`, detecte um clique do mouse (`InputEventMouseButton`,
  irmão de `InputEventMouseMotion` do tópico
  [04](04-camera-e-mouse.md)) para disparar um raio e imprimir no console o
  que foi atingido.
- **Vida e dano.** Adicione propriedades `health` e `max_health` ao
  `Player`, seguindo o padrão de propriedades já usado para `speed` (tópico
  [02](02-primeira-classe-gdextension.md)), e um método `take_damage(double
  amount)`.
- **Sinais.** Pesquise como o Godot expõe **sinais** a partir do C++
  (`ADD_SIGNAL` dentro de `_bind_methods`) para avisar, por exemplo, uma
  interface de GDScript quando a vida do jogador chegar a zero — uma boa
  introdução a como C++ e GDScript podem colaborar no mesmo projeto, cada um
  fazendo a parte para a qual é mais adequado.
- **Múltiplas armas.** Use o que foi aprendido sobre classes e herança
  (`GDCLASS`) para criar uma segunda classe, por exemplo `Weapon`, que o
  `Player` referencia e delega o disparo.

Cada uma dessas extensões deveria, por sua vez, virar um novo arquivo
numerado neste tutorial (por exemplo `09-sistema-de-tiro.md`) — veja
[MANUTENCAO.md](MANUTENCAO.md) para o processo de manter tutorial e código
sincronizados conforme o projeto cresce.
