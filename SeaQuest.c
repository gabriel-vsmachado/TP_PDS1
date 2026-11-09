/**
 * @file seaquest_v0_documentado.c
 * @brief Versao inicial de um jogo inspirado em Seaquest, usando Allegro 5.
 *
 * Este arquivo e o ponto de partida do trabalho pratico de PDS-1.
 * O codigo fornecido ja cria a janela, trata as teclas W, A, S e D,
 * desenha um submarino e um objeto circular e atualiza suas posicoes.
 *
 * Nesta versao inicial, o submarino ainda pode sair dos limites da tela;
 * o objeto se move apenas para a direita, sem reaparecer ao sair da tela.
 * As mecanicas de resgate, inimigos, colisoes, oxigenio e recorde ainda
 * nao foram implementadas. A pontuacao comeca em zero e nao e alterada.
 *
 * O projeto utiliza Allegro 5.0.10 e requer o arquivo de fonte
 * "arial.ttf" no diretorio de execucao para desenhar textos.
 *
 * Os comentarios descrevem o comportamento efetivamente implementado.
 * As estruturas e constantes tambem incluem campos destinados a etapas
 * futuras do trabalho pratico.
 */


#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>




/**
 * @brief Dados do submarino controlado pelo jogador.
 *
 * As coordenadas (x, y) correspondem ao canto superior esquerdo do
 * submarino. dir_x e dir_y representam a direcao solicitada pelo teclado
 * (-1, 0 ou 1, normalmente). A velocidade e medida em pixels por quadro.
 * O campo oxygen e inicializado, mas ainda nao e atualizado no jogo.
 */
typedef struct Sub {
	float x, y;
	int vel;	
	int dir_x;
	int dir_y;
	float oxygen;
	ALLEGRO_COLOR color;
} Sub;


/**
 * @brief Dados de um objeto movel no oceano.
 *
 * O nome da estrutura e os campos dir_x e foe foram mantidos para
 * futuras etapas. No codigo atual, existe apenas um objeto, desenhado
 * como um circulo verde. O campo foe recebe zero e nao e consultado;
 * dir_x tambem nao e utilizado pela funcao de movimentacao atual.
 */
typedef struct Diver {
	float x,y;
	int dir_x;
	int vel;
	int foe; // Indicador reservado para diferenciar mergulhadores e inimigos.
	ALLEGRO_COLOR color;
} Diver;




/* Frequencia de atualizacao do jogo, em quadros por segundo.
 * As velocidades das estruturas sao medidas em pixels por quadro. */
const float FPS = 100;


/* Dimensoes da janela, tamanho do submarino e velocidades previstas.
 * Na versao inicial, initSub utiliza SUB_VEL_ABOVE; SUB_VEL_SEA ainda
 * nao e utilizado para alternar a velocidade ao mergulhar. */
const int SCREEN_W = 960;
const int SCREEN_H = 540;
const int SUB_HEIGHT = 20;
const int SUB_WIDTH = 80;
const int SUB_VEL_ABOVE = 5;
const int SUB_VEL_SEA = 1;


/* Constantes reservadas para a implementacao dos objetos e do oxigenio.
 * O programa inicial cria somente um Diver, sem utilizar NUM_DIVERS. */
#define NUM_DIVERS 12
const int DIVER_WIDTH = 8;
const int DIVER_HEIGHT = 3;
const int MAX_OXYGEN = 12;


/* Dimensoes previstas para inimigos; nao utilizadas nesta versao. */
const int FOE_WIDTH = 32;
const int FOE_HEIGTH = 14;


/* Altura da faixa superior da tela e pontuacao corrente.
 * SCORE permanece em zero, pois ainda nao ha sistema de resgate. */
int SEA_LEVEL;
int SCORE = 0;


/* Recursos globais: cores e fonte para a interface.
 * SKY_COLOR e as distancias maximas de colisao estao reservadas
 * para extensoes do projeto e ainda nao sao utilizadas. */
ALLEGRO_COLOR SEA_COLOR;
ALLEGRO_COLOR SKY_COLOR;
ALLEGRO_FONT *size_32;
float MAX_DIST_COLLISION_DIVER;
float MAX_DIST_COLLISION_FOE;


/**
 * @brief Inicializa o nivel da superficie, as cores e a fonte global.
 *
 * Define SEA_LEVEL como um sexto da altura da tela, define as cores
 * SEA_COLOR e SKY_COLOR e carrega a fonte "arial.ttf" no tamanho 32.
 * @note A funcao nao verifica se o carregamento da fonte teve sucesso.
 */
