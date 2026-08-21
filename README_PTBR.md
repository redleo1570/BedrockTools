# Render Optimizer V2 — mod independente

Esta versão usa o **template público do LeviLauncher** para criar um `.levipack` separado e usa a **API pública do BedrockTools** somente como ponte de compatibilidade.

## Já implementado

- Biblioteca `libRenderOptimizer.so` separada.
- `RenderOptimizer.levipack` separado.
- Menu próprio via `pl::modmenu`.
- FPS alvo, limiares, janela de amostras e estabilidade.
- Histerese e recuperação gradual.
- Assinatura/probe de:
  - `RenderLevel`
  - `TessellatorBegin`
  - `MeshHelpersRenderMeshImmediately`
  - `RenderChunkCoordinatorSetAllDirty`
- Recepção do evento `Frame` do BedrockTools para medir frametime real.
- ARM64 / NDK r28c.

## O que NÃO faz ainda

A V2 não instala hooks no renderer. Ela é uma **build de validação segura**: confirma que o mod separado carrega, que o menu aparece, que a API BedrockTools está acessível e quais assinaturas existem.

Isso é intencional. Os tipos de função de `RenderLevel`/tessellation precisam ser validados antes de colocar um detour. O primeiro hook da V3 será instalado somente se o alvo resolver para endereço válido; se não resolver, o mod continua funcionando sem o hook.

## Dependência

O `.so` é separado do `libBedrockTools.so`, mas a V2 usa a API ABI pública do BedrockTools para:
1. receber o evento `Frame`;
2. resolver as assinaturas versionadas.

Portanto, para o monitor de frametime/probe funcionar, BedrockTools deve estar carregado. O mod continua sendo um pacote `.levipack` independente.

A documentação do BedrockTools confirma que ele expõe uma API ABI versionada (`AbiVersion = 1`) com `resolveSignature`, `subscribe` e `unsubscribe`, além das assinaturas de renderização citadas.
