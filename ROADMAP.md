```md
# miniC Compiler Project Roadmap

## Project goal

Build a small but serious **C-like compiler/interpreter in C**, slowly and carefully, to learn compiler construction step by step.

The project will grow in stages:

1. build a solid front end
2. build semantic analysis and interpreter
3. define a stable language subset
4. introduce IR
5. add simple optimizations
6. generate assembly
7. gradually expand toward a more complete C-like compiler

This project is **not trying to implement full ISO C immediately**.  
The goal is to build a correct and understandable compiler pipeline for a meaningful subset first.

---

## Current status recap

### Compiler pipeline implemented
- [x] Tokenizer
- [x] Parser
- [x] Semantic analysis
- [x] Evaluation / interpreter

### AST node kinds in use
- [x] `AST_PROGRAM`
- [x] `AST_BLOCK`
- [x] `AST_DECLARATION`
- [x] `AST_ASSIGNMENT`
- [x] `AST_PRINT`
- [x] `AST_IF`
- [x] `AST_WHILE`
- [x] `AST_IDENTIFIER`
- [x] `AST_NUMBER`
- [x] `AST_UNARY`
- [x] `AST_BINARY`

### Type system currently
- [x] `TYPE_INT`
- [x] `TYPE_FLOAT`
- [x] `TYPE_UNKNOWN`
- [x] `TYPE_UNUSED`
- [x] `TYPE_ERROR`

### Statements currently supported
- [x] variable declaration
- [x] variable declaration with initializer
- [x] assignment
- [x] print statement
- [x] block
- [x] `if`
- [x] `if / else`
- [x] `while`

### Expressions currently supported
- [x] integer literals
- [x] float literals
- [x] identifiers
- [x] unary `+`
- [x] unary `-`
- [x] binary `+`
- [x] binary `-`
- [x] binary `*`
- [x] binary `/`
- [x] comparison `<`
- [x] comparison `>`
- [x] equality `==`
- [x] inequality `!=`
- [x] parentheses

### Semantic analysis currently supports
- [x] declaration identifier validity
- [x] redeclaration rejection in same scope
- [x] identifier lookup through nested scopes
- [x] assignment LHS must be identifier
- [x] assignment target must be declared
- [x] initializer expression analysis
- [x] RHS expression analysis
- [x] type checking for expressions
- [x] print requires expression
- [x] `if` condition must be numeric
- [x] `while` condition must be numeric
- [x] AST nodes get `type_kind` annotation

### Evaluation currently supports
- [x] declarations with default initialization
- [x] declarations with initializer
- [x] assignment
- [x] print
- [x] expression evaluation
- [x] identifier lookup in runtime symbol table
- [x] `if / else`
- [x] `while`
- [x] division by zero runtime error
- [x] block scopes and shadowing
- [x] separate semantic and runtime symbol tables

### Debugging / visibility
- [x] AST printer shows node kind
- [x] AST printer shows operators
- [x] AST printer shows type information

---

## Current miniC behavior choices

### Default initialization rule
For simplicity in miniC:

```c
int x;
```

stores `x = 0`.

Likewise:
- `float x;` stores `x = 0.0`

This is a deliberate miniC simplification for now.

### Condition truthiness
Numeric truthiness is used:

- `0` / `0.0` => false
- nonzero int/float => true

This applies to:
- `if`
- `while`

### else-if support
`else if (...)` already works naturally because `else` accepts a statement and `if (...) ...` is itself a statement.

---

## Project philosophy

Development will continue in a **slow, incremental, beginner-friendly way**.

For each new feature:

1. tokenizer support
2. parser support
3. AST integration
4. semantic analysis
5. evaluation / execution
6. tests
7. AST/debug output review

This is a good learning method and should continue.

---

## Important long-term strategy

A compiler can be built incrementally feature by feature, but to reach a fuller compiler, development must also be organized into larger **phases**.

So the project should balance:

- small local steps for learning
- larger roadmap phases for long-term direction

### Important rule
Do **not** keep adding syntax forever without eventually freezing a language subset and moving to IR/code generation.

A better goal is:

> Build a small but complete compiler pipeline for a clear subset.

---

## Development roadmap

---

### Phase 1: Finish the core language

Goal: complete a solid structured miniC front end + interpreter.

#### Expressions to add
- [ ] logical AND `&&`
- [ ] logical OR `||`
- [ ] logical NOT `!`
- [ ] `<=`
- [ ] `>=`
- [ ] modulus `%` for integers only
- [ ] maybe explicit casts later

#### Statements to add
- [ ] `break`
- [ ] `continue`
- [ ] `for`
- [ ] `return`

#### General improvements
- [ ] improve tests
- [ ] improve diagnostics / error messages
- [ ] keep AST/type annotations consistent
- [ ] keep scope behavior clean
- [ ] strengthen parser error handling gradually

#### Recommended near-term order
1. [ ] `&&`
2. [ ] `||`
3. [ ] `!`
4. [ ] `<=`
5. [ ] `>=`
6. [ ] `break`
7. [ ] `continue`
8. [ ] `for`

---

### Phase 2: Add functions

Goal: move from statement scripts to real programs.

#### Features
- [ ] function declarations
- [ ] function definitions
- [ ] parameter lists
- [ ] function calls
- [ ] argument checking
- [ ] return statements
- [ ] return type checking
- [ ] parameter/local scopes
- [ ] function symbol table
- [ ] distinguish variables from functions in symbol handling

---

### Phase 3: Expand the type system carefully

Goal: become more C-like, but slowly and deliberately.

#### Candidate additions
- [ ] `void`
- [ ] `char`
- [ ] pointers
- [ ] address-of `&`
- [ ] dereference `*`
- [ ] arrays
- [ ] string literals
- [ ] maybe structs later

### Note
This phase is where complexity rises sharply. Do not rush it.

---

### Phase 4: Freeze a supported language subset

Goal: stop adding syntax for a while and stabilize the compiler design.

Before IR is added, clearly define:

- what syntax is supported
- what types are supported
- what statements are supported
- what conversions are supported
- what is intentionally not supported yet

#### Example miniC v1 subset
- scalar types: `int`, `float`, maybe `char`
- variables and blocks
- `if`, `while`, `for`
- functions and returns
- arithmetic/comparison/logical expressions
- maybe pointers/arrays if already stable
- no preprocessor
- no macros
- no header system
- no full ISO C compatibility promise

This step is important because otherwise the project can stay in front-end expansion forever.

---

### Phase 5: Introduce IR

Goal: separate front end from execution and prepare for optimization/code generation.

#### Add a new compiler layer
- [ ] lower AST -> IR
- [ ] add IR printer/debugger
- [ ] define IR instructions and structure
- [ ] represent control flow explicitly

#### Recommended IR style
Use a simple three-address-code-like IR with things such as:

- temporaries
- labels
- jumps
- conditional branches
- binary operations
- unary operations
- loads/stores
- calls
- returns

#### Important idea
Keep both for a while:
- AST interpreter
- IR generator

This allows front-end behavior to be compared with lower-level forms.

---

### Phase 6: Add simple optimizations

Goal: learn middle-end basics using the IR.

#### First optimizations
- [ ] constant folding
- [ ] constant propagation
- [ ] dead code elimination
- [ ] unreachable code removal
- [ ] simple algebraic simplifications

#### Maybe later
- [ ] copy propagation
- [ ] common subexpression elimination
- [ ] strength reduction

Do not begin with advanced optimizations.

---

### Phase 7: Add backends

Goal: compile programs into assembly, not just interpret them.

Desired backends:
- [ ] x86-64 assembly backend
- [ ] RISC-V assembly backend

#### Recommended order
1. [ ] x86-64 first
2. [ ] RISC-V second

Reason:
- one backend is enough to validate the full pipeline
- the second backend becomes easier once IR and codegen structure are stable

#### Backend tasks
- [ ] stack frame design
- [ ] local variable layout
- [ ] function call convention support
- [ ] expression lowering
- [ ] branch lowering
- [ ] return value handling
- [ ] assembly emission
- [ ] external assembly + linking workflow

---

### Phase 8: Improve correctness and completeness

Goal: make the compiler more robust and maintainable.

#### Improvements
- [ ] better diagnostics
- [ ] clearer internal invariants
- [ ] more test programs
- [ ] regression tests
- [ ] command-line driver cleanup
- [ ] file input support
- [ ] debug flags
- [ ] AST dump mode
- [ ] IR dump mode
- [ ] cleaner module structure
- [ ] more complete language support only after the main pipeline is stable

---

## Backend testing plan

You want both **x86-64 assembly** and **RISC-V assembly** and to be able to test both.

### x86-64 testing
On a normal x86-64 Linux machine, later workflow can be:

1. miniC compiler outputs `out.s`
2. system compiler/assembler links it:
   ```bash
   gcc out.s -o out
   ```
3. run it:
   ```bash
   ./out
   ```

This makes x86-64 the easiest first native backend to test.

### RISC-V testing
If the host machine is not RISC-V, testing can still be done with:

- a RISC-V cross-toolchain
- an emulator such as QEMU

Possible later workflow:

1. miniC compiler outputs RISC-V assembly
2. assemble/link with a RISC-V cross-toolchain
3. run the result under a RISC-V emulator

So yes, RISC-V is still practical even without native hardware.

### Practical advice
Do **not** start both backends at once.

Best progression:
1. make IR solid
2. make one backend work fully
3. add the second backend

---

## Recommended overall implementation order

### Near term
1. [ ] logical operators
2. [ ] `<=`, `>=`
3. [ ] `break`, `continue`
4. [ ] `for`

### Then
5. [ ] functions
6. [ ] returns and calls

### Then
7. [ ] type system expansion
8. [ ] freeze subset

### Then
9. [ ] IR
10. [ ] IR printer/debugging
11. [ ] simple optimizations

### Then
12. [ ] x86-64 backend
13. [ ] RISC-V backend

---

## Realistic expectations

### Can a full compiler be built this way?
Yes — incrementally.

The current approach is valid and good for learning:
- add one feature
- wire it through all phases
- test it carefully

### What changes later?
Later, progress must shift from:
- feature-by-feature only

to:
- compiler-layer milestones

That means:
- front end
- semantic analysis
- IR
- optimization
- backend

### How long will it take?
For a beginner moving slowly, a serious C-like compiler with:
- front end
- semantic analysis
- interpreter
- IR
- simple optimizations
- x86-64 backend
- RISC-V backend

is a **many-month project**, possibly much longer depending on pace.

That is normal.

The goal is not speed.  
The goal is a correct and understandable pipeline.

---

## Current milestone

- [x] `while` support completed

## Immediate next milestone

- [ ] logical operators

Recommended sub-order:
1. [ ] `&&`
2. [ ] `||`
3. [ ] `!`

---

## Short checklist

### Already done
- [x] tokenizer
- [x] parser
- [x] semantic analysis
- [x] evaluator
- [x] scopes
- [x] int/float
- [x] blocks
- [x] `if / else`
- [x] `while`

### Next
- [ ] `&&`
- [ ] `||`
- [ ] `!`
- [ ] `<=`
- [ ] `>=`
- [ ] `break`
- [ ] `continue`
- [ ] `for`
- [ ] functions
- [ ] returns
- [ ] function calls
- [ ] freeze subset
- [ ] IR
- [ ] optimizations
- [ ] x86-64 backend
- [ ] RISC-V backend
```
