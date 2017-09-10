// Universidade da Madeira
// Centro de Competencias das Ciencias Exactas e da Engenharia
// Arquitectura de Computadores
// Docentes: Dionisio Barros, Élvio Jesus, Nuno Ferreira, Sofia Inácio
// Alunos: Diogo Henrique da Silva Cruz (2030115), Diogo Duarte Rodrigues Nóbrega (2027415)
// Descricao: Programa de um jogo para fugir de objetos. O jogador utiliza um carro e tem de tentar fugir de todos os objetos que vão caindo em posições aleatórias.
// Linguagem: C


/********************************************************************************/
// Livrarias externas
/********************************************************************************/

#include <Reg51.h>
#include <stdlib.h>			//Usada para gerar numeros aleatorios

/********************************************************************************/
// Definicao de constantes
/********************************************************************************/

#define NUM_LINHAS 7		//Numero de linhas de LEDs
#define CMSBT0	0x3c	 	//Byte mais significativo do timer 0 - 50 ms (12MHz)
#define CLSBT0	0xaf	 	//Byte menos significativo do timer 0 - 50 ms (12MHz)
#define CTempo 20			//Tempo do timer 0 para medir 1 s (20x50 ms)
#define CT1 0x70	   		//0x70 Tempo do timer 1 - 112 us (12MHz)

/********************************************************************************/
// Definicao de variaveis
/********************************************************************************/

char VTempo; 				// Variavel para medir 1s (20x50 ms)
int  vidas = 3;				//Numero de vidas (inicializado com 3)
char  nivel = 1;			//Nivel (inicializado com 1 - Primeiro Nivel)
char tempo_total = 18;		//Tempo entre queda de obstaculos (controla a velocidade) (inicializado com 18 - valor usado para o primeiro nivel)
char tempo_decorrido = 0;	//Tempo que ja passou entre quedas de obstaculos
char tempo_nivel = 0;		//Tempo em que esta no nivel atual
char linha;					//Usada na funcao Mostrar() - Indica a linha da matriz de leds que esta a ser atualizada
char sucesso = 0;			//Usada na funcao adicionaObstaculo() - Garante que e gerado um numero que acenda um dos 5 leds de determinada linha
char acabou = 0;			//Indica se o jogo ja acabou (0 - esta a decorrer; 1 - ja acabou)

char Colunas[NUM_LINHAS] = {0, 0, 0, 0, 0, 0, 4};					//Vetor que e colocado no display (inicializado com apenas o jogador no display (a meio, em baixo))
char Linhas[NUM_LINHAS] = {254, 253, 251, 247, 239, 223, 191};		//Vetor que controla as linhas;
char Vitoria[NUM_LINHAS] = {17,17,17,17,17,10,4};					//Vetor que guarda a informacao respetiva a imagem de "Vitoria" (V - Victory)
char Derrota[NUM_LINHAS] = {16,16,16,16,16,16,31};					//Vetor que guarda a informacao respetiva a imagem de "Derrota" (L - Loss)

/********************************************************************************/
// Tratamento da excecao 0 (Esquerda - P3.2)
/********************************************************************************/
// Funcao que trata da excecao 0
// Descricao:
// Move o jogador para a esquerda, exceto se este ja estiver no extremo esquerdo

void Trata_Excecao0() interrupt 0
{
	if(acabou == 0)									//Se o jogo ainda nao acabou
	{
		if ( Colunas[6] < 16 )						//Se o jogador tiver numa posicao que nao a extrema esquerda (8, 4, 2, 1)
			Colunas[6] = Colunas[6] * 2;			//Multiplica por 2 para obter o valor correspondente a posicao da esquerda
	}
}

/********************************************************************************/
// Tratamento da excecao 1 (Direita - P3.3)
/********************************************************************************/
// Funcao que trata da excecao 1
// Descricao:
// Move o jogador para a direita, exceto se este ja estiver no extremo direito

void Trata_Excecao1() interrupt 2
{
	if(acabou == 0)									//Se o jogo ainda nao acabou
	{
		if ( Colunas[6] > 1 )						//Se o jogador tiver numa posicao que nao a extrema direita (16, 8, 4, 2)
			Colunas[6] = Colunas[6] / 2;			//Divide por 2 para obter o valor correspondente a posicao da direita
	}
}

