# Migrating to Reflection-Based Actions (C++26)

This guide describes how to migrate existing HPX action definitions to use
C++26 static reflection, eliminating boilerplate macros while maintaining
full backwards compatibility.

## Requirements

- CMake: `-DHPX_WITH_CXX26_REFLECTION=ON`
- Compiler: GCC trunk (`-std=c++26 -freflection`) or Clang P2996
  (`-std=c++26 -freflection-latest`)
- Existing HPX_PLAIN_ACTION and HPX_DEFINE_COMPONENT_ACTION code continues
  to work unchanged — reflection path is opt-in via the build flag.

## Plain Actions

### Before (3 manual steps, error-prone)

```cpp
namespace app {
    int compute(double x, double y) { return (int)(x + y); }
}

HPX_PLAIN_ACTION(app::compute, compute_action)
HPX_REGISTER_ACTION_DECLARATION(compute_action)
HPX_REGISTER_ACTION(compute_action)
```

Forgetting HPX_REGISTER_ACTION causes silent runtime failures.

### After (single line, zero registration needed)

```cpp
namespace app {
    int compute(double x, double y) { return (int)(x + y); }
}

using compute_action = HPX_ACTION(app::compute);
```

Name extraction, arity, and registration are all automatic via C++26
reflection. The action name "plain action(app::compute)" is derived at
compile time using scope_builder<^^func>.

### Existing macro path (unchanged, still works)

```cpp
// HPX_PLAIN_ACTION still works — internally uses reflect_action<^^func>
// when HPX_HAVE_CXX26_REFLECTION is defined
HPX_PLAIN_ACTION(app::compute, compute_action)
```

## Component Actions

### Before

```cpp
namespace app {
    class server : public hpx::components::component_base<server> {
    public:
        int compute(double x, double y) { return (int)(x + y); }
        HPX_DEFINE_COMPONENT_ACTION(server, compute, compute_action)
    };
}
HPX_REGISTER_ACTION(app::server::compute_action)
```

### After

```cpp
namespace app {
    class server : public hpx::components::component_base<server> {
    public:
        int compute(double x, double y) { return (int)(x + y); }
        HPX_COMPONENT_ACTION(server, compute, compute_action);
    };
}
```

Component type is extracted automatically via parent_of(^^server::compute).

## Remote Invocation (unchanged)

The remote invocation API is identical — only the action definition changes:

```cpp
// Works the same with both old and new action definitions
hpx::id_type remote = hpx::find_remote_localities()[0];
auto result = hpx::async<compute_action>(remote, 3.0, 4.0);
int value = result.get(); // = 7
```

## What Changes Under the Hood

| Feature | Old (macro) | New (reflection) |
|---------|-------------|------------------|
| Action name | manual via HPX_REGISTER_ACTION | automatic via scope_builder<F> |
| Arity | not available | parameters_of(F).size() |
| Registration | HPX_REGISTER_ACTION required | static invocation_count_registrar_ |
| Backwards compat | — | full, macros still work |

## Compiler Notes

- GCC trunk: full support, both macro and HPX_ACTION paths work
- Clang P2996: full support including annotation path ([[=hpx_action]])
- Apple Clang / MSVC: use existing macro path (HPX_WITH_CXX26_REFLECTION=OFF)
