# Tópico 6 — Compilando e executando o jogo

## Objetivo

Entender o arquivo `.gdextension` que conecta a biblioteca compilada ao
projeto Godot, o comando de build completo, e uma particularidade real do
Godot headless que pode fazer parecer que a classe `Player` "sumiu".

## Passo 1 — O arquivo `.gdextension`

Dentro de `game/bin/`, crie `fps.gdextension`:

```ini
[configuration]

entry_symbol = "fps_library_init"
compatibility_minimum = "4.7"
reloadable = true

[libraries]

linux.debug.x86_64 = "res://bin/libfps.linux.template_debug.x86_64.so"
linux.release.x86_64 = "res://bin/libfps.linux.template_release.x86_64.so"
windows.debug.x86_64 = "res://bin/libfps.windows.template_debug.x86_64.dll"
windows.release.x86_64 = "res://bin/libfps.windows.template_release.x86_64.dll"
macos.debug = "res://bin/libfps.macos.template_debug.framework"
macos.release = "res://bin/libfps.macos.template_release.framework"
```

Este arquivo não é código C++ nem GDScript — é a "receita" que o Godot lê
para saber que existe uma extensão nativa e como carregá-la:

- `entry_symbol` precisa bater exatamente com o nome da função `extern "C"`
  que criamos no tópico [02](02-primeira-classe-gdextension.md),
  `fps_library_init`. É por esse nome que o Godot encontra o ponto de
  entrada dentro do arquivo `.so`.
- `compatibility_minimum` declara a versão mínima do Godot com a qual esta
  extensão funciona — o próprio engine recusa carregar a extensão se a
  versão instalada for anterior a essa.
- `reloadable = true` permite recarregar a biblioteca sem fechar o editor
  (útil durante o desenvolvimento, editando e recompilando repetidamente).
- A seção `[libraries]` lista, para cada combinação de plataforma e modo
  (debug/release), qual arquivo carregar. O Godot escolhe automaticamente a
  entrada certa de acordo com o sistema operacional e o tipo de build atual
  — por isso listamos Linux, Windows e macOS mesmo compilando só para Linux
  neste tutorial: outras linhas simplesmente ficam sem efeito até que você
  compile para essas plataformas.

Repare que os nomes dos arquivos (`libfps.linux.template_debug.x86_64.so`)
seguem exatamente o padrão que o próprio `SConstruct`
(tópico [02](02-primeira-classe-gdextension.md)) monta a partir de
`env["suffix"]` e `env["SHLIBSUFFIX"]` — os dois arquivos precisam concordar
nesse nome, ou o Godot procura em um lugar e encontra outro.

## Passo 2 — O comando de build

```bash
cd gdextension
scons platform=linux target=template_debug api_version=4.7 -j$(nproc)
```

Para gerar uma versão otimizada, de distribuição, o parâmetro que muda é
`target`:

```bash
scons platform=linux target=template_release api_version=4.7 -j$(nproc)
```

`template_debug` inclui informações extras úteis durante o desenvolvimento
(mensagens de erro mais detalhadas, verificações adicionais) e
`template_release` gera uma biblioteca menor e mais rápida, sem elas — é a
versão que você usaria para exportar o jogo para jogadores.

## Passo 3 — Rodar o jogo

Pelo editor, basta abrir `game/project.godot` no Godot e apertar o botão de
play. Para rodar sem interface gráfica (útil em scripts de automação ou em
um servidor):

```bash
cd game
godot --headless --path . --quit
```

## Uma armadilha real: o Godot precisa "escanear" a extensão primeiro

Em um projeto totalmente novo (sem nunca ter sido aberto no editor), rodar o
comando acima pode produzir:

```
ERROR: Cannot get class 'Player'.
   at: _instantiate_internal (core/object/class_db.cpp:587)
WARNING: Node Player of type Player cannot be created. A placeholder will be created instead.
```

Isso não é um bug no código C++ — é que o Godot mantém um cache interno
(dentro de uma pasta oculta `.godot/`, criada na primeira vez que o projeto é
aberto) que lista quais arquivos `.gdextension` existem no projeto. Em um
projeto que nunca foi aberto pelo editor, esse cache ainda não existe, e o
modo `--headless --path . --quit` sozinho não o cria.

A correção é rodar, uma única vez, uma passagem do editor em modo headless,
que força esse escaneamento:

```bash
godot --headless --editor --quit-after 1
```

Isso vai imprimir algo como:

```
[  33% ] first_scan_filesystem | Verificando GDExtensions...
```

e criar `.godot/extension_list.cfg`, contendo o caminho para o nosso
`fps.gdextension`. A partir daí, `godot --headless --path . --quit` volta a
funcionar normalmente, sem o erro de "Cannot get class 'Player'".

Isso só precisa ser feito uma vez por projeto (ou sempre que a pasta
`.godot/` for apagada) — não é um passo repetido a cada build. Depois da
primeira vez, o próprio editor mantém esse cache atualizado sempre que você
o abre normalmente.

## Exercícios

**Fácil.** Apague a pasta `game/.godot/` inteira e rode
`godot --headless --path . --quit` diretamente, sem o passo de escaneamento
antes. Confirme que o erro "Cannot get class 'Player'" reaparece, e que
rodar `godot --headless --editor --quit-after 1` o resolve de novo.

**Médio.** Gere uma build `template_release` (além da `template_debug` já
existente) e liste os dois arquivos `.so` resultantes em `game/bin/` com
`ls -la`. Compare o tamanho dos dois arquivos.

**Difícil.** Renomeie `entry_symbol` em `fps.gdextension` de
`"fps_library_init"` para `"fps_init"` (só no arquivo `.gdextension`, sem
mudar o nome da função em `register_types.cpp`). Rode o jogo e observe a
mensagem de erro do Godot ao tentar carregar a extensão. Depois desfaça a
mudança.

## Perguntas para fixação

1. Por que existem versões `template_debug` e `template_release` separadas,
   em vez de uma única build que serve para os dois casos?
2. O erro "Cannot get class 'Player'" acontece na fase de *compilação* do
   C++, ou na fase em que o *Godot carrega* a extensão já compilada? Como
   você distinguiria as duas coisas ao investigar um erro parecido no
   futuro?
