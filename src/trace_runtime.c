#include "trace_runtime.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#if !defined(__x86_64__)
#error "Este runtime didatico suporta apenas Linux x86_64."
#endif

static void fill_event_from_regs(pid_t pid,
                                 int entering,
                                 const struct user_regs_struct *regs,
                                 struct syscall_event *ev)
{
    /*
     * FEITO Semana 4:
     *
     * Preencha struct syscall_event usando os registradores x86_64.
     *
     * Dicas:
     * - regs->orig_rax contem o numero da syscall.
     * - regs->rax contem o retorno, valido na saida.
     * - os seis argumentos ficam em rdi, rsi, rdx, r10, r8 e r9.
     * - ev->entering deve copiar o parametro entering.
     */
    memset(ev, 0, sizeof(*ev));
    ev->pid = pid;
    ev->entering = entering;
    ev->syscall_no = regs->orig_rax;
    ev->ret = regs->rax;
    ev->args[0] = regs->rdi;
    ev->args[1] = regs->rsi;
    ev->args[2] = regs->rdx;
    ev->args[3] = regs->r10;
    ev->args[4] = regs->r8;
    ev->args[5] = regs->r9;
}

static pid_t launch_tracee(char *const argv[])
{
    /*
     * TODO Semana 2:
     *
     * Crie o processo monitorado.
     *
     * Fluxo esperado:
     * - fork()
     * - no filho:
     *   - ptrace(PTRACE_TRACEME, ...)
     *   - raise(SIGSTOP)
     *   - execvp(argv[0], argv)
     * - no pai:
     *   - retornar o pid do filho
     *
     * Em erro, imprima uma mensagem com perror() e retorne -1.
     */
    pid_t pid = fork();
    // Entra na condicional se o processo for o filho
    if (pid == 0) {
        // Filho notifica ao kernel que será rastreado pelo pai
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("ptrace TRACEME");
            _exit(1);
        }
        // Para imediatamente para o pai fazer o waitpid inicial
        if (raise(SIGSTOP) != 0) {
            perror("raise SIGSTOP");
            _exit(1);
        }
        // Executa o programa requisitado
        execvp(argv[0], argv);
        // Se o execvp falhar, exibe o erro
        perror("execvp falhou!");
        _exit(1);
    }else if (pid > 0) {
        return pid;
    }else
        perror("Erro no fork");
    return -1;
}

static int wait_for_initial_stop(pid_t child)
{
    /*
     * TODO Semana 2:
     *
     * O filho chama raise(SIGSTOP) antes de executar o programa alvo.
     * O pai precisa esperar essa parada inicial com waitpid().
     *
     * Retorne 0 se o filho parou como esperado, -1 em erro.
     */
    int status;
    // Pai espera o filho para em SIGSTOP
    if (waitpid(child, &status, 0) == -1) 
    {
        perror("Erro na execução do waitpid");
        return -1;
    }
    // Verifica se realmente parou
    if (!WIFSTOPPED(status)) 
    {
        fprintf(stderr, "Filho não parou corretamente\n");
        return -1;
    }
    return 0;
}

static int configure_trace_options(pid_t child)
{
    /*
     * FEITO Semana 3:
     *
     * Configure PTRACE_O_TRACESYSGOOD com PTRACE_SETOPTIONS.
     * Isso ajuda a diferenciar paradas de syscall de outros sinais.
     */
    // Configura opções do ptrace para diferenciar paradas de syscall de outros sinais
    if (ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD) == -1) {
        perror("ptrace SETOPTIONS falhou");
        return -1;
    }
    return 0;
}

static int resume_until_next_syscall(pid_t child, int signal_to_deliver)
{
    /*
     * Feito Semana 3:
     *
     * Usa PTRACE_SYSCALL para continuar a execução do filho
     * até a próxima entrada ou saída de syscall.
     *
     * signal_to_deliver é repassado ao processo filho.
     */

    if (ptrace(PTRACE_SYSCALL, child, NULL, signal_to_deliver) == -1) 
    {
        perror("ptrace SYSCALL falhou");
        return -1;
    }

    return 0;
}

static int wait_for_syscall_stop(pid_t child, int *status)
{
    /*
     * FEITO Semana 3:
     *
     * Espere o filho com waitpid().
     *
     * Retorne:
     *   1 se a parada foi uma parada de syscall;
     *   0 se o filho terminou normalmente ou por sinal;
     *  -1 em erro.
     *
     * Dicas:
     * - WIFEXITED e WIFSIGNALED indicam fim do processo.
     * - WIFSTOPPED indica que o processo parou.
     * - com PTRACE_O_TRACESYSGOOD, syscall-stops aparecem com bit 0x80.
     * - paradas SIGTRAP comuns nao devem ser entregues de volta ao filho.
     */
    int sig;
    while (1) {
        if (waitpid(child, status, 0) == -1) {
            perror("waitpid");
            return -1;
        }

        if (WIFEXITED(*status) || WIFSIGNALED(*status))
            return 0;

        if (WIFSTOPPED(*status)) {
            if (WSTOPSIG(*status) & 0x80)
                return 1;

            sig = WSTOPSIG(*status);
            if (sig == SIGTRAP)
                sig = 0;

            if (resume_until_next_syscall(child, sig) == -1)
                return -1;
        } else {
            return -1;
        }
    }
}

int trace_program(char *const argv[],
                  trace_observer_fn observer,
                  void *userdata)
{
    pid_t child;
    int status = 0;
    int entering = 1;

    if (argv == NULL || argv[0] == NULL) {
        fprintf(stderr, "erro: programa alvo ausente\n");
        return -1;
    }

    child = launch_tracee(argv);
    if (child < 0) {
        return -1;
    }

    if (wait_for_initial_stop(child) < 0) {
        return -1;
    }

    if (configure_trace_options(child) < 0) {
        return -1;
    }

    if (resume_until_next_syscall(child, 0) < 0) {
        return -1;
    }

    while (1) {
        struct user_regs_struct regs;
        struct syscall_event ev;
        int stop_kind;

        stop_kind = wait_for_syscall_stop(child, &status);
        if (stop_kind < 0) {
            return -1;
        }
        if (stop_kind == 0) {
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }
            return 0;
        }

        /*
         * Feito Semana 4:
         *
         * Use PTRACE_GETREGS para preencher regs.
         * Depois chame fill_event_from_regs() e observer().
         */
        
        memset(&regs, 0, sizeof(regs)); /* Inicializa os registradores */

        if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1) /* PTRACE_GETREGS */
        {
                perror("ptrace GETREGS falhou");
                return -1;
        }

        fill_event_from_regs(child, entering, &regs, &ev); /* chama fill e observer */
        if (observer != NULL) 
        {
            observer(&ev, userdata);
        }

        entering = !entering;

        if (resume_until_next_syscall(child, 0) < 0) 
        {
            return -1;
        }
    }
}
