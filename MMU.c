#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

/* -- Especificacoes de memória -- */
#define MEM_LOGICA_SIZE     1024
#define MEM_PRINC_SIZE      64
#define PAGE_SIZE           8
#define PAGE_TABLE_SIZE     128
#define MAX_TAM_PROCESSO    500000//byte

/* ----------- Métodos ----------- */
void printMemoriaLogica();
void printMemoriaFisica();
void printMMU();
void printEstatisticas();
void* processo();
void addNaMemoriaLogica();
void addNaMemoriaFisica();
int verificaFrameLivre();
int verificaFrameMemoria();
void lru();
int decimal_binario();

/* ----------- Estruturas ----------- */
struct Frame{
	char informacao;
	int endereco_fisico; 
};

struct Pagina{
	char informacao;
	int endereco_logico;
};

struct TabelaDePaginas{
	int endereco_fisico;
	int endereco_logico;
	int bit_validade;
	
	//Relogio para LRU
	time_t relogio;
	int tempo[2];
	time_t tempo_default;
};

struct TabelaDePaginas MMU[PAGE_TABLE_SIZE]; 
struct Pagina memoriaLogica[MEM_LOGICA_SIZE/PAGE_SIZE];
struct Frame memoriaFisica[MEM_PRINC_SIZE/PAGE_SIZE];
int ml_qtdd_paginas = MEM_LOGICA_SIZE/PAGE_SIZE;
int mf_qtdd_frames = MEM_PRINC_SIZE/PAGE_SIZE; 

/* ------- Variáveis Globais ------- */
struct Pagina paginasP[125];
int pag; 
bool ml_preenchida = false;
int pagSolicitada = -1;
char printSolicitada;
//Estatísicas
int frames_na_memoria;
int falta_de_paginas;
int qtt_paginas_retiradas;
int qtt_paginas_solicitadas;
  


int main(){
	//signal
	signal(SIGUSR1, addNaMemoriaLogica);
	signal(SIGUSR2, addNaMemoriaFisica);
	signal(SIGTERM, lru);
	signal(SIGINT, printEstatisticas);

	printf("\n----------- INICIANDO GERENCIAMENTO DE MEMÓRIA -----------\n");
	sleep(2);
	printf("TAMANHO MEMORIA LOGICA:		%ld\n", (sizeof(memoriaLogica)/sizeof(memoriaLogica[0])));
	printf("TAMANHO MEMORIA PRINCIPAL:	%ld\n", (sizeof(memoriaFisica)/sizeof(memoriaFisica[0])));

	sleep(4);
	//Configurando as memorias
	for(int i = 0 ; i < ml_qtdd_paginas; i++) memoriaLogica[i].endereco_logico = i;
	for(int i = 0 ; i < mf_qtdd_frames; i++) memoriaFisica[i].endereco_fisico = i;
	
	printMemoriaLogica();
	sleep(2);
	printMemoriaFisica();
	sleep(4);
	
	/* ---------------------- CRIANDO PROCESSO ----------------------*/
	pthread_t t1;
	//Definido tamanho aleatóriamente
	srand(time(NULL));
	int novoProcesso_tamanho = (rand()%MAX_TAM_PROCESSO);
	float tamanho =  (float) novoProcesso_tamanho+90000;
	pag = ceil((tamanho/1000)/8);
	
	printf("\n----------- NOVO PROCESSO CRIADO -----------");	
	if(pthread_create(&t1, NULL, &processo, &pag) != 0){
		perror("Falha ao criar processo");
		exit(EXIT_FAILURE);
	}

	pthread_exit(NULL);
	raise(SIGINT);
	return 0;
}

