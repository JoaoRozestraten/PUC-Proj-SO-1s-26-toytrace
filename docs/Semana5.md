# Semana 5

Resolvido o "TODO Semana 5", focado em formatar as chamadas de sistema de forma amigável e legível para o usuário, implementando a formatação genérica de syscalls e tratamentos especiais para syscalls específicas, incluindo a leitura de strings na memória do processo filho.

**Implementação 1 (João, Daniel e Gabriel): `student_format_event`**
Implementada a formatação de eventos de syscall no arquivo `src/student/formatter.c`. A função realiza a formatação genérica padrão para todas as syscalls e implementa exibições especializadas para `read`, `write`, `openat`, `execve` e `exit_group`. Ela utiliza `read_child_string()` para obter de forma segura as strings de caminho das syscalls `openat` e `execve` diretamente do espaço de memória do processo filho, tratando falhas de leitura com a exibição de `"<ilegivel>"`.