void initGlobalVars() {
	SEA_LEVEL = SCREEN_H/6;
	SEA_COLOR = al_map_rgb(0,0,155);
	SKY_COLOR = al_map_rgb(10,10,10);
	size_32 = al_load_font("arial.ttf", 32, 1);   
}




/**
 * @brief Desenha o fundo azul, a faixa superior cinza e a pontuacao.
 * @param sub Estado do submarino; parametro ainda nao utilizado aqui.
 *
 * @note Esta versao nao desenha a barra de oxigenio.
 * @note O vetor text_score comporta somente tres caracteres numericos
 *       e o terminador '\0'; o codigo original usa sprintf sem limite.
 */
void drawScenario(Sub sub) {


   al_clear_to_color(SEA_COLOR);   
   
   al_draw_filled_rectangle(0, 0, SCREEN_W, SEA_LEVEL, al_map_rgb(70,70,70)); 


   char text_score[4];
   sprintf(text_score, "%d", SCORE);
   al_draw_text(size_32, al_map_rgb(0, 200, 0), 10, 10, 0, text_score);   


}


/**
 * @brief Inicializa posicao, velocidade, direcoes, oxigenio e cor.
 * @param sub Ponteiro para o submarino a ser inicializado.
 *
 * O submarino comeca no limite da faixa superior, com velocidade
 * SUB_VEL_ABOVE, sem movimento e com o reservatorio cheio.
 */
void initSub(Sub *sub) {
	sub->x = 0;
	sub->y = SEA_LEVEL - SUB_HEIGHT/2;
	sub->vel = SUB_VEL_ABOVE;
	sub->dir_x = 0;
	sub->dir_y = 0;
	sub->oxygen = MAX_OXYGEN;
	sub->color = al_map_rgb(230, 230, 0);
}




/**
 * @brief Imprime os atributos de um objeto para depuracao.
 * @param d Objeto cujos dados serao exibidos.
 * @param id Identificador exibido na mensagem.
 * @note Funcao auxiliar disponivel, mas nao chamada no laco principal.
 */
void printDiver(Diver d, int id) {
	printf("\n(%d) %.1f %.1f %d %d %d", id, d.x, d.y, d.dir_x, d.vel, d.foe);
}


/**
 * @brief Inicializa um unico objeto circular verde.
 * @param diver Ponteiro para o objeto a ser inicializado.
 *
 * Define a posicao inicial em (0, SCREEN_H/2), velocidade 1,
 * dir_x igual a 1 e foe igual a 0. Nao cria um vetor, nao sorteia
 * caracteristicas e nao distribui objetos por diferentes profundidades.
 */
void initDiver(Diver *diver) {
	diver->dir_x = 1;
	diver->x = 0;
	diver->y = SCREEN_H/2;
	diver->vel = 1;
	diver->foe = 0;
	diver->color = al_map_rgb(0,255,0);
}


/**
 * @brief Desenha o submarino como um retangulo preenchido arredondado.
 * @param sub Estado utilizado para obter a posicao e a cor do desenho.
 *
 * As dimensoes sao definidas pelas constantes SUB_WIDTH e SUB_HEIGHT.
 */
void drawSub(Sub sub) {


	al_draw_filled_rounded_rectangle(sub.x, sub.y, sub.x+SUB_WIDTH, sub.y+SUB_HEIGHT, SUB_WIDTH/3, SUB_HEIGHT/3, sub.color);


}


/**
 * @brief Atualiza a posicao do submarino de acordo com o teclado.
 * @param s Ponteiro para o submarino a ser atualizado.
 *
 * A cada quadro, soma dir_x * vel a x e dir_y * vel a y.
 * Nesta versao, nao limita a posicao a tela, nao altera a velocidade
 * por profundidade e nao consome nem reabastece oxigenio.
 */
void updateSub(Sub *s) {




	s->x += s->dir_x * s->vel;


	s->y += s->dir_y * s->vel;




}




/**
 * @brief Desenha o objeto como um circulo preenchido.
 * @param diver Objeto que fornece centro e cor do circulo.
 *
 * O raio do circulo e fixo em 10 pixels; DIVER_WIDTH e
 * DIVER_HEIGHT ainda nao sao utilizados no desenho.
 */
void drawDiver(Diver diver) {


	al_draw_filled_circle(diver.x, diver.y, 10, diver.color);


}


/**
 * @brief Desloca um unico objeto horizontalmente para a direita.
 * @param diver Ponteiro para o objeto cuja posicao sera atualizada.
 *
 * Acrescenta vel a coordenada x em cada quadro. Ainda nao considera
 * dir_x, nao testa os limites da tela e nao reposiciona o objeto.
 */
