# Semana 4

Resolvido o "TODO Semana 4", focado na extração dos dados dos registradores após uma parada de chamada de sistema (syscall) para preencher os eventos capturados, além da implementação do suporte à depuração de eventos crus.

**Implementação 1 (João): `fill_event_from_regs`**
Lógica em `src/trace_runtime.c` para preencher a estrutura `syscall_event` utilizando os registradores da arquitetura x86_64. Os valores extraídos incluem o número da syscall (`orig_rax`), o valor de retorno (`rax`), e os seis argumentos (armazenados em `rdi`, `rsi`, `rdx`, `r10`, `r8` e `r9`). O campo `entering` também é copiado do parâmetro.

**Implementação 2 (Daniel e Gabriel): PTRACE_GETREGS em `trace_program`**
Adicionada a extração dos valores dos registradores utilizando `ptrace(PTRACE_GETREGS, ...)` no laço principal em `src/trace_runtime.c`. Após a leitura, os dados são enviados à função `fill_event_from_regs()` e em seguida os eventos formatados são notificados por meio da invocação do callback `observer()`.

**Análise (Daniel e Gabriel): Suporte de depuração `student_debug_raw_event`**
Lógica de suporte à depuração implementada no arquivo `src/student/formatter.c`. O objetivo é inspecionar os eventos crus para observar características das syscalls, como nome, tipo de evento (entrada ou saída) e PID (opcionalmente argumentos e retorno). Ela responde à pergunta fundamental da semana:
*Por que a mesma syscall aparece duas vezes?* (Sendo capturada uma vez na entrada da chamada de sistema e outra na sua saída).

Resposta:

A mesma syscall aparece duas vezes porque o mecanismo de rastreamento com ptrace intercepta tanto a entrada quanto a saída da chamada de sistema. Na primeira interceptação, o processo é parado antes da execução da syscall, permitindo observar informações como nome, PID e argumentos. Após a execução pelo kernel, ocorre uma segunda interceptação, correspondente à saída da syscall, na qual é possível analisar o valor de retorno e possíveis erros. Assim, cada syscall gera dois eventos distintos, um no início e outro no término de sua execução.