void* processo(void *arg){
	int tamanho = *(int*) arg;
	printf("\nProcesso iniciado com %d paginas\n", tamanho);
	sleep(4);
	
	//Adiciona informacoes
	struct Pagina paginas[tamanho];
	char infos;
	printf("\nAdicionando informações...\n");
	sleep(2);
	for(int i= 0; i < tamanho;i++){
		paginas[i].informacao = rand()%25+65;
		paginasP[i] =paginas[i] ;
		printf("%c ", paginas[i].informacao);
	}
	
	sleep(2);
	//Sinal para adicionar as páginas na memória lógica
	raise(SIGUSR1);
	
	//Inicia a solicitacao de paginas algumas páginas inicialmente
	pagSolicitada = rand()%tamanho;
	int quantidade_solicitacoes = rand()%300+60;
	//printf("\nO processo vai solicitar: %d\n", quantidade_solicitacoes);
	
	for(int i = 0; i < quantidade_solicitacoes; i++){
		printf("\nNova pagina solicitada pelo processo\n");
		raise(SIGUSR2);
		printf("Informação da pagina solicitada: %c\n", printSolicitada);
		sleep(3);
		pagSolicitada = rand()%tamanho;
	}

	return NULL;
}

void addNaMemoriaLogica(){
		printf("\n\nAdicionando na memória lógica...");
		for(int i = 0; i< pag; i++) {
			//adicionando na memória lógica
			for (int j = 0; j < 125; j++){
				if(memoriaLogica[j].informacao == 0){
					paginasP[i].endereco_logico = j;
					memoriaLogica[j] = paginasP[i];
					
					//add na tabela de paginas
					for (int k = 1; k < 200; k++){
						if(MMU[k].endereco_logico == 0){
							MMU[k].endereco_logico = paginasP[i].endereco_logico;
							break;
						}
					}
					break;
				}
		}
	}
	MMU[0].endereco_logico = paginasP[0].endereco_logico;
	printMemoriaLogica();
	printMMU();
}

void addNaMemoriaFisica(){
		qtt_paginas_solicitadas++;
		//Funcao para verificar o prox frame livre
		int frameLivre = verificaFrameLivre();
		
		printf("\nAdicionando na memória física a página %d(%c)", decimal_binario(pagSolicitada), paginasP[pagSolicitada].informacao);
		sleep(1);
		
		if (verificaFrameMemoria(paginasP[pagSolicitada]) == 0){ //verifica se ja ta na mem fisica
			if(frameLivre == -1){ //verifica para LRU
				printf("Não há frames livres na memória física");
				raise(SIGTERM);
				addNaMemoriaFisica();
			}else{
					//atualiza na memória física
					struct Frame novoFrame;
					novoFrame.informacao = paginasP[pagSolicitada].informacao;
					novoFrame.endereco_fisico = memoriaFisica[frameLivre].endereco_fisico;
					memoriaFisica[frameLivre] = novoFrame; 
					printSolicitada = novoFrame.informacao;
					//atualiza a tabela de páginas
					for (int i = 0; i < MEM_LOGICA_SIZE; i++){	//Passa por toda a tabela
						if(MMU[i].endereco_logico == paginasP[pagSolicitada].endereco_logico){
							MMU[i].endereco_fisico = novoFrame.endereco_fisico;
							MMU[i].bit_validade = 1;

							MMU[i].relogio = time(NULL);
							struct tm tm = *localtime(&MMU[i].relogio);
							MMU[i].tempo[0] = tm.tm_min;
							MMU[i].tempo[1] = tm.tm_sec;
							break;
						}
					}				
			}
		}else{ //Só atualiza o tempo
			for (int i = 0; i < MEM_LOGICA_SIZE; i++){	
				if(MMU[i].endereco_logico == paginasP[pagSolicitada].endereco_logico){
					MMU[i].relogio = time(NULL);
					struct tm tm = *localtime(&MMU[i].relogio);
					MMU[i].tempo[0] = tm.tm_min;
					MMU[i].tempo[1] = tm.tm_sec;
					break;
				}
			}
			frames_na_memoria++;		
		}
		sleep(2);
		printMemoriaFisica();
		printMMU();
}


