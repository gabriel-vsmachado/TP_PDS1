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
typedef struct {
	float x, y;
	int vel;	
	int dir_x;
	int dir_y;
	float oxygen;
	int diversRescued;
	int lives;
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
typedef struct {
	float x,y;
	int dir_x;
	int vel;
	ALLEGRO_COLOR color;
} Diver;




//Frequencia de atualizacao do jogo, em quadros por segundo
//As velocidades das estruturas sao medidas em pixels por quadro
const float FPS = 100;


/* Dimensoes da janela, tamanho do submarino e velocidades previstas.
 * Na versao inicial, initSub utiliza SUB_VEL_ABOVE; SUB_VEL_SEA ainda
 * nao e utilizado para alternar a velocidade ao mergulhar. */
const int SCREEN_W = 1920;
const int SCREEN_H = 1080;
const int SUB_HEIGHT = 40;
const int SUB_WIDTH = 160;
const int SUB_VEL_ABOVE = 10;
const int SUB_VEL_SEA = 2;


/* Constantes reservadas para a implementacao dos objetos e do oxigenio.
 * O programa inicial cria somente um Diver, sem utilizar NUM_DIVERS. */
#define NUM_DIVERS 12
const int DIVER_WIDTH = 16;
const int DIVER_HEIGHT = 6;
const int MAX_OXYGEN = 12;


//Dimensoes previstas para inimigos
const int FOE_WIDTH = 32;
const int FOE_HEIGHT = 14;


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
	SEA_COLOR = al_map_rgb(0, 0, 155);
	SKY_COLOR = al_map_rgb(70, 130, 180);
	size_32 = al_load_font("arial.ttf", 32, 1);   
}

void drawOxygenBar(Sub sub) {

	//Delimita as dimensões e posicionamento da barra na tela
	float width = 400;
	float height = 40;
	float x = SCREEN_W - width - 20;
	float y = 15;

	float oxygenFilled = sub.oxygen / MAX_OXYGEN;
	float widthFilled = width * oxygenFilled;

	//Desenha o fundo cinza da barra vazia
	al_draw_filled_rectangle(x, y, x + width, y + height, al_map_rgb(128, 128, 128));

	//Desenha a parte preenchida do oxigênio por cima
	ALLEGRO_COLOR oxygenColor;
	if(sub.oxygen <= 3) {
		oxygenColor = al_map_rgb(255, 0, 0);	
	}
	else {
		oxygenColor = al_map_rgb(255, 255, 255);
	}	
	al_draw_filled_rectangle(x, y, x + widthFilled, y + height, oxygenColor);	
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
   
   	al_draw_filled_rectangle(0, 0, SCREEN_W, SEA_LEVEL, SKY_COLOR);

	char text_score[4];
   	sprintf(text_score, "%d", SCORE);
   	al_draw_text(size_32, al_map_rgb(0, 200, 0), 10, 10, 0, text_score);  
	
	drawOxygenBar(sub);
}

//Inicializao submarino com posição, velocidade, direção e oxigênio iniciais
void initSub(Sub *sub) {
	sub->x = 0;
	sub->y = SEA_LEVEL - SUB_HEIGHT/2;
	sub->vel = SUB_VEL_ABOVE;
	sub->dir_x = 0;
	sub->dir_y = 0;
	sub->oxygen = MAX_OXYGEN;
	sub->diversRescued = 0;
	sub->lives = 3;
	sub->color = al_map_rgb(138, 43, 226);
}

/**
 * @brief Imprime os atributos de um objeto para depuracao.
 * @param d Objeto cujos dados serao exibidos.
 * @param id Identificador exibido na mensagem.
 * @note Funcao auxiliar disponivel, mas nao chamada no laco principal.
 */
void printDiver(Diver d, int id) {
	printf("\n(%d) %.1f %.1f %d %d", id, d.x, d.y, d.dir_x, d.vel);
}


/**
 * @brief Inicializa um unico objeto circular verde.
 * @param diver Ponteiro para o objeto a ser inicializado.
 *
 * Define a posicao inicial em (0, SCREEN_H/2), velocidade 2,
 * dir_x igual a 1 e foe igual a 0. Nao cria um vetor, nao sorteia
 * caracteristicas e nao distribui objetos por diferentes profundidades.
 */
