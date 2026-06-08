# Explicação das Alterações no `toytrace`

Este documento explica de forma simples e direta o que foi alterado nos arquivos `pairer.c` e `formatter.c`, por que essas alterações foram necessárias, e como elas resolvem os problemas observados.

---

## 1. O Problema do `execve("<ilegivel>", ...)`

Quando você executava o `toytrace` antes das alterações, o resultado da chamada `execve` vinha com a string do caminho do executável vazia/ilegível, assim:
```text
execve("<ilegivel>", ...) = 0
```

### Por que isso acontecia?
O `ptrace` intercepta as chamadas de sistema (syscalls) em dois momentos:
1. **Entrada da Syscall:** O processo filho solicita o serviço. Os argumentos estão na memória dele e os registradores apontam para esses argumentos.
2. **Saída da Syscall:** O kernel executou o serviço e o retorno está disponível.

O `execve` serve para carregar e rodar um novo executável, **sobrescrevendo completamente o espaço de memória (pilha, heap, código, etc.) do processo filho**.

- Na **entrada** do `execve`, os ponteiros originais apontando para os argumentos (ex: `"/bin/echo"`, `"oi"`) estão na memória antiga do processo e são legíveis.
- Na **saída** (quando o `execve` completa com sucesso), aquela memória antiga **deixou de existir** e foi substituída pela memória do novo programa. 

Se tentarmos ler a string do caminho do executável na saída (que é quando a linha de log final é formatada e impressa), o ponteiro antigo não aponta para nada válido na nova memória, fazendo com que a função `read_child_string` falhasse e retornasse `<ilegivel>`.

### Como resolvemos? (no `pairer.c` e `formatter.c`)
A solução foi cachear (guardar) as informações na **entrada** da syscall, quando a memória antiga ainda está intacta.

1. **No [pairer.c](file:///c:/Users/joaor/Documents/GitArrumar/PUC-Proj-SO-1s-26-toytrace/src/student/pairer.c):**
   Criamos dois buffers globais na memória do `toytrace` (o pai):
   - `student_last_execve_path`: Para guardar o caminho do executável (ex: `"/bin/echo"`).
   - `student_last_execve_args`: Para guardar a formatação dos argumentos (ex: `["/bin/echo", "oi"], 0x7ffd... /* 33 vars */`).

   Quando detectamos a entrada de um `execve` (`ev->entering == 1`):
   - Lemos o caminho com `read_child_string`.
   - Lemos a lista de argumentos (`argv`) e o ambiente (`envp`).
   
   **Como lemos o `argv` e `envp` do filho?**
   O `argv` e o `envp` no Linux são arrays de ponteiros. Criamos uma função auxiliar `read_child_pointer()` que usa `ptrace(PTRACE_PEEKDATA)` para ler esses ponteiros de 8 bytes diretamente da memória do filho:
   - Lemos os ponteiros do array `argv` um por um. Para cada ponteiro válido (diferente de zero), usamos `read_child_string()` para puxar a string em si e a adicionamos formatada em uma string (ex: `["/bin/echo", "oi"]`).
   - Lemos os ponteiros do array `envp` para contar quantas variáveis de ambiente existem.
   - Guardamos tudo formatado no buffer `student_last_execve_args`.

2. **No [formatter.c](file:///c:/Users/joaor/Documents/GitArrumar/PUC-Proj-SO-1s-26-toytrace/src/student/formatter.c):**
   Ao formatar a syscall `execve` (na saída), tentamos ler o caminho da memória. Se falhar (o que sempre acontece no `execve` de sucesso), usamos o caminho guardado no buffer de cache. Além disso, se o cache de argumentos estiver preenchido, imprimimos os argumentos completos e a contagem de variáveis em vez de apenas `...`.

---

## 2. A Syscall `exit_group` (e `exit`) Nunca Imprimia

### Por que isso acontecia?
O pareador original (`student_pair_syscall`) só retornava `1` (sinalizando que o evento está pronto para ser impresso) quando recebia a **saída** da syscall (`ev->entering == 0`).

Porém, as chamadas `exit` e `exit_group` servem justamente para finalizar e destruir o processo!
Logo:
- O processo entra na syscall `exit_group`.
- O `toytrace` intercepta o evento de **entrada**.
- O processo é finalizado pelo kernel e deixa de existir.
- O evento de **saída** nunca ocorre!

Como a saída nunca ocorria, a chamada `exit_group` nunca era pareada, nunca era formatada e nunca era impressa. O `toytrace` simplesmente encerrava sem mostrar essa linha.

### Como resolvemos? (no `pairer.c`)
No início da função [student_pair_syscall](file:///c:/Users/joaor/Documents/GitArrumar/PUC-Proj-SO-1s-26-toytrace/src/student/pairer.c), adicionamos uma verificação:
```c
if (strcmp(syscall_name(ev->syscall_no), "exit_group") == 0 ||
    strcmp(syscall_name(ev->syscall_no), "exit") == 0) {
    *out = *ev;
    out->entering = 0; // Fingimos que é a saída para o formatador aceitar
    out->ret = 0;      // exit não retorna, definimos como 0
    return 1;          // Retornamos 1 imediatamente para forçar a impressão!
}
```
Com isso, assim que o `exit_group` inicia (no evento de entrada), nós já simulamos o par completo dele. Ele é formatado e impresso logo antes do processo filho morrer.

---

## 3. O Formato Genérico do `close`

### Por que isso acontecia?
Toda syscall que não possui um tratamento especial no formatador é impressa usando um modelo padrão (genérico) que exibe todos os 6 registradores de argumentos em hexadecimal:
```text
close(0x3, 0x5ccf, 0x7f60af42b000, 0x2, 0x3, 0) = 0
```
Isso acontecia porque `close` não estava na lista de tratamentos específicos em `formatter.c`.

### Como resolvemos? (no `formatter.c`)
Adicionamos uma condição explícita para o `close` em [student_format_event](file:///c:/Users/joaor/Documents/GitArrumar/PUC-Proj-SO-1s-26-toytrace/src/student/formatter.c):
```c
if (strcmp(name, "close") == 0) {
    snprintf(buf, bufsz, "close(%ld) = %ld",
             (long)ev->args[0],
             ev->ret);
    return;
}
```
Agora, o formatador extrai apenas o primeiro argumento (o descritor do arquivo `fd`) e descarta os outros registradores irrelevantes, produzindo uma saída limpa e idêntica ao `strace`:
```text
close(3) = 0
```
