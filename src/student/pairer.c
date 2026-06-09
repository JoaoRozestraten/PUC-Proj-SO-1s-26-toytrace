#include "student_api.h"
#include "syscall_names.h"
#include "trace_helpers.h"
#include <string.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/syscall.h>

char student_last_execve_path[512] = "";
char student_last_execve_args[1024] = "";

static unsigned long read_child_pointer(pid_t pid, unsigned long addr)
{
    errno = 0;
    long word = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);
    if (word == -1 && errno != 0) {
        return 0;
    }
    return (unsigned long)word;
}

int student_pair_syscall(struct syscall_pairer *pairer,
                         const struct syscall_event *ev,
                         struct syscall_event *out)
{
    /*
     * Feito Semana 2:
     *
     * O runtime chama esta funcao duas vezes para cada syscall:
     *
     *   1. uma vez antes da syscall executar
     *   2. uma vez depois da syscall terminar
     *
     * Na primeira parada, os argumentos estao disponiveis.
     * Na segunda parada, o retorno esta disponivel.
     *
     * Seu trabalho e produzir um evento completo apenas quando ja existirem
     * as duas metades da syscall.
     *
     * Dicas:
     * - ev->entering == 1 indica entrada de syscall.
     * - ev->entering == 0 indica saida de syscall.
     * - para comecar, assuma apenas um processo monitorado.
     *
     * Retorne:
     *   1 se out contem uma syscall completa
     *   0 se ainda nao ha syscall completa
     *  -1 se a sequencia de eventos parece invalida
     */
    if (ev->entering == 1) {
        /* Se for exit ou exit_group, pareia imediatamente pois nunca haverá um evento de saída */
        if (ev->syscall_no == SYS_exit_group || ev->syscall_no == SYS_exit) {
            *out = *ev;
            out->entering = 0;
            out->ret = 0;
            return 1;
        }

        /* Verifica se ja tinhamos uma entrada salva sem a respectiva saida (sequencia invalida) */
        if (pairer->has_entry) {
            return -1;
        }
        /* Salva a metade correspondente a entrada da syscall */
        pairer->entry = *ev;
        pairer->has_entry = 1;

        /* Se for execve, lê e salva o caminho e os argumentos a partir da memoria do filho */
        if (ev->syscall_no == SYS_execve) {
            read_child_string(ev->pid, ev->args[0], student_last_execve_path, sizeof(student_last_execve_path));

            char argv_buf[512] = "";
            size_t argv_len = 0;
            argv_len += snprintf(argv_buf + argv_len, sizeof(argv_buf) - argv_len, "[");

            int i = 0;
            while (1) {
                unsigned long arg_ptr = read_child_pointer(ev->pid, ev->args[1] + i * sizeof(void *));
                if (arg_ptr == 0) {
                    break;
                }
                char arg_str[128];
                if (read_child_string(ev->pid, arg_ptr, arg_str, sizeof(arg_str)) < 0) {
                    strncpy(arg_str, "<ilegivel>", sizeof(arg_str));
                }

                if (i > 0) {
                    argv_len += snprintf(argv_buf + argv_len, sizeof(argv_buf) - argv_len, ", ");
                }
                argv_len += snprintf(argv_buf + argv_len, sizeof(argv_buf) - argv_len, "\"%s\"", arg_str);
                i++;
            }
            argv_len += snprintf(argv_buf + argv_len, sizeof(argv_buf) - argv_len, "]");

            int env_vars = 0;
            while (1) {
                unsigned long env_ptr = read_child_pointer(ev->pid, ev->args[2] + env_vars * sizeof(void *));
                if (env_ptr == 0) {
                    break;
                }
                env_vars++;
            }

            snprintf(student_last_execve_args, sizeof(student_last_execve_args),
                     "%s, %#lx /* %d vars */", argv_buf, ev->args[2], env_vars);
        }

        /* Retorna 0 pois ainda nao temos as duas metades */
        return 0;
    } else if (ev->entering == 0) {
        /* Verifica se recebemos a saida sem ter a entrada correspondente (sequencia invalida) */
        if (!pairer->has_entry) {
            return -1;
        }
        /* Copia os dados originais da entrada para o evento de saida completo 'out' */
        *out = pairer->entry;
        /* Atualiza com o valor de retorno obtido no evento de saida */
        out->ret = ev->ret;
        out->entering = 0; /* Para refletir que a chamada foi finalizada */
        
        /* Limpa o estado de entrada para que o pairer possa processar a proxima syscall */
        pairer->has_entry = 0;
        /* Retorna 1 informando que a syscall foi totalmente capturada e 'out' esta pronto */
        return 1;
    }

    /* Qualquer outro caso e invalido */
    return -1;
}