void initDiver(Diver *diver) {
	
	//Soreteia a direção horizontal do diver (entre -1 e 1)
	diver->dir_x = ((rand() % 2) * 2) - 1;
	
	//Sorteia a velocidade do diver (entre 1 e 3 pixels por quadro)
	diver->vel = (rand() % 3) + 1;
	
	//Define a cor do diver como branco
	diver->color = al_map_rgb(255,255,255);

	//Define o posicionamento horizontal inicial fora da tela
	if(diver->dir_x > 0) {
		diver->x = -100 - (rand() % 300);
	} 
	else {
		diver->x = SCREEN_W + 100 + (rand() % 300);
	}

	//Sorteia a profundidade do diver
	diver->y = SEA_LEVEL + 50 + (rand() % (int)(SCREEN_H - SEA_LEVEL - 100)); 
}

// @brief Desenha o submarino como um retangulo preenchido arredondado
//As dimensoes sao definidas pelas constantes SUB_WIDTH e SUB_HEIGHT
void drawSub(Sub sub) {


	al_draw_filled_rounded_rectangle(sub.x, sub.y, sub.x+SUB_WIDTH, sub.y+SUB_HEIGHT, SUB_WIDTH/3, SUB_HEIGHT/3, sub.color);


}

//Criação de uma estrutura para representar torpedos inimigos com posição, velocidade, direção e cor
typedef struct {
	float x, y;
	int vel;
	int dir_x;
	ALLEGRO_COLOR color;
}  Foe;

//Inicializa um torpedo inimigo com posição, velocidade e direção iniciais
void initFoe(Foe *foe) {
	
	//Sorteia a direção horizontal do torpedo inimigo (entre -1 e 1)
	foe->dir_x = ((rand() % 2) * 2) - 1;

	//Sorteia a velocidade do torpedo inimigo (entre 3 e 6 pixels por quadro)
	foe->vel = (rand() % 4) + 3;

	//Posicionamento horizontal inicial do torpedo inimigo fora da tela
	if(foe->dir_x > 0) {
		foe->x = -150 - (rand() % 200);
	} 
	else {
		foe->x = SCREEN_W + 150 + (rand() % 200);
	}

	//Sorteia a pprofundidade inicial do torpedo inimigo
	foe->y = SEA_LEVEL + 50 + (rand() % (int)(SCREEN_H - SEA_LEVEL - 100));

	//Define a cor vermelha do torpedo inimigo
	foe->color = al_map_rgb(255, 0, 0);
}

//Atualizações do torpedo inimigo
void updateFoe(Foe *foe) {

	//Atualiza a posição horizontal do torpedo inimigo (eixo x)
	foe->x += foe->dir_x * foe->vel;

	//Reposiciona o torpedo que sair da tela à direita com novos valores
	if(foe->dir_x > 0 && foe->x > SCREEN_W) {
		initFoe(foe);
	}
	//Reposiciona o torpedo que sair da tela à esquerda com novos valores
	if(foe->dir_x < 0 && foe->x < 0) {
		initFoe(foe);
	}
}

//Desenha o torpedo inimigo 
void drawFoe(Foe foe) {

	al_draw_filled_rectangle(foe.x, foe.y - FOE_HEIGHT/2, foe.x + FOE_WIDTH, foe.y + FOE_HEIGHT/2, foe.color);

}

//Atualiza a posição do submarino de acordo com o teclado
//A cada quadro, soma dir_x * vel a x e dir_y * vel a y.
//nao altera a velocidade por profundidade
void updateSub(Sub *s) {

	//Atualiza a posição horizontal do submarino (eixo X)
	s->x += s->dir_x * s->vel;

	//Atualiza a posição vertical do submarino (eixo Y)
	s->y += s->dir_y * s->vel;

	//Restringe o limite da tela à esquerda
	if(s->x < 0) {
		s->x = 0;
	}

	//Restringe o limite da tela à direita
	if(s->x > SCREEN_W - SUB_WIDTH) {
		s->x = SCREEN_W - SUB_WIDTH;
	}

	//Restringe o limite superior da tela (superfície do mar)
	if(s->y < SEA_LEVEL - SUB_HEIGHT/2) {
		s->	y = SEA_LEVEL - SUB_HEIGHT/2;
	}

	//Restringe o limite inferior da tela
	if(s->y + SUB_HEIGHT > SCREEN_H) {
		s->y = SCREEN_H - SUB_HEIGHT;
	}

	//Mecânica de oxigênio:
	
	//Verifica se o submarino está abaixo do nível do mar e consome oxigênio, deixando o valor mínimo em 0
	//Pela taxa de atualização de 100fps, o oxigênio completo consome em 40 segundos
	if(s->y > SEA_LEVEL) {
		s->oxygen -= 0.003;

		if(s->oxygen < 0) {
			s->oxygen = 0;
		}
	}
	//Se o submarino estiver acima do nível do mar, reabastece o oxigênio (máximo em MAX_OXYGEN)
	//Pela taxa de atualização de 100fps, o oxigênio completo reabastece em 6 segundos
	else {
		s->oxygen += 0.02;

		//Se houver mergulhadores resgatados, adiciona pontos por cada resgate
		if(s->diversRescued > 0) {
			SCORE += s->diversRescued * 100;
			printf("\nMergulhadores entregues na superfície! Placar atual: %d", SCORE);
			//Esvazia o submarino 
			s->diversRescued = 0;
		}
	}
	if(s->oxygen > MAX_OXYGEN) {
		s->oxygen = MAX_OXYGEN;
	}
	printf("\nOxigênio: %.2f", s->oxygen);
}




