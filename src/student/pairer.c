#include "student_api.h"

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
        /* Verifica se ja tinhamos uma entrada salva sem a respectiva saida (sequencia invalida) */
        if (pairer->has_entry) {
            return -1;
        }
        /* Salva a metade correspondente a entrada da syscall */
        pairer->entry = *ev;
        pairer->has_entry = 1;
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
