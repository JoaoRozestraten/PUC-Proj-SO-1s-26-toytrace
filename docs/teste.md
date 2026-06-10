# Testes do Toytrace (Semana 5)

Este documento descreve como compilar, testar e o que esperar de resultado ao rodar o `toytrace` no Linux (Fedora/Ubuntu).

---

## 1. Compilação

Para compilar o projeto completo (incluindo o binário principal `toytrace` e os programas de teste em `tests/targets/`), execute:

```bash
make clean
make
```

Isso gerará:
- O binário `./toytrace` na raiz do projeto.
- Os binários de teste em `./tests/targets/` (ex: `hello_write`, `failed_open`, `open_hosts`, `exec_echo`).

---

## 2. Testes Automatizados

### A. Testes Unitários (C)
Os testes unitários validam a lógica do pareador (`student_pair_syscall`) e a formatação genérica do formatador (`student_format_event`).

**Comando:**
```bash
make test-unit
```
ou executando diretamente o binário de testes:
```bash
./tests/unit/test_student
```

**Resultado esperado:**
```text
testes unitarios concluidos
```

### B. Testes de Integração (Python)
Os testes de integração executam o `toytrace` contra os programas sob `./tests/targets/` e validam se as saídas contêm os padrões esperados (como as chamadas especiais `write` e `execve`).

**Comando:**
```bash
make test-integration
```
ou usando o pytest diretamente:
```bash
python3 -m pytest -v tests/test_integration.py
```

**Resultado esperado:**
Deverá indicar que todos os 3 testes passaram com sucesso (exemplo):
```text
tests/test_integration.py::test_trace_hello_write_contains_write PASSED
tests/test_integration.py::test_raw_events_show_entry_and_exit PASSED
tests/test_integration.py::test_trace_exec_echo_mentions_execve PASSED
```

---

## 3. Testes Manuais e Saídas Esperadas

Para validar a formatação especial da **Semana 5**, você pode executar manualmente o `toytrace` contra os alvos de teste:

### A. Alvo `hello_write`
Este programa escreve uma mensagem no terminal usando a syscall `write`.

**Comando:**
```bash
./toytrace trace -- ./tests/targets/hello_write
```

**Resultado esperado:**
```text
execve("./tests/targets/hello_write", ...) = 0
ola toytrace
write(1, 0x7ffd0000, 13) = 13
exit_group(0) = 0
```
*(Nota: O endereço do buffer hexadecimal, ex: `0x7ffd0000`, varia a cada execução).*

---

### B. Alvo `failed_open`
Este programa tenta abrir um arquivo inexistente, fazendo com que a syscall `openat` falhe e retorne um erro (valor negativo).

**Comando:**
```bash
./toytrace trace -- ./tests/targets/failed_open
```

**Resultado esperado:**
```text
execve("./tests/targets/failed_open", ...) = 0
openat(-100, "/arquivo/que/nao/deve/existir/toytrace", 0x0, 0x0) = -2
exit_group(0) = 0
```
*(Nota: `-100` corresponde à constante `AT_FDCWD` e `-2` é o código de erro para `ENOENT` - No such file or directory).*

---

### C. Alvo `open_hosts`
Este programa abre o arquivo `/etc/hosts` para leitura e lê os primeiros 32 bytes usando `read`.

**Comando:**
```bash
./toytrace trace -- ./tests/targets/open_hosts
```

**Resultado esperado:**
```text
execve("./tests/targets/open_hosts", ...) = 0
openat(-100, "/etc/hosts", 0x0, 0x0) = 3
read(3, 0x7ffd0000, 32) = 32
exit_group(0) = 0
```

---

### D. Alvo `exec_echo`
Este programa executa um novo processo (`/bin/echo`) usando `execve`.

**Comando:**
```bash
./toytrace trace -- ./tests/targets/exec_echo
```

**Resultado esperado:**
```text
execve("/bin/echo", ...) = 0
eco
exit_group(0) = 0
```