void lru(){
	printf("\nSolicitando LRU...");
	sleep(3);
	int endereco_fisico;
	time_t relogio = time(NULL);
	qtt_paginas_retiradas++;

	//Encontrar o menos recente
	for (int i = 0; i < PAGE_TABLE_SIZE; i++){
		if(relogio > MMU[i].relogio && MMU[i].relogio != MMU[i].tempo_default) {
			relogio =  MMU[i].relogio;
		}
	}

	//atualiza os dados na tabela, zerando
	for (int i = 0; i < PAGE_TABLE_SIZE; i++){
		if(relogio == MMU[i].relogio){
		
			endereco_fisico = MMU[i].endereco_fisico;
			MMU[i].endereco_fisico = 0;
			MMU[i].bit_validade = 0;
			MMU[i].relogio = MMU[i].tempo_default;
			MMU[i].tempo[0] = 0;
			MMU[i].tempo[1] = 0;
				
			for(int i = 0; i < mf_qtdd_frames; i ++){
				if(memoriaFisica[i].endereco_fisico == endereco_fisico){
					memoriaFisica[i].informacao = 0;
					break;
				}
			}
			break;
		}
	}
	printMemoriaFisica();
	printMMU();
}

int verificaFrameLivre(){
	for (int i = 0; i < mf_qtdd_frames; i++){
		if(memoriaFisica[i].informacao == 0) return i;
	}
	return -1;
}

int verificaFrameMemoria(struct Pagina p){
	printf("\nVerificando se o frame já está na memória fisica...");
	sleep(3);
	for (int i = 0; i < PAGE_TABLE_SIZE; i++){
					if(MMU[i].endereco_logico == p.endereco_logico && MMU[i].bit_validade == 1){
							printf("O frame já está na memória. Tabela:  %03d => %02d (%d) %d:%d  \n", decimal_binario(MMU[i].endereco_logico), decimal_binario(MMU[i].endereco_fisico), MMU[i].bit_validade,MMU[i].tempo[0], MMU[i].tempo[1]);
							return -1;
							}
				}
				printf("Não está na memória ainda\n");
				falta_de_paginas++;
				return 0;
}


int decimal_binario(int n){
	int bin = 0;
	int rem, i = 1;
	
	while(n != 0){
		rem = n %2;
		n /=2;
		bin += rem * i;
		i *= 10;
	}
	
	return bin;
}

/*-----------------------------PRINTS-------------------------*/
void printMemoriaLogica(){
	printf("\nMEMORIA LOGICA\n");
	for(int i = 0; i < ml_qtdd_paginas; i ++){
		if(i%9 == 0) printf("\n");
		printf("| %07d (%c) ", decimal_binario(memoriaLogica[i].endereco_logico),  memoriaLogica[i].informacao); 
	}
	printf("|\n");
}

void printMemoriaFisica(){
	printf("\nMEMORIA FISICA\n");
	for(int i = 0; i < mf_qtdd_frames; i ++){
		printf("| %02d (%c) ", decimal_binario(memoriaFisica[i].endereco_fisico), memoriaFisica[i].informacao); 
	}
	printf("|\n");
}

void printMMU(){
	printf("\nTABELA DE PAGINAS\n");
	for(int i = 0; i < PAGE_TABLE_SIZE/2; i ++){
		if(MMU[i].endereco_logico == 0 && i != 0) break;
		printf("|  %03d => %02d (%d) %d:%d  \n", decimal_binario(MMU[i].endereco_logico), decimal_binario(MMU[i].endereco_fisico), MMU[i].bit_validade,MMU[i].tempo[0], MMU[i].tempo[1]);
	}
	printf("\n");
}

void printEstatisticas(){
	printf("\n\n==================== ESTATÍSTICAS ====================\n");
	printf("  QUANTIDADE DE PAGINAS TOTAL:	%d\n", pag);
	printf("  SOLICITACOES: %d\n", qtt_paginas_solicitadas);
	printf("  FALTAS DE PÁGINAS:	%d\n", falta_de_paginas);
	printf("  FRAMES JA NA MEMORIA:	%d\n", frames_na_memoria);
	printf("  LRU: %d\n", qtt_paginas_retiradas);
	printf("========================================================");
	
	kill(getpid(), SIGKILL);

}


