bindings_crate := "bindgen"
bindings_lib_name := "bindings"
dotnet_sdk_6 := "6.0"
dotnet_sdk_7 := "7.0"
dotnet_sdk := "8.0"

rust__path := "rust"
out__path := rust__path + "/" + bindings_crate + "/out"

csharp_sdk := "csharp_new/src/Vista.SDK"
python_sdk := "python/src/vista_sdk"

# Exposed Recipes

default:
    @just --choose

generate-bindings TARGET="debug": && copy-to-targets
    @echo [Info] Building...
    cargo build {{if lowercase(TARGET) == "release" { "--release" } else if lowercase(TARGET) == "debug" { "" } else { error("Target must be either [DEBUG, RELEASE]") } }} --manifest-path {{rust__path}}/bindgen/Cargo.toml
    @echo [Info] Generating bindings
    @echo [Info] Python
    cd {{rust__path}} && cargo run --bin uniffi-bindgen generate --library target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so --language python --out-dir bindgen/out
    @echo [Info] C#
    uniffi-bindgen-cs --out-dir {{out__path}} {{rust__path}}/{{bindings_crate}}/src/{{bindings_lib_name}}.udl --config {{rust__path}}/uniffi.toml


test-bindings:
    @echo [Info] Starting bindings tests
    @echo [Info] .NET
    dotnet test {{out__path}}/tests/dotnet/Bindings.Tests

    @echo [Info] Python
    for f in {{out__path}}/tests/python/*-tests.py; do python3 "$f"; done


# Private Recipes

copy-to-targets TARGET="debug":
  @echo [Info] Copy build output to targets

  # For bindings tests
  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{out__path}}
  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{out__path}}/tests/dotnet/Bindings.Tests/bin/{{uppercamelcase(TARGET)}}/net{{dotnet_sdk}}

  # For SDKs

  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{csharp_sdk}}/bin/{{uppercamelcase(TARGET)}}/net{{dotnet_sdk_6}}
  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{csharp_sdk}}/bin/{{uppercamelcase(TARGET)}}/net{{dotnet_sdk_7}}
  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{csharp_sdk}}/bin/{{uppercamelcase(TARGET)}}/net{{dotnet_sdk}}
  @just _copy-to-or-create {{rust__path}}/target/{{lowercase(TARGET)}}/lib{{bindings_lib_name}}.so {{python_sdk}}

  @echo [Info] Copy bindings to respective SDK

  # For SDKs
  @just _copy-to-or-create {{out__path}}/bindings.cs {{csharp_sdk}}
  @just _copy-to-or-create {{out__path}}/bindings.py {{python_sdk}}

_copy-to-or-create from to:
    mkdir -p {{to}} && cp {{from}} {{to}}
