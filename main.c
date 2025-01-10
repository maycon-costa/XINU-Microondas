#include <xinu.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum { INATIVO, RODANDO, PAUSADO, RESFRIANDO } Estado;

typedef struct {
    char *nome;
    int duracao;   
    int potencia;  
    int curvaTemp; 
} CicloAquecimento;

struct Microondas {
    Estado estado;
    int tempoRestante;
    bool portaAberta;
    bool emergenciaAtivada;
    bool agendamentoAtivo;
    int tempoAgendado; 
    CicloAquecimento cicloAtual;
    sid32 mutex; 
};

CicloAquecimento ciclos[] = {
    {"Carnes", 600, 100, 1},
    {"Peixe", 450, 80, 2},
    {"Frango", 500, 90, 1},
    {"Lasanha", 700, 100, 2},
    {"Pipoca", 300, 70, 1}
};

void controle_klystron(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->estado == RODANDO) {
            if (microondas->cicloAtual.curvaTemp == 1) {
                kprintf("Klystron: Potencia constante em %d%%\n", microondas->cicloAtual.potencia);
            } else if (microondas->cicloAtual.curvaTemp == 2) {
                kprintf("Klystron: Potencia ajustada (curva exponencial)\n");
            }
        } else if (microondas->estado == RESFRIANDO) {
            kprintf("Klystron: Ciclo de resfriamento ativo.\n");
        }
        signal(microondas->mutex);
        sleep(1); 
    }
}
void ciclo_funcionamento(struct Microondas *microondas){
    while (microondas->tempoRestante > 0){
        wait(microondas->mutex);    
        microondas->tempoRestante--;
        kprintf("Tempo Restante: %d segundos!\n", microondas->tempoRestante);
        if (microondas->tempoRestante == 0) {
            microondas->estado = INATIVO; 
        }
        signal(microondas->mutex);
        sleep(1);
    }
}

void anunciador_bip(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->estado == INATIVO && microondas->tempoRestante == 0) {
            kprintf("Beep! Ciclo concluído!\n");
            microondas->estado = INATIVO;
        } else if (microondas->emergenciaAtivada) {
            kprintf("Beep! Emergência ativada!\n");
            microondas->emergenciaAtivada = false;
        }
        signal(microondas->mutex);
        sleep(1);
    }
}

void botao_emergencia(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->emergenciaAtivada) {
            microondas->estado = INATIVO;
            microondas->tempoRestante = 0;
            kprintf("Emergencia ativada! Ciclo cancelado.\n");
            microondas->emergenciaAtivada = false;
        }
        signal(microondas->mutex);
        sleepms(100);
    }
}

void resfriamento(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->estado == RESFRIANDO) {
            kprintf("Ventilação ativada para resfriamento.\n");
            sleep(5);
            microondas->estado = INATIVO;
            kprintf("Micro-ondas resfriado e pronto.\n");
        }
        signal(microondas->mutex);
        sleep(1);
    }
}

void programacao_futura(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->agendamentoAtivo && microondas->tempoAgendado > 0) {
            microondas->tempoAgendado--;
            if (microondas->tempoAgendado == 0) {
                microondas->estado = RODANDO;
                microondas->agendamentoAtivo = false;
                kprintf("Ciclo agendado iniciado: %s\n", microondas->cicloAtual.nome);
            }
        }
        signal(microondas->mutex);
        sleep(1);
    }
}



void relogio_cortesia() {
    while (1) {
        wait(mutex);
        uint32_t agora = clktime; 
        uint32_t horas = (agora / 3600) % 24;
        uint32_t minutos = (agora / 60) % 60;
        uint32_t segundos = agora % 60;
        
        kprintf("Relogio cortesia: %02d:%02d:%02d\n", horas, minutos, segundos);
        signal(mutex);
        
        sleep(5);
    }
}

void tracao_prato(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->estado == RODANDO) {
            kprintf("Prato girando...\n");
        } else if (microondas->estado == INATIVO || microondas->estado == RESFRIANDO) {
            kprintf("Prato parado.\n");
        }
        signal(microondas->mutex);
        sleep(2); 
    }
}