/********************************************************************************/
// Verifica colisao
/********************************************************************************/
// Funcao invocada por atualiza()
// Descricao:
// Verifica se houve colisao. Se houver decrementa as vidas e indica que o jogo acabou

void verificaColisao()
{
	if(Colunas[5] == Colunas[6]) 					//Se as duas ultimas linhas forem iguais
	{												//entao estao a colidir
		if(vidas != 0)								//Se ainda tiver vidas
			vidas--;								//Decrementa-as
		else										//Se nao tiver
			acabou = 1;								//Indica que acabou
	}
}

/********************************************************************************/
// Retorna Numero Aleatorio
/*******************************************************************************/
// Funcao invocada por descerObstaculos()
// Descricao:
// Retorna um numero aleatorio

int getNumeroAleatorio(int min, int max)
{
    return (min + (int) (rand() / (double) (RAND_MAX + 1) * (max - min + 1))) * 2;
}

/********************************************************************************/
// Adiciona Obstaculo
/*******************************************************************************/
// Funcao invocada por descerObstaculos()
// Descricao:
// Adiciona um obstaculo a primeira linha numa posicao aleatoria

char adicionaObstaculo()
{
	int aux;										//Variavel de retorno
	sucesso = 0;									//Variavel que garante que e gerado um numero que acenda um dos 5 leds de determinada linha
	while(sucesso == 0)								//Enquanto nao tiver sido gerado um numero adequado gera outro
	{
		aux = (rand () % 25) + 1;					//Gera um numero aleatorio
		if(aux == 1 || aux == 2 || aux == 4 || aux == 8 || aux == 16)	//Se esse numero for um que permita acender uma e apenas uma linha
			sucesso = 1;												//Indica que obteve sucesso e nao precisa de gerar outro
	}
	return aux;										//Retorna o valor que sera colocado na primeira linha
}

/********************************************************************************/
// Descer Obstaculos
/*******************************************************************************/
// Funcao invocada por atualiza()
// Descricao:
// Faz os obstaculos descerem

void descerObstaculos()
{
	if(acabou == 0)									//Se ainda nao tiver acabado o jogo
	{
		int i;										//Variavel local para o for
		for(i = 5 ; i > 0 ; i--)					//Para cada linha
		{
			Colunas[i] = Colunas[i-1];				//A linha i e igual a linha que esta acima
		}
		if(getNumeroAleatorio(0,10) > 5)			//Gera numero aleatorio que controla a frequencia com que e adicionado um novo obstaculo
		{											//para permitir que ocasionalmente apareca uma (ou mais) linhas "livres"
			Colunas[0] = adicionaObstaculo();		//Adiciona obstaculo na primeira linha
		}
		else										//Caso contrario
			Colunas[0] = 0;							//A primeira linha fica "livre"
	}
}

/********************************************************************************/
// Mostrar
/*******************************************************************************/
// Funcao invocada por Trata_Timer1()
// Descricao: 
// Atualiza a matriz de leds

void Mostrar()
{
	P1 = 0xff;							//Desliga todas as linhas de saida
	P2 = Colunas[linha];				//Mostra a linha
	P1 = Linhas[linha];					//Ativa todas as linhas de saida
	linha++;							//Passa para a proxima linha
	if(linha == NUM_LINHAS)				//Se ja tiver chegado a ultima linha do display
		linha = 0;						//Volta para a primeira linha
}

/********************************************************************************/
// Atualiza
/*******************************************************************************/
// Funcao invocada por Trata_Timer0()
// Descricao: 
// Atualiza os leds que deverao estar acesos e controla a velocidade do jogo

