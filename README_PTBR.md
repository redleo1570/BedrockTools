# Render Optimizer — mod independente

Comecei a versão realmente independente usando o template público de mods nativos do LeviLauncher.

A referência pública confirma CMake, NDK 28.2.13676358, `PL_REGISTER_MOD` e os headers `pl/Mod.hpp`, `pl/Config.hpp`, `pl/ModMenu.hpp`, `pl/memory/Hook.hpp`, `Patch.hpp` e `Signature.hpp`.

## Estado desta V1

- `.so` própria: sim, arquitetura preparada para ARM64.
- Registro como mod LeviLauncher: sim.
- Configuração própria: preparada.
- Sistema de frametime/histerese: sim.
- Hooks do RenderDragon: NÃO ainda.

Não coloquei hooks especulativos. Primeiro precisamos validar as assinaturas da versão exata do Minecraft/RenderDragon. Isso evita gerar uma `.so` que simplesmente cause crash.

## Build

O workflow instala o NDK r28c e compila `arm64-v8a`.

O artefato gerado será um pacote inicial do mod. A configuração/menu deve ser ajustada ao schema exato exposto pelo preloader da versão instalada.