void luz_interna(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->portaAberta || microondas->estado == RODANDO) {
            kprintf("Luz interna: LIGADA\n");
        } else {
            kprintf("Luz interna: DESLIGADA\n");
        }
        signal(microondas->mutex);
        sleep(1); 
    }
}

void ligacao_automatica(struct Microondas *microondas) {
    while (1) {
        wait(microondas->mutex);
        if (microondas->estado == PAUSADO && microondas->tempoRestante > 0) {
            kprintf("Ligacao automatica em 5 segundos...\n");
            sleep(5);
            microondas->estado = RODANDO;
            kprintf("Ligacao automatica: Micro-ondas reiniciado.\n");
        }
        signal(microondas->mutex);
        sleep(1);
    }
}

void main(void) {

    
    struct Microondas microondas;
    microondas.estado = INATIVO;
    microondas.tempoRestante = 0;
    microondas.portaAberta = false;
    microondas.emergenciaAtivada = false;
    microondas.agendamentoAtivo = false;
    microondas.mutex = semcreate(1);

    resume(create(controle_klystron, 1024, 20, "Klystron", 1, &microondas));
    resume(create(anunciador_bip, 1024, 20, "Bip", 1, &microondas));
    resume(create(botao_emergencia, 1024, 20, "Emergencia", 1, &microondas));
    resume(create(resfriamento, 1024, 20, "Resfriamento", 1, &microondas));
    resume(create(programacao_futura, 1024, 20, "Agendamento", 1, &microondas));
   // resume(create(relogio_cortesia, 1024, 20, "Relogio", 0));
    resume(create(tracao_prato, 1024, 20, "TracaoPrato", 1, &microondas));
    resume(create(luz_interna, 1024, 20, "LuzInterna", 1, &microondas));
    resume(create(ligacao_automatica, 1024, 20, "LigacaoAutomatica", 1, &microondas));

    while (1) {
        kprintf("\nMenu:\n");
        kprintf("1. Abrir porta\n");
        kprintf("2. Fechar porta\n");
        kprintf("3. Selecionar ciclo\n");
        kprintf("4. Iniciar\n");
        kprintf("5. Programar inicio futuro\n");
        kprintf("6. Acionar emergencia\n");
        kprintf("7. Sair\n");

        int opcao;
        char buffer[10];
        read(CONSOLE, buffer, sizeof(buffer));
        opcao = atoi(buffer);
        kprintf("Opcao escolhida foi %d\n", opcao);

        wait(microondas.mutex);
        switch (opcao) {
            case 1:
                microondas.portaAberta = true;
                kprintf("Porta aberta.\n");
                break;
            case 2:
                microondas.portaAberta = false;
                kprintf("Porta fechada.\n");
                break;
            case 3: {
                kprintf("Selecione o ciclo (0-Carnes, 1-Peixe, ...): ");

                int escolha;
                
                char buffer[10];
                read(CONSOLE, buffer, sizeof(buffer));
                escolha = atoi(buffer);

                if (escolha >= 0 && escolha < 5) {
                    microondas.cicloAtual = ciclos[escolha];
                    microondas.tempoRestante = ciclos[escolha].duracao;
                    kprintf("Ciclo '%s' selecionado.\n", ciclos[escolha].nome);
                } else {
                    kprintf("Ciclo invalido.\n");
                }
                break;
            }
            case 4:
                if (!microondas.portaAberta && microondas.tempoRestante > 0) {
                    microondas.estado = RODANDO;
                    kprintf("Ciclo iniciado.\n");
                } else {
                    kprintf("Não eh possivel iniciar o ciclo.\n");
                }
                break;
            case 5:
                kprintf("Informe o tempo de agendamento (em segundos): ");
                int tempo;

                char buffer[10];
                read(CONSOLE, buffer, sizeof(buffer));
                tempo = atoi(buffer);

                microondas.tempoAgendado = tempo;
                microondas.agendamentoAtivo = true;
                kprintf("Ciclo agendado para comecar em %d segundos.\n", tempo);
                break;
            case 6:
                microondas.emergenciaAtivada = true;
                break;
            case 7:
                signal(microondas.mutex);
                kprintf("Encerrando micro-ondas...\n");
                return;
            
        }
    }
  return 0;
}
