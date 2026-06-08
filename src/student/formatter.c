#include "student_api.h"

#include "syscall_names.h"
#include "trace_helpers.h"

#include <stdio.h>
#include <string.h>

extern char student_last_execve_path[512];
extern char student_last_execve_args[1024];


void student_debug_raw_event(const struct syscall_event *ev,
                             char *buf,
                             size_t bufsz)
{
    /*
     * Suporte de depuracao para a Semana 4:
     *
     * Esta funcao existe para inspecionar eventos crus depois que o runtime
     * ja consegue parar em syscalls e preencher struct syscall_event.
     * Ela nao e a formatacao final do projeto.
     *
     * Experimento sugerido:
     * - imprima o nome da syscall;
     * - imprima se o evento e entrada ou saida;
     * - imprima o pid;
     * - em eventos de entrada, observe os argumentos;
     * - em eventos de saida, observe o valor de retorno.
     *
     * Depois compare a saida de:
     *
     *   ./toytrace trace --raw-events -- ./tests/targets/hello_write
     *
     * A pergunta importante da Semana 4 e:
     * por que a mesma syscall aparece duas vezes?
     */
    snprintf(buf, bufsz, "pid=%d %s %s",
             ev->pid,
             syscall_name(ev->syscall_no),
             ev->entering ? "entrada" : "saida");
}

void student_format_event(const struct syscall_event *ev,
                          char *buf,
                          size_t bufsz)
{
    /*
     * FEITO Semana 5:
     *
     * Primeiro, formate uma syscall completa em uma linha simples.
     *
     * Depois, adicione casos especiais para:
     *     read(fd, buf, count)
     *     write(fd, buf, count)
     *     openat(dirfd, "path", flags, mode)
     *     execve("path", ...)
     *     exit_group(status)
     *
     * Para caminhos do processo monitorado, use read_child_string().
     * Se a leitura falhar, imprima "<ilegivel>".
     */
    const char *name = syscall_name(ev->syscall_no);
    char path_buf[256];

    if (strcmp(name, "read") == 0) {
        snprintf(buf, bufsz, "read(%ld, %#lx, %lu) = %ld",
                 (long)ev->args[0],
                 ev->args[1],
                 (unsigned long)ev->args[2],
                 ev->ret);
        return;
    }

    if (strcmp(name, "write") == 0) {
        snprintf(buf, bufsz, "write(%ld, %#lx, %lu) = %ld",
                 (long)ev->args[0],
                 ev->args[1],
                 (unsigned long)ev->args[2],
                 ev->ret);
        return;
    }

    if (strcmp(name, "openat") == 0) {
        if (read_child_string(ev->pid, ev->args[1], path_buf, sizeof(path_buf)) < 0) {
            snprintf(path_buf, sizeof(path_buf), "<ilegivel>");
        }
        snprintf(buf, bufsz, "openat(%ld, \"%s\", %#lx, %#lx) = %ld",
                 (long)ev->args[0],
                 path_buf,
                 ev->args[2],
                 ev->args[3],
                 ev->ret);
        return;
    }

    if (strcmp(name, "execve") == 0) {
        if (read_child_string(ev->pid, ev->args[0], path_buf, sizeof(path_buf)) < 0) {
            if (student_last_execve_path[0] != '\0') {
                strncpy(path_buf, student_last_execve_path, sizeof(path_buf));
                path_buf[sizeof(path_buf) - 1] = '\0';
            } else {
                snprintf(path_buf, sizeof(path_buf), "<ilegivel>");
            }
        }
        if (student_last_execve_args[0] != '\0') {
            snprintf(buf, bufsz, "execve(\"%s\", %s) = %ld",
                     path_buf,
                     student_last_execve_args,
                     ev->ret);
        } else {
            snprintf(buf, bufsz, "execve(\"%s\", ...) = %ld",
                     path_buf,
                     ev->ret);
        }
        return;
    }

    if (strcmp(name, "exit_group") == 0) {
        snprintf(buf, bufsz, "exit_group(%ld) = %ld",
                 (long)ev->args[0],
                 ev->ret);
        return;
    }

    if (strcmp(name, "close") == 0) {
        snprintf(buf, bufsz, "close(%ld) = %ld",
                 (long)ev->args[0],
                 ev->ret);
        return;
    }

    // Caso genérico
    snprintf(buf, bufsz, "%s(%#lx, %#lx, %#lx, %#lx, %#lx, %#lx) = %ld",
             name,
             ev->args[0],
             ev->args[1],
             ev->args[2],
             ev->args[3],
             ev->args[4],
             ev->args[5],
             ev->ret);
}