void atualiza()
{
	TH0 = CMSBT0;									//Timer 0 = 50 ms
	TL0 = CLSBT0;									//Verifica se ja passou 1s 
	VTempo --;										//Decrementa
	tempo_decorrido += 1;							//Incrementa o tempo que ja passou entre quedas de obstaculos
	if (VTempo == 0)								//Verifica se o tempo terminou
	{
		VTempo = CTempo;
		tempo_nivel++;								//Incrementa o tempo em que se esta neste nivel
		if(tempo_nivel == 10)						
		{
			if(nivel < 5)							//Se o nivel atual for menor que 5
			{
				nivel++;							//Passa para o proximo nivel
				tempo_nivel = 0;					//Volta a colocar a 0 para comecar a contar o tempo do nivel atual
			}
			else if(acabou == 0)					//Se tiver sobrevivido
			{
				int i;								//Variavel local para o for
				for(i = 6 ; i > 0 ; i--)			//Para cada linha
				{
					Colunas[i] = Vitoria[i];		//A linha que sera mostrada na matriz de leds na proxima invocacao de Mostrar() e a linha correspondente em "Vitoria"
					acabou = 1;						//Indica que acabou
				}
				Colunas[0] = 17;
			}
		}
	}
	if(tempo_decorrido == tempo_total && nivel < 6) //Se o tempo necessario para descer os obstaculos e o nivel for menor que 6
	{
		tempo_decorrido = 0; 						//Coloca novamente a 0
		verificaColisao();							//Verifica se ha colisao, decrementa as vidas se houver e indica que acabou
		if(vidas > 0 && acabou == 0)				//Se ainda tiver vidas e nao tiver acabado
			descerObstaculos();						//Desce os obstaculos
		else if(vidas <= 0 && acabou == 0)			//Se nao tiver vidas e colocada a informacao referente ao ecra "Derrota - L"
		{
			int i;									//Variavel local para o for
			for(i = 6 ; i > 0 ; i--)				//Para cada linha
			{
				Colunas[i] = Derrota[i];			//A linha a ser mostrada na proxima chamada de Mostrar()
				acabou = 1;							//Indica que acabou
			}
			Colunas[0] = 16;						
		}
		switch(nivel)								//Dependendo do nivel
		{
			case 1:									//Nivel 1
				tempo_total = 18;					//Tempo entre quedas de obstaculos (controla a velocidade)
				break;								//Sai do switch
			case 2:									//Nivel 2
				tempo_total = 15;					//Tempo entre quedas de obstaculos (controla a velocidade)
				break;								//Sai do switch
			case 3:									//Nivel 3
				tempo_total = 12;					//Tempo entre quedas de obstaculos (controla a velocidade)
				break;								//Sai do switch
			case 4:									//Nivel 4
				tempo_total = 9;					//Tempo entre quedas de obstaculos (controla a velocidade)
				break;								//Sai do switch
			case 5:									//Nivel 5
				tempo_total = 6;					//Tempo entre quedas de obstaculos (controla a velocidade)
				break;								//Sai do switch
			default:
				break;
		}
	}
}

/*******************************************************************************/
// Tratamento do Timer 0
/*******************************************************************************/
// Funcao de tratamento da interrupcao do temporizador 0
// Descricao: 
// Actualiza os obstaculos e controla a velocidade do jogo

void Trata_Timer0() interrupt 1
{
	atualiza(); //Invoca funcao que atualiza os leds que deverao estar acesos e que controla a velocidade do jogo
}

/*******************************************************************************/
//Tratamento do Timer 1
/*******************************************************************************/
// Funcao de tratamento da interrupcao do temporizador 1
// Descricao: 
// Actualiza a linha do display (placa grafica)

void Trata_Timer1() interrupt 3
{
	Mostrar(); //Invoca a funcao que atualiza a matriz de leds
}
	
/********************************************************************************/
// Main
/********************************************************************************/
// Funcao Principal
// Descricao: 
// Primeira funcao a ser chamada

void main(void)
{
	TMOD = 33;					// Timer 0 de 16 bits - #00100001b
	// Timer 1 de 8 bits - auto reload
	TH0 = CMSBT0;				// Timer 0 = 50 ms
	TL0 = CLSBT0; 
	TH1 = CT1;					// Timer 1 = 112 us
	TL1 = CT1; 
	VTempo = CTempo;   				// Inicializa variavel para medir 1s
	IP = 0;						// Nao altera as prioridades
	IE = 143;					// Activa as interrupcoes - #10001111b:
	// Ext 0 
	// Timer 0
	// Ext1
	// Timer 1
	IT0 = 1;					// Ext0 detectada na transicao descendente
	IT1 = 1;					// Ext1 detectada na transicao descendente
	TR0 = 1;					// Inicia timer 0
	TR1 = 1;					// Inicia timer 1
	linha = 0;					// Inicia linha a 0
	P3 = 0xff;					// P3 e uma porta de entrada
	for(;;)
	{
		
	}
}