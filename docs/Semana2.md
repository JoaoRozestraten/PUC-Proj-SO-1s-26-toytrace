# Semana 2

Resolvido o "TODO Semana 2", focado em configurar o rastreamento do processo alvo e parear corretamente a entrada e a saída das chamadas de sistema.

**Implementação 1 (João): `student_pair_syscall`**
Implementada a lógica de agrupamento de chamadas de sistema (syscalls) em `src/student/pairer.c`. As informações da entrada da syscall são salvas no estado do pareador e, ao receber a saída correspondente, as duas partes são combinadas em um único evento completo. (Implementação esperada na semana 5)

**Implementação 2 (Daniel): `launch_tracee`**
Implementada a criação do processo filho a ser monitorado em `src/trace_runtime.c`. A função utiliza `fork()` para criar o novo processo. No processo filho, ela avisa ao kernel que será rastreada (`ptrace(PTRACE_TRACEME)`), suspende a própria execução (`raise(SIGSTOP)`) para o pai sincronizar, e por fim executa o programa desejado via `execvp()`. No processo pai, retorna o PID do filho criado.

**Implementação 3 (Cattuzo): `wait_for_initial_stop`**
Adicionada a lógica no processo pai (em `src/trace_runtime.c`) para esperar o processo filho parar na sua inicialização, garantindo que tudo está pronto para o rastreamento. Para isso, foi utilizada a função `waitpid()`, seguida da verificação `WIFSTOPPED()` que atesta que o filho parou corretamente em estado de *stop* gerado pelo `SIGSTOP`.
