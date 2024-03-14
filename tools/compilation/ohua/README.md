# The Ohua Compiler

This repository unites the different packages that form the Ohua compiler. The compiler is comprised of:

- `ohua-frontend` -- The generic compiler frontend.
- `ohua-core` -- The middle-end, if you will. This is the actual core of the compiler.
- `ohua-backend` -- Generic backend for lowering the IR to the target code subset and then to the target language AST.
- `ohuac-commons` -- Some common facilities used throughout the project.
- `ohuac-integrations` -- The actual integrations of the different languages. Contains mainly language-specific front-end and back-end extensions.
- `ohuac` -- The actual compiler.

## Prerequisites

To build the compiler, you need to install [Haskell Stack](https://docs.haskellstack.org/en/stable/README/), which will take care of selecting an appropriate compiler version and downloading all necessary dependencies.

Currently, the project is configured to use [`Nix`](https://nixos.org/download.html) for fetching packages. If you don't use NixOS or don't have Nix installed, you can switch this behavior off in `ohuac/stack.yaml`. Just set `nix -> enable` to `false`.

## Building the compiler

To build `ohuac`, just clone this repository, navigate to the compiler directory and start the build process:
```bash
git clone --recursive git@code.it4i.cz:everest/everest-sdk.git
cd everest-sdk/tools/compilation/compiler/ohua/ohuac
stack build

# you may optionally run `stack install` to get a binary of the compiler in your path
```

