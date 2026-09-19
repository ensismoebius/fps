# Tutorial: construindo um FPS em C++ com Godot (GDExtension)

Este tutorial ensina C++ construindo, passo a passo, o mesmo projeto que existe
neste repositório: um jogo em primeira pessoa feito no Godot 4, cuja lógica de
jogador é escrita em C++ e carregada pelo engine através de uma GDExtension.

Não é uma coleção de exemplos soltos. Cada tópico adiciona uma peça real do
projeto em `gdextension/` e `game/`, na ordem em que ela precisou existir. Ao
final, o código do tutorial e o código do repositório são o mesmo código.

## Convenção de nomes

Nomes que pertencem a uma API ou biblioteca (classes do Godot como
`CharacterBody3D`, `Input`, `ClassDB`, macros como `GDCLASS`, funções como
`move_and_slide`) aparecem exatamente como na documentação oficial, em inglês.
Nomes que nós inventamos neste projeto (a classe `Player`, variáveis como
`camera_pivot`, ações de input como `move_forward`) também seguem o padrão já
usado no código real — copie-os exatamente como aparecem.

## Pré-requisitos

- Um computador Linux com acesso a `sudo` (os exemplos usam Arch Linux e
  `pacman`; troque pelo gerenciador de pacotes da sua distribuição se for
  diferente).
- Nenhuma experiência prévia com C++ é necessária. Assume-se que você já
  programou antes em alguma linguagem (variáveis, funções, laços, condicionais)
  — o tutorial explica o que é específico de C++ conforme aparece.

## Ordem de leitura

| Arquivo | Conteúdo |
|---|---|
| [00-preparando-o-ambiente.md](00-preparando-o-ambiente.md) | Instalando Godot, SCons e as demais ferramentas |
| [01-primeiro-programa-em-cpp.md](01-primeiro-programa-em-cpp.md) | O ciclo editar → compilar → executar, fora do Godot |
| [02-primeira-classe-gdextension.md](02-primeira-classe-gdextension.md) | **Tópico 1** — O que é uma GDExtension e a primeira classe reconhecida pelo Godot |
| [03-gravidade-e-movimento.md](03-gravidade-e-movimento.md) | **Tópico 2** — Física, gravidade e movimento com WASD |
| [04-camera-e-mouse.md](04-camera-e-mouse.md) | **Tópico 3** — Câmera em primeira pessoa e captura do mouse |
| [05-montando-a-cena.md](05-montando-a-cena.md) | **Tópico 4** — Construindo `main.tscn` e o mapa de teclas |
| [06-lendo-erros-do-compilador.md](06-lendo-erros-do-compilador.md) | **Tópico 5** — Como ler e corrigir erros reais do compilador C++ |
| [07-compilando-e-executando.md](07-compilando-e-executando.md) | **Tópico 6** — O comando de build, o arquivo `.gdextension` e como rodar o jogo |
| [08-conclusao-proximos-passos.md](08-conclusao-proximos-passos.md) | Fechamento: a árvore de arquivos completa e para onde ir a partir daqui |

Os arquivos `00` e `01` são preparação e fundamentos de C++; não fazem parte da
numeração dos tópicos principais porque não introduzem uma peça nova do jogo,
apenas o ferramental necessário para chegar até ele. O arquivo `08` fecha o
tutorial da mesma forma.

## Mantendo o tutorial atualizado

Este tutorial é o *log de construção* do projeto: sempre que o código em
`gdextension/` ou `game/` mudar, os tópicos correspondentes precisam ser
atualizados no mesmo commit. Veja [MANUTENCAO.md](MANUTENCAO.md) para o mapa
de qual arquivo de código afeta qual tópico e o processo a seguir.
