# ***Jellyfish AI Library by Fossil Logic***

Jellyfish v2 is the core reasoning engine behind Truthful Intelligence. It is a lightweight, deterministic AI library written in pure C, designed explicitly for embedded, safety-critical, and trust-sensitive environments. Rather than opaque model behavior, Jellyfish v2 emphasizes traceability, bounded adaptation, and forensic auditability.

At its core, Jellyfish v2 records inputs, outputs, and internal decisions as structured memory blocks linked by cryptographic hashes. This forms an auditable reasoning ledger where every conclusion can be traced back to concrete inputs and rule applications. Learning is strictly memory-based and opt-in, with clear limits on growth, decay, and pruning to prevent drift, corruption, or uncontrolled behavior.

Jellyfish v2 is not a black box. It is an inspectable system intended to be understood, verified, and trusted.

## Key Features

- **Deterministic, Cross-Platform Core**  
  Designed to run predictably on Windows, macOS, Linux, and embedded targets with identical behavior across builds.

- **Pure C, Zero Dependencies**  
  Implemented entirely in standard C to maximize portability, auditability, and long-term maintainability.

- **Hashed Reasoning Ledger**  
  Inputs, outputs, and decisions are stored as linked, cryptographically hashed blocks, enabling tamper-evident inspection and replay.

- **Memory-Based Inference (Not Model Guessing)**  
  Decisions are derived from recorded experience and explicit logic paths, not probabilistic hallucination.

- **Controlled Pruning and Decay**  
  Obsolete, low-confidence, or conflicting memory blocks are pruned using deterministic rules, preserving clarity over time.

- **Forensic-Level Auditability**  
  Every decision can be reconstructed step-by-step, supporting debugging, compliance, and ethical review.

- **Modular v2 Architecture**  
  Designed as a small, embeddable core with optional sub-libraries (such as chat, learning, or storage) that can be enabled or omitted as needed.

## ***Prerequisites***

To get started, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson `1.8.0` or newer installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-ai')` in `meson.build` so Meson can fetch and build it automatically.

#### Integrate the Dependency:

Add the `fossil-ai.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-ai.git
revision = v0.1.4

[provide]
dependency_names = fossil-ai
```

**Note**: For the best experience, always use the latest releases. Visit the [releases](https://github.com/fossillogic/fossil-ai/releases) page for the latest versions.

## Configure Options

Jellyfish offers configurable options to tailor the build process to your needs:

- **Running Tests**: To enable testing, configure the build with `-Dwith_test=enabled`.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

### Tests Double as Samples

The project is designed so that **test cases serve two purposes**:

- ✅ **Unit Tests** – validate the framework’s correctness.  
- 📖 **Usage Samples** – demonstrate how to use these libraries through test cases.  

This approach keeps the codebase compact and avoids redundant “hello world” style examples.  
Instead, the same code that proves correctness also teaches usage.  

This mirrors the **Meson build system** itself, which tests its own functionality by using Meson to test Meson.  
In the same way, Fossil Logic validates itself by demonstrating real-world usage in its own tests via Fossil Test.  

```bash
meson test -C builddir -v
```

Running the test suite gives you both verification and practical examples you can learn from.

## Contributing and Support

For those interested in contributing, reporting issues, or seeking support, please open an issue on the project repository or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information. Your feedback and contributions are always welcome.