void updateDiver(Diver *diver) {


	diver->x += diver->vel;


}






 
/**
 * @brief Inicializa a Allegro e executa o laco principal do jogo.
 *
 * Inicializa os modulos graficos, o temporizador, a janela, o teclado
 * e a fila de eventos. Cria um Sub e um Diver. A cada evento do timer,
 * desenha o cenario, atualiza as duas posicoes e apresenta o quadro.
 * As teclas W, A, S e D controlam a direcao; fechar a janela encerra
 * o laco. Ao terminar, apresenta uma tela com a pontuacao corrente.
 *
 * Nao ha verificacao de colisoes, oxigenio ou recorde. O unico evento
 * que encerra normalmente a partida e o fechamento da janela.
 *
 * @param argc Quantidade de argumentos de linha de comando (nao usada).
 * @param argv Vetor de argumentos de linha de comando (nao usado).
 * @return 0 em caso de encerramento normal, ou -1 em determinadas
 *         falhas de inicializacao.
 */
int main(int argc, char **argv){




	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
   
	// ----------------------- Rotinas de inicializacao -----------------------
    
	// Inicializa a biblioteca Allegro.
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    // Inicializa o modulo de primitivas graficas da Allegro.
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	// Inicializa o modulo de carregamento de imagens (ainda nao utilizado).
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	// Cria um temporizador que gera eventos a cada 1.0/FPS segundos.
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	// Cria uma janela de SCREEN_W por SCREEN_H pixels.
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}


	// Habilita a entrada por teclado.
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	


	// Inicializa o modulo de fontes da Allegro.
	al_init_font_addon();


	// Inicializa o modulo que permite carregar fontes TrueType (TTF).
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	// Carrega novamente uma fonte local de tamanho 32 para uso na tela final.
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}


 	// Cria a fila de eventos utilizada pelo laco principal.
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
   




	// Registra eventos da janela (por exemplo, o clique no botao de fechar).
	al_register_event_source(event_queue, al_get_display_event_source(display));
	// Registra os eventos produzidos pelo temporizador.
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	// Registra eventos de pressionamento e liberacao de teclas.
	al_register_event_source(event_queue, al_get_keyboard_event_source());


	initGlobalVars();


	Sub sub;
	initSub(&sub);


	Diver diver;
	initDiver(&diver);






	// Inicia a emissao de eventos periodicos pelo temporizador.
	al_start_timer(timer);
	
	int playing = 1;
	while(playing) {
		ALLEGRO_EVENT ev;
		// Aguarda o proximo evento e armazena seus dados em ev.
		al_wait_for_event(event_queue, &ev);


		// A cada evento do temporizador, atualiza e desenha um quadro.
		if(ev.type == ALLEGRO_EVENT_TIMER) {


			drawScenario(sub);




			updateSub(&sub);
			updateDiver(&diver);








			drawSub(sub);
			drawDiver (diver);




			// Apresenta na janela o quadro que acabou de ser desenhado.
			al_flip_display();
		}
		// Fecha a partida caso a janela seja encerrada.
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}


		// Ao pressionar uma tecla, altera a direcao de movimento.
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {


			switch(ev.keyboard.keycode) {
			// Tecla W: movimento para cima (no evento de pressionamento).
				case ALLEGRO_KEY_W:
					sub.dir_y--;
				break;


				case ALLEGRO_KEY_S:
					sub.dir_y++;
				break;


				case ALLEGRO_KEY_A:
					sub.dir_x--;
				break;


				case ALLEGRO_KEY_D:
					sub.dir_x++;
				break;	
			}
		}			


		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			// Linha auxiliar de depuracao (desabilitada).
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);


			switch(ev.keyboard.keycode) {
			// Tecla W: interrompe a contribuicao da tecla para o movimento.
				case ALLEGRO_KEY_W:
					sub.dir_y++;
				break;


				case ALLEGRO_KEY_S:
					sub.dir_y--;
				break;


				case ALLEGRO_KEY_A:
					sub.dir_x++;
				break;


				case ALLEGRO_KEY_D:
					sub.dir_x--;
				break;	
				
			}
		}			


	} // Fim do laco principal.
     
	// Aguarda brevemente, exibe a pontuacao e libera recursos.
	
	al_rest(2);
	
	
	char my_text[20];	
	int record; // Variavel reservada para o futuro sistema de recorde.
	// Limpa a tela com uma cor clara para o resultado final.
	al_clear_to_color(al_map_rgb(230,240,250));
	sprintf(my_text, "Score: %d", SCORE);
	al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2, 0, my_text);




	
	// Apresenta a tela final na janela.
	al_flip_display();	
    al_rest(3);	
	
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   
 
	return 0;
}
