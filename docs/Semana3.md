# Semana 3

Resolvido o "TODO Semana 3", focado em configurar o `ptrace` para facilitar a detecção de chamadas de sistema e implementar o controle de execução do processo alvo, parando-o a cada syscall.

**Implementação 1 (João): `configure_trace_options`**
Implementada a configuração de opções do rastreamento no arquivo `src/trace_runtime.c`. A opção `PTRACE_O_TRACESYSGOOD` foi ativada por meio de `ptrace(PTRACE_SETOPTIONS)`. Isso garante que o kernel adicione o bit 0x80 ao sinal `SIGTRAP` quando a parada for causada por uma chamada de sistema, permitindo diferenciá-la facilmente de paradas e sinais comuns.

**Implementação 2 (NOME): `resume_until_next_syscall`**
*(A ser implementado)* Lógica que permitirá prosseguir a execução do processo monitorado utilizando `ptrace(PTRACE_SYSCALL, ...)`. O objetivo é deixar o processo filho executar livremente até encontrar a próxima entrada ou saída de uma chamada de sistema, repassando eventuais sinais pendentes.

**Implementação 3 (Daniel): `wait_for_syscall_stop`**
Lógica de sincronização no processo pai para aguardar o filho parar em uma chamada de sistema, utilizando `waitpid()`. A função deverá analisar o status retornado com macros como `WIFEXITED`, `WIFSIGNALED` e `WIFSTOPPED`, retornando estados específicos para informar se o processo terminou normalmente, por erro ou se realizou uma parada legítima de syscall (verificando o bit 0x80 do `TRACESYSGOOD`).