/**
 * @brief Desenha o objeto como um circulo preenchido.
 * @param diver Objeto que fornece centro e cor do circulo.
 *
 * O raio do circulo e fixo em 20 pixels; DIVER_WIDTH e
 * DIVER_HEIGHT ainda nao sao utilizados no desenho.
 */
void drawDiver(Diver diver) {


	al_draw_filled_circle(diver.x, diver.y, 20, diver.color);


}

void updateDiver(Diver *diver) {

	//Atualiza a posição horizontal com base na direção e velocidade do objeto
	diver->x += diver->dir_x *diver->vel;

	//Loop para reposicionar divers que sairem da tela à direita
	if(diver->dir_x > 0 && diver->x > SCREEN_W) {
		diver->x = -20;
	}
	//Loop para reposicionar divers que sairem da tela à esquerda
	if(diver->dir_x < 0 && diver->x < 0) {
		diver->x = SCREEN_W + 20;
	}
}

bool checkCollisionSubDiver(Sub s, Diver d) {

	//Variáveis temporárias que armazenam a posição mais próxima do diver ao submarino
	float closestx = d.x;
	float closesty = d.y;
	//Trava x dentro dos limites horizontais do submarino
	if(closestx < s.x){
		closestx = s.x;
	}
	else if(closestx > s.x + SUB_WIDTH){
		closestx = s.x + SUB_WIDTH;
	}
	//Trava y dentro dos limites verticais do submarino
	if(closesty < s.y){
		closesty = s.y;
	}
	else if(closesty > s.y + SUB_HEIGHT){
		closesty = s.y + SUB_HEIGHT;
	}

	//Usando Pitágoras, calcula a distância entre o ponto mais próximo entre o submarino e o centro do diver
	float distx = d.x - closestx;
	float disty = d.y - closesty;
	float distSquared= (distx * distx) + (disty * disty);

	//Retorna verdadeiro ou falso caso a distância seja menor que o raio do diver de 20 pixels)
	return sqrt(distSquared) < 20.0;
}

bool checkCollisionSubFoe(Sub sub, Foe foe) {

	//Definição das medidas do submarino para a colisão entre o submarino e o torpedo inimigo
	float subWidth = 160;
	float subHeight = 40;

	//Verifica se há sobreposição entre o submarino e o torpedo inimigo
	if(sub.x < foe.x + FOE_WIDTH && sub.x + subWidth > foe.x &&
	   sub.y < foe.y + FOE_HEIGHT && sub.y + subHeight > foe.y) {
		return true;
	}
	return false;
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

	Diver divers[6];
	for(int i=0; i<6; i++) {
		initDiver(&divers[i]);
	}

	Foe foes[12];
	for(int i=0; i<12; i++) {
		initFoe(&foes[i]);
	}

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
			for(int i=0; i<6; i++){
				updateDiver(&divers[i]);
			}
			for(int i=0; i<12; i++) {
				updateFoe(&foes[i]);
			}

			for(int i=0; i<6; i++){
				if(checkCollisionSubDiver(sub, divers[i])){
					sub.diversRescued++;
					SCORE += 10;
					printf("\nMergulhador resgatado! Pontos: %d", SCORE);
					divers[i].x = -20;
					initDiver(&divers[i]);
				}
			}
				
			for(int i=0; i<12; i++) {
				if(checkCollisionSubFoe(sub, foes[i])) {
					sub.lives--;
					printf("\nSubmarino atingido por um torpedo inimigo! Vidas restantes: %d", sub.lives);
				
					initFoe(&foes[i]);

					if(sub.lives <= 0) {
						printf("\nGame Over! Final Score: %d", SCORE);
						playing = false;
					}
				}
			}

			if(sub.oxygen <= 0) {
				printf("\nGame Over! O oxigênio acabou! Final Score: %d", SCORE);
				playing = false;
			}

			drawSub(sub);
			for(int i=0; i<6; i++) {
				drawDiver(divers[i]);
			}
			
			for(int i=0; i<12; i++) {
				drawFoe(foes[i]);
			}

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
