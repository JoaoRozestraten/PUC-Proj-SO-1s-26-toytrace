# Mapa do Código - Toytrace

Este documento tem como objetivo registrar o entendimento inicial sobre a estrutura do projeto, finalizando a exploração da Semana 1.

*   **Onde o programa começa?**
    O programa começa no arquivo `src/main.c`, dentro da função `main()`. É lá que a CLI é processada e a execução da lógica de tracing é iniciada por meio da chamada à função `trace_program()`.

*   **Onde o processo alvo é criado?**
    A criação do processo a ser monitorado (tracee) acontece no arquivo `src/trace_runtime.c`, especificamente dentro da função `launch_tracee()`.

*   **Onde o runtime chama o callback?**
    O callback que notifica os eventos capturados é invocado no arquivo `src/trace_runtime.c`, no laço infinito da função `trace_program()`. A chamada é feita via ponteiro de função (`observer(&ev, userdata);`) logo após preencher as informações dos registradores.

*   **Quais arquivos o grupo deve modificar?**
    Durante as próximas semanas, o grupo deve modificar os seguintes arquivos:
    1.  `src/trace_runtime.c`: para implementar a infraestrutura básica do ptrace e resolver os *TODOs* indicados. (SEMANAS 2 3 e 4)
    2.  `src/student/pairer.c`: para realizar o pareamento entre o evento de entrada e o de saída de uma mesma syscall. (SEMANA 2)
    3.  `src/student/formatter.c`: para fazer a formatação e impressão legível dSSas syscalls capturadas. (SEMANAS 4 e 5)

*   **Qual TODO aparece primeiro ao executar o scaffold?**
    Ao rodar o comando `./toytrace trace -- ./tests/targets/hello_write`, o programa falha e imprime a seguinte mensagem proveniente da função `launch_tracee()`:
    `erro: TODO Semana 2: implementar launch_tracee()`

*   **Qual é a principal dúvida técnica do grupo neste momento?**
    A principal dúvida do grupo neste momento é entender como o `ptrace` deve ser utilizado na função `launch_tracee()`, especialmente a sequência correta de chamadas (`fork`, `ptrace`, `exec`) e como o processo filho passa a ser monitorado pelo processo pai.