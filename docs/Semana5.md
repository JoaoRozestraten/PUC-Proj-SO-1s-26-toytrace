# Semana 5

Resolvido o “TODO Semana 5”, focado na formatação textual dos eventos de syscalls capturados pelo tracer no arquivo `src/student/formatter.c`. A implementação da função `student_format_event()` permitiu converter os dados da estrutura `syscall_event` em uma saída legível, semelhante ao formato utilizado pelo `strace`.

**Implementação (Daniel e Gabriel): `student_format_event`**  
Foi implementada inicialmente uma formatação genérica para imprimir o nome da syscall, seus seis argumentos em hexadecimal e o valor de retorno. Em seguida, foram adicionados casos especiais obrigatórios para `read`, `write`, `openat`, `execve` e `exit_group`, melhorando a legibilidade das chamadas mais importantes.

Nas syscalls `openat` e `execve`, foram utilizadas funções para leitura de strings diretamente da memória do processo monitorado. Em `openat`, `read_child_string()` recupera o caminho do arquivo e, em caso de falha, imprime `"<ilegivel>"`. Já em `execve`, o buffer é inicializado com `"<ilegivel>"` como valor padrão, e `read_child_string()` tenta sobrescrever com o caminho real. Caso a memória do processo já tenha sido substituída pelo execve, o valor padrão é mantido.