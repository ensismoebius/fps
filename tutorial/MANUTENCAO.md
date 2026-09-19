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
| `gdextension/src/player.h` (novas propriedades, novos métodos, nova herança) | [02](02-primeira-classe-gdextension.md), [03](03-gravidade-e-movimento.md), [04](04-camera-e-mouse.md) | Qualquer passo que declara membros ou assinaturas do `Player` |
| `gdextension/src/player.cpp` — `_physics_process` | [03-gravidade-e-movimento.md](03-gravidade-e-movimento.md) | Todos os passos do tópico, o algoritmo de movimento é montado ali linha a linha |
| `gdextension/src/player.cpp` — `_ready`, `_unhandled_input` | [04-camera-e-mouse.md](04-camera-e-mouse.md) | Captura do mouse, busca do `CameraPivot`, rotação da câmera |
| `game/main.tscn` | [05-montando-a-cena.md](05-montando-a-cena.md) | Árvore de nós, formas de colisão, malhas |
| `game/project.godot` — seção `[input]` | [05-montando-a-cena.md](05-montando-a-cena.md) | Tabela de ações de input e teclas |
| `game/bin/fps.gdextension` | [07-compilando-e-executando.md](07-compilando-e-executando.md) | Campos `entry_symbol`, `compatibility_minimum`, caminhos de bibliotecas |
| Comando de build (`scons ...`) ou plataformas suportadas | [07-compilando-e-executando.md](07-compilando-e-executando.md) | Comando exato mostrado no tópico |
| Qualquer arquivo novo em `gdextension/src/` | [08-conclusao-proximos-passos.md](08-conclusao-proximos-passos.md) | Árvore de arquivos final |

## Processo ao mudar o código

1. Identifique quais arquivos você alterou (`git diff --stat` ajuda).
2. Consulte a tabela acima para achar o(s) tópico(s) afetado(s).
3. Atualize os blocos de código citados nesses tópicos para que voltem a ser
   uma cópia literal do arquivo real — não apenas uma paráfrase.
4. Se a mudança altera o *comportamento* explicado (não só a sintaxe), releia
   a explicação em prosa do passo e ajuste-a também.
5. Recompile (`cd gdextension && scons platform=linux target=template_debug
   api_version=4.7`) e rode a verificação do [tópico
   07](07-compilando-e-executando.md) para confirmar que os passos, na ordem
   escrita, ainda produzem um build funcional.
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
