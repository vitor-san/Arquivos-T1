/*
* Programa que manipula dados de arquivos,
* permitindo leitura, escrita e impressao
* de campos e registros escritos em disco.
* Autor: 10734345, Vitor Santana Cordeiro
* Turma: BCC B
* Sao Carlos, SP - Brasil
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

typedef unsigned char byte; //define o tipo de dados "byte"

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    long long topoLista;
    char tagCampo[5];
    char desCampo[5][40];
} regCabec;

typedef struct {  //define o tipo de dados "registro de dados"
    char removido;
    int tamanhoRegistro;
    long long encadeamentoLista; //inteiro de 8 bytes
    int idServidor;
    double salarioServidor;
    char telefoneServidor[14];
    int tamCampo4;  //indicador de tamanho
    char tagCampo4;
    char *nomeServidor; //string de tamanho variavel
    int tamCampo5;  //indicador de tamanho
    char tagCampo5;
    char *cargoServidor;  //string de tamanho variavel
} regDados;

/*
  Funcao que le o arquivo CSV e organiza seus
  registros em um arquivo binario de saida,
  utilizando organizacao hibrida dos campos.
*/
void leCSV() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    scanf("%50s", fileName);

    FILE *readFile = fopen(fileName, "r");  //abro o arquivo de nome "fileName" para leitura
    FILE *writeFile = fopen("arquivoTrab1.bin", "wb");  //crio um novo arquivo binario para escrita

    if (readFile == NULL || writeFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    byte lixo[TAMPAG];    //crio uma "pagina de disco" so com lixo
    memset(lixo, '@', TAMPAG);  //preencho com lixo

    fwrite(lixo, TAMPAG, 1, writeFile); //inicializo a primeira pagina de disco do arquivo com lixo
    fseek(writeFile, -TAMPAG, SEEK_CUR);  //retorno o "ponteiro" de escrita para o inicio da pagina de disco

    regCabec cabecalho;   //crio um registro de cabecalho, utilizando valores definidos na especificacao
    cabecalho.status = '0'; //ao se estar escrevendo em um arquivo, seu status deve ser 0
    cabecalho.topoLista = -1L;
    cabecalho.tagCampo[0] = 'i';
    cabecalho.tagCampo[1] = 's';
    cabecalho.tagCampo[2] = 't';
    cabecalho.tagCampo[3] = 'n';
    cabecalho.tagCampo[4] = 'c';

    //agora, so falta armazenar os metadados
    char c, buffer[100];  //o buffer sera utilizado tanto aqui quanto na hora de ler os registros de dados
    int idx = 0, cont = 0;   //indexador e contador

    while ((c = fgetc(readFile)) != EOF) {   //enquanto nao chegar no final do arquivo...
        if (c == ',' || c == '\n') {   //se achei uma virgula ou uma quebra de linha, cheguei ao fim de um campo
            buffer[idx++] = '\0';   //finalizo a string ate entao lida
            strcpy(cabecalho.desCampo[cont++], buffer);   //guardo o metadado lido no campo correspondente
            idx = 0;  //reinicio o indexador do buffer

            if (c == '\n') break;   //se cheguei no '\n', isso significa que a primeira linha (que contem os metadados) ja foi lida
        }
        else buffer[idx++] = c;  //guardo o char lido no buffer
    }

    fwrite(&(cabecalho.status), 1, 1, writeFile);  //escrevo o campo "status" no arquivo binario
    fwrite(&(cabecalho.topoLista), 8, 1, writeFile);  //escrevo o campo "topoLista" no arquivo binario
    for (int i = 0; i < 5; i++) {
        fwrite(&(cabecalho.tagCampo[i]), 1, 1, writeFile);  //escrevo o campo "tagCampoX" no arquivo binario
        fwrite(cabecalho.desCampo[i], strlen(cabecalho.desCampo[i])+1, 1, writeFile);  //escrevo o campo "desCampoX" no arquivo binario
        fseek(writeFile, 40-strlen(cabecalho.desCampo[i])-1, SEEK_CUR); //como "desCampoX" eh de tamanho fixo, pulo para o comeco do proximo campo (se nao a funcao escreveria os dados em cima do campo anterior)
    }

    fseek(writeFile, TAMPAG, SEEK_SET);   //reposiciono o ponteiro de escrita para ficar no comeco da proxima pagina de disco

    //agora leio o resto do arquivo, criando os registros de dados

    regDados registro;   //molde para guardar temporariamente os valores lidos do csv, antes da sua escrita no arquivo binario
    registro.removido = '-';  //os registros vem "nao-removidos" por padrao
    registro.encadeamentoLista = -1L; //por padrao
    registro.tagCampo4 = cabecalho.tagCampo[3]; //char 'n'
    registro.nomeServidor = malloc(100*sizeof(char));
    registro.tagCampo5 = cabecalho.tagCampo[4]; //char 'c'
    registro.cargoServidor = malloc(100*sizeof(char));

    cont = 0, idx = 0;   //reinicio o contador e o indexador do buffer
    memset(buffer, '\0', 100);  //limpo o buffer
    int ehNulo = 0;   //indica se o campo que esta se lendo eh nulo ou nao
    int tamAntigo = 0;  //guarda o tamanho do ultimo registro escrito no arquivo binario
    int haDados = 0;  //indica se o arquivo CSV possui dados (alem do cabecalho)

    while ((c = fgetc(readFile)) != EOF) {  //enquanto nao chegar no final do arquivo...
        haDados = 1;

        if (c == '\n') {    //se cheguei no '\n', significa que este eh o ultimo campo do registro de dados atual

            if (ehNulo) {
                free(registro.cargoServidor);   //libero a memoria alocada anteriormente para o campo
                registro.cargoServidor = NULL;
            } else {
                buffer[idx] = '\0';   //finalizo a string ate entao lida
                if (registro.cargoServidor == NULL) registro.cargoServidor = malloc(100*sizeof(char));  //aloco memoria para o campo, caso ele nao tenha
                strcpy(registro.cargoServidor, buffer);
                registro.tamCampo5 = strlen(registro.cargoServidor) + 2;  // +2 pois conta 1 byte da tag e 1 byte do '\0' (ele nao eh contabilizado na strlen)
            }

            //calculo o tamanho total do registro e o armazeno
            if (registro.nomeServidor == NULL && registro.cargoServidor == NULL) {
                registro.tamanhoRegistro = 34;
                //34 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor e 14 do telefoneServidor
            }
            else if (registro.cargoServidor == NULL) {
                registro.tamanhoRegistro = 38 + registro.tamCampo4;
                //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 4
            }
            else if (registro.nomeServidor == NULL) {
                registro.tamanhoRegistro = 38 + registro.tamCampo5;
                //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 5
            }
            else {
                registro.tamanhoRegistro = 42 + registro.tamCampo4 + registro.tamCampo5;
                //42 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor, 4 do indicador de tamanho do campo 4 e mais 4 do indicador de tamanho do campo 5
            }

            //verifico o espaco disponivel na pagina de disco atual
            if ( (ftell(writeFile)%TAMPAG + registro.tamanhoRegistro + 5) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)

                //insiro o registro em uma outra pagina de disco, mas antes vou precisar completar a pagina de disco atual com lixo e trocar o indicador de tamanho do ultimo registro dela, para que ele contabilize esse lixo tambem
                int diff = TAMPAG - ftell(writeFile)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco

                fseek(writeFile, - (tamAntigo+4), SEEK_CUR); //movo o ponteiro de escrita para o inicio do indicador de tamanho do ultimo registro inserido
                int tamNovo = diff + tamAntigo;  //calculo o valor do novo indicador de tamanho
                fwrite(&tamNovo, 4, 1, writeFile); //sobreescrevo o valor do indicador de tamanho antigo pelo novo

                fseek(writeFile, tamAntigo, SEEK_CUR);  //coloco o ponteiro no final do ultimo registro
                fwrite(lixo, 1, diff, writeFile);  //completo com lixo
            }

            fwrite(&(registro.removido), 1, 1, writeFile);  //escrevo o campo "removido" no arquivo binario
            fwrite(&(registro.tamanhoRegistro), 4, 1, writeFile);  //escrevo o indicador de tamanho do registro no arquivo binario
            fwrite(&(registro.encadeamentoLista), 8, 1, writeFile);  //escrevo o campo "encadeamentoLista" no arquivo binario
            fwrite(&(registro.idServidor), 4, 1, writeFile);  //escrevo o campo "idServidor" no arquivo binario
            fwrite(&(registro.salarioServidor), 8, 1, writeFile);  //escrevo o campo "salarioServidor" no arquivo binario

            if (registro.telefoneServidor[0] == '\0') {   //se o campo "telefoneServidor" for nulo, entao...
                fwrite(registro.telefoneServidor, 1, 1, writeFile);  //escrevo o '\0'
                fwrite(lixo, 13, 1, writeFile);  //insiro mais 13 '@'s para completar o tamanho fixo do campo
            } else {
                fwrite(registro.telefoneServidor, 14, 1, writeFile);  //escrevo o campo "telefoneServidor" no arquivo binario
            }

            if (registro.nomeServidor != NULL) {
                fwrite(&(registro.tamCampo4), 4, 1, writeFile);  //escrevo seu indicador de tamanho no arquivo binario
                fwrite(&(registro.tagCampo4), 1, 1, writeFile);  //escrevo sua tag no arquivo binario
                fwrite(registro.nomeServidor, strlen(registro.nomeServidor)+1, 1, writeFile);  //escrevo-o no arquivo binario
            }

            if (registro.cargoServidor != NULL) {
                fwrite(&(registro.tamCampo5), 4, 1, writeFile);  //escrevo seu indicador de tamanho no arquivo binario
                fwrite(&(registro.tagCampo5), 1, 1, writeFile);  //escrevo sua tag no arquivo binario
                fwrite(registro.cargoServidor, strlen(registro.cargoServidor)+1, 1, writeFile);  //escrevo-o no arquivo binario
            }

            cont = 0; //reinicio o contador
            idx = 0;  //reinicio o indexador do buffer
            memset(buffer, '\0', 100);  //limpo o buffer
            tamAntigo = registro.tamanhoRegistro; //atualizo o tamanho do ultimo registro inserido
        }

        else if (c == ',') {

            switch (cont) {
                case 0: //o campo que li foi o idServidor
                    registro.idServidor = atoi(buffer); //converto o valor lido para inteiro e o armazeno
                    break;
                case 1: //o campo que li foi o salarioServidor
                    registro.salarioServidor = atof(buffer);  //converto o valor lido para double e o armazeno
                    break;
                case 2: //o campo que li foi o telefoneServidor
                    if (ehNulo) {
                        strcpy(registro.telefoneServidor, "\0");
                    } else {
                        for (int i = 0; i < 14; i++) registro.telefoneServidor[i] = buffer[i];
                    }
                    break;
                case 3: //o campo que li foi o nomeServidor
                    if (ehNulo) {
                        free(registro.nomeServidor);  //libero a memoria alocada anteriormente para o campo
                        registro.nomeServidor = NULL;
                    } else {
                        buffer[idx] = '\0';   //finalizo a string ate entao lida
                        if (registro.nomeServidor == NULL) registro.nomeServidor = malloc(100*sizeof(char));  //aloco memoria para o campo, caso ele nao tenha
                        strcpy(registro.nomeServidor, buffer);
                        registro.tamCampo4 = strlen(registro.nomeServidor) + 2;  // +2 pois conta 1 byte da tag e 1 byte do '\0' (ele nao eh contabilizado na strlen)
                    }
            }

            cont++;
            idx = 0;  //reinicio o indexador do buffer
            memset(buffer, '\0', 100);  //limpo o buffer
            ehNulo = 1;   //faz com que, se o proximo char lido for outra virgula, o loop saiba que aquele campo tem valor nulo
        }

        else {
            buffer[idx++] = c;
            ehNulo = 0;   //nao foi outra virgula, entao o valor do campo nao eh nulo
        }

    }

    if (haDados) {
        /* registro os dados da ultima linha do csv (ja que ela nao possui o '\n' no final) */

        //calculo o tamanho total do registro e o armazeno
        if (registro.nomeServidor == NULL && registro.cargoServidor == NULL) {
            registro.tamanhoRegistro = 34;
            //34 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor e 14 do telefoneServidor
        }
        else if (registro.cargoServidor == NULL) {
            registro.tamanhoRegistro = 38 + registro.tamCampo4;
            //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 4
        }
        else if (registro.nomeServidor == NULL) {
            registro.tamanhoRegistro = 38 + registro.tamCampo5;
            //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 5
        }
        else {
            registro.tamanhoRegistro = 42 + registro.tamCampo4 + registro.tamCampo5;
            //42 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor, 4 do indicador de tamanho do campo 4 e mais 4 do indicador de tamanho do campo 5
        }

        //verifico o espaco disponivel na pagina de disco atual
        if ( (ftell(writeFile)%TAMPAG + registro.tamanhoRegistro + 5) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)
            //insiro o registro em uma outra pagina de disco, mas antes vou precisar completar a pagina de disco atual com lixo e trocar o indicador de tamanho do ultimo registro dela, para que ele contabilize esse lixo tambem
            int diff = TAMPAG - ftell(writeFile)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco

            fseek(writeFile, - (tamAntigo+4), SEEK_CUR); //move o ponteiro de escrita para o inicio do indicador de tamanho do ultimo registro inserido
            int tamNovo = diff + tamAntigo;  //calculo o valor do novo indicador de tamanho
            fwrite(&tamNovo, 4, 1, writeFile); //sobreescrevo o valor do indicador de tamanho antigo pelo novo

            fseek(writeFile, tamAntigo, SEEK_CUR);  //coloco o ponteiro no final do ultimo registro
            fwrite(lixo, 1, diff, writeFile);  //completo com lixo
        }

        fwrite(&(registro.removido), 1, 1, writeFile);  //escrevo o campo "removido" no arquivo binario
        fwrite(&(registro.tamanhoRegistro), 4, 1, writeFile);  //escrevo o indicador de tamanho do registro no arquivo binario
        fwrite(&(registro.encadeamentoLista), 8, 1, writeFile);  //escrevo o campo "encadeamentoLista" no arquivo binario
        fwrite(&(registro.idServidor), 4, 1, writeFile);  //escrevo o campo "idServidor" no arquivo binario
        fwrite(&(registro.salarioServidor), 8, 1, writeFile);  //escrevo o campo "salarioServidor" no arquivo binario

        if (registro.telefoneServidor[0] == '\0') {   //se o campo "telefoneServidor" for nulo, entao...
            fwrite(registro.telefoneServidor, 1, 1, writeFile);  //escrevo o '\0'
            fwrite(lixo, 13, 1, writeFile);  //insiro mais 13 '@'s para completar o tamanho fixo do campo
        } else {
            fwrite(registro.telefoneServidor, 14, 1, writeFile);  //escrevo o campo "telefoneServidor" no arquivo binario
        }

        if (registro.nomeServidor != NULL) {
            fwrite(&(registro.tamCampo4), 4, 1, writeFile);  //escrevo seu indicador de tamanho no arquivo binario
            fwrite(&(registro.tagCampo4), 1, 1, writeFile);  //escrevo sua tag no arquivo binario
            fwrite(registro.nomeServidor, strlen(registro.nomeServidor)+1, 1, writeFile);  //escrevo-o no arquivo binario
        }

        if (registro.cargoServidor != NULL) {
            fwrite(&(registro.tamCampo5), 4, 1, writeFile);  //escrevo seu indicador de tamanho no arquivo binario
            fwrite(&(registro.tagCampo5), 1, 1, writeFile);  //escrevo sua tag no arquivo binario
            fwrite(registro.cargoServidor, strlen(registro.cargoServidor)+1, 1, writeFile);  //escrevo-o no arquivo binario
        }
    }

    printf("arquivoTrab1.bin");

    if (registro.nomeServidor != NULL) free(registro.nomeServidor);   //libero a memoria alocada anteriormente para o campo
    if (registro.cargoServidor != NULL) free(registro.cargoServidor);   //libero a memoria alocada anteriormente para o campo
    fclose(readFile);

    //antes de fechar o arquivo de escrita, coloco seu status para '1'
    fseek(writeFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    cabecalho.status = '1';
    fwrite(&(cabecalho.status), 1, 1, writeFile);  //sobrescrevo o campo "status" do arquivo binario
    fclose(writeFile);

    return;
}

/*
    Funcao que imprime na tela, organizadamente,
    um registro de um arquivo binario gerado
    anteriormente por este programa. A funcao
    assume que o usuario ira chama-la quando
    o ponteiro de leitura estiver exatamente no
    comeco do registro.

    Parametro:
        FILE *file - arquivo binario
*/
void mostraRegistro(FILE *file) {
    regDados registro;  //registro que ajudara a guardar os dados lidos
    int tamanho;    //ajudara a contar quantos bytes faltam para terminar o registro

    fread(&tamanho, 4, 1, file);    //leio os 4 bytes do indicador de tamanho do registro e os armazeno em "tamanho"
    fseek(file, 8, SEEK_CUR);   //pulo os 8 bytes do "encadeamentoLista", pois este campo nao sera mostrado
    tamanho -= 8;

    fread(&(registro.idServidor), 4, 1, file);   //leio os 4 bytes do campo "idServidor"
    tamanho -= 4;
    printf("%d", registro.idServidor);   //mostro o valor do campo "idServidor" na tela

    fread(&(registro.salarioServidor), 8, 1, file);   //leio os 8 bytes do campo "salarioServidor"
    tamanho -= 8;
    if (registro.salarioServidor != -1.0) printf(" %.2lf", registro.salarioServidor);   //mostro o valor do campo "salarioServidor" na tela
    else printf("         ");   //se o campo for nulo, mostro 8 espacos em branco

    fread(registro.telefoneServidor, 14, 1, file);   //leio os 14 bytes do campo "telefoneServidor"
    tamanho -= 14;
    if (registro.telefoneServidor[0] != '\0') printf(" %.14s", registro.telefoneServidor);   //mostro o valor do campo "telefoneServidor" na tela
    else printf("               "); //se o campo for nulo, mostro 14 espacos em branco

    if (tamanho == 0) {     //se ja cheguei no fim do registro...
        printf("\n");   //termino de mostrar o registro
        return;
    }

    byte b = fgetc(file);   //leio o byte apontado pelo ponteiro de leitura
    if (b == '@') {    //o registro eh o ultimo de uma pagina de disco
        printf("\n");   //termino de mostrar o registro
        fseek(file, tamanho-1, SEEK_CUR);   //reposiciono o ponteiro de leitura para ficar logo depois do final desse registro
        return;
    }
    ungetc(b, file);    //"devolvo" o byte lido para o arquivo

    //chegando neste ponto, sabemos que pelo menos um dos campos (nomeServidor ou cargoServidor) existe
    //OBS: nao temos certeza de qual, mas, como na hora de mostrar isso eh indiferente, ignoramos a especificidade

    fread(&(registro.tamCampo4), 4, 1, file);   //leio os 4 bytes do indicador de tamanho do campo
    tamanho -= 4;
    printf(" %d", registro.tamCampo4 - 2);  //mostro quantos caracteres o campo possui (-2 por conta do '\0' e da tag)

    fgetc(file);    //pulo a tag do campo
    tamanho -= 1;

    registro.nomeServidor = malloc(100*sizeof(char));
    //como dito, aqui estou guardando como se fosse o campo "nomeServidor", mas na verdade pode ser qualquer um dos dois
    fread(registro.nomeServidor, 1, registro.tamCampo4-1, file);    //leio os bytes do campo
    tamanho -= (registro.tamCampo4-1);  //-1 pois se retirou a tag da contagem
    printf(" %s", registro.nomeServidor);

    //testamos mais uma vez para ver se acabou o registro ou nao

    if (tamanho == 0) {     //se ja cheguei no fim do registro...
        printf("\n");   //termino de mostrar o registro
        free(registro.nomeServidor);    //libero memoria anteriormente alocada
        return;
    }

    b = fgetc(file);   //leio o byte apontado pelo ponteiro de leitura
    if (b == '@') {    //o registro eh o ultimo de uma pagina de disco
        printf("\n");   //termino de mostrar o registro
        fseek(file, tamanho-1, SEEK_CUR);   //reposiciono o ponteiro de leitura para ficar logo depois do final desse registro
        free(registro.nomeServidor);    //libero memoria anteriormente alocada
        return;
    }
    ungetc(b, file);    //"devolvo" o byte lido para o arquivo

    //agora sim, chegando a este ponto, sabemos que o campo previamente lido foi o "nomeServidor"

    fread(&(registro.tamCampo5), 4, 1, file);   //leio os 4 bytes do indicador de tamanho do campo "cargoServidor"
    tamanho -= 4;
    printf(" %d", registro.tamCampo5 - 2);  //mostro quantos caracteres o campo possui (-2 por conta do '\0' e da tag)

    fgetc(file);    //pulo a tag do campo
    tamanho -= 1;

    registro.cargoServidor = malloc(100*sizeof(char));
    fread(registro.cargoServidor, 1, registro.tamCampo5-1, file);    //leio os bytes do campo
    tamanho -= (registro.tamCampo5-1);  //-1 pois se retirou a tag da contagem
    printf(" %s", registro.cargoServidor);

    printf("\n");   //termino de mostrar o registro

    if (tamanho != 0) fseek(file, tamanho, SEEK_CUR);   //se o registro ainda nao terminou (provavelmente porque eh o ultimo registro de uma pagina de disco), reposiciono o ponteiro de leitura para ficar logo depois do final desse registro

    free(registro.nomeServidor);    //libero memoria anteriormente alocada
    free(registro.cargoServidor);   //libero memoria anteriormente alocada
    return;
}

/*
  Le um arquivo binario (anteriormente gerado pelo programa)
  e mostra na tela todos os seus registros, organizadamente.
  Ao final, mostra quantas paginas de disco foram acessadas
  ao todo.
*/
void mostraBin() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    scanf("%50s", fileName);

    FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária

    if (readFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    int acessosPagina = 0; //vai contar a quantidade de acessos a paginas de disco no decorrer da execucao

    if (fgetc(readFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    acessosPagina++;

    fseek(readFile, TAMPAG-1, SEEK_CUR);  //pulo a primeira pagina, que so tem o registro de cabecalho (lembrando que ja tinha lido o campo "status", por isso o -1)

    byte b = fgetc(readFile);

    if (feof(readFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

    while (!feof(readFile)) {
        if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

        if (b == '-') mostraRegistro(readFile);  //mostra o registro se ele nao esta removido
        else if (b == '*') {    //se ele esta removido...
            int pulo;
            fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
            fseek(readFile, pulo, SEEK_CUR);    //pula o registro
        }

        b = fgetc(readFile);
    }

    printf("Número de páginas de disco acessadas: %d", acessosPagina);

    fclose(readFile);
    return;
}

/*
    Funcao que imprime na tela, organizadamente,
    um registro de um arquivo binario gerado
    anteriormente por este programa. A funcao
    assume que o usuario ira chama-la quando
    o ponteiro de leitura estiver exatamente no
    comeco do registro. Alem disso, esta funcao
    imprimira, antes de cada campo, o metadado
    correspondente, que vira a partir de um
    registro de cabecalho passado por parametro.

    Parametros:
        FILE *file - arquivo binario
        regCabec *cabec - cabecalho do arquivo
*/
void mostraRegistroMeta(FILE *file, regCabec *cabec) {
    regDados registro;  //registro que ajudara a guardar os dados lidos
    int tamanho;    //ajudara a contar quantos bytes faltam para terminar o registro
    int tamCampo;   //ajudara na hora de manipular os campos de tamanho variavel

    fread(&tamanho, 4, 1, file);    //leio os 4 bytes do indicador de tamanho do registro e os armazeno em "tamanho"
    fseek(file, 8, SEEK_CUR);   //pulo os 8 bytes do "encadeamentoLista", pois este campo nao sera mostrado
    tamanho -= 8;

    fread(&(registro.idServidor), 4, 1, file);   //leio os 4 bytes do campo "idServidor"
    tamanho -= 4;
    printf("%s: ", cabec->desCampo[0]);   //mostro o metadado referente ao campo
    printf("%d\n", registro.idServidor);   //mostro o valor do campo "idServidor" na tela

    fread(&(registro.salarioServidor), 8, 1, file);   //leio os 8 bytes do campo "salarioServidor"
    tamanho -= 8;
    printf("%s: ", cabec->desCampo[1]);   //mostro o metadado referente ao campo
    if (registro.salarioServidor != -1.0) printf("%.2lf\n", registro.salarioServidor);   //mostro o valor do campo "salarioServidor" na tela
    else printf("valor nao declarado\n");

    fread(registro.telefoneServidor, 14, 1, file);   //leio os 14 bytes do campo "telefoneServidor"
    tamanho -= 14;
    printf("%s: ", cabec->desCampo[2]);   //mostro o metadado referente ao campo
    if (registro.telefoneServidor[0] != '\0') printf("%.14s\n", registro.telefoneServidor);   //mostro o valor do campo "telefoneServidor" na tela
    else printf("valor nao declarado\n");

    if (tamanho == 0) {     //se ja cheguei no fim do registro...
        printf("%s: ", cabec->desCampo[3]);   //mostro o metadado referente ao campo
        printf("valor nao declarado\n");
        printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
        printf("valor nao declarado\n");
        printf("\n");   //termino de mostrar o registro
        return;
    }

    byte b = fgetc(file);   //leio o byte apontado pelo ponteiro de leitura
    if (b == '@') {    //o registro eh o ultimo de uma pagina de disco
        printf("%s: ", cabec->desCampo[3]);   //mostro o metadado referente ao campo
        printf("valor nao declarado\n");
        printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
        printf("valor nao declarado\n");
        printf("\n");   //termino de mostrar o registro
        fseek(file, tamanho-1, SEEK_CUR);   //reposiciono o ponteiro de leitura para ficar logo depois do final desse registro
        return;
    }
    ungetc(b, file);    //"devolvo" o byte lido para o arquivo

    fread(&tamCampo, 4, 1, file);   //leio os 4 bytes do indicador de tamanho do campo
    tamanho -= 4;

    char tag = fgetc(file);    //leio a tag referente ao campo
    tamanho -= 1;

    if (tag == 'n') {   //o campo a ser lido sera "nomeServidor"
        registro.nomeServidor = malloc(100*sizeof(char));   //aloco espaco dinamicamente para guardar o campo
        fread(registro.nomeServidor, 1, tamCampo-1, file);    //leio os bytes do campo
        tamanho -= (tamCampo-1);  //-1 pois se retirou a tag da contagem
        printf("%s: ", cabec->desCampo[3]);   //mostro o metadado referente ao campo
        printf("%s\n", registro.nomeServidor);

        //testamos mais uma vez para ver se acabou o registro ou nao

        if (tamanho == 0) {     //se ja cheguei no fim do registro...
            printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
            printf("valor nao declarado\n");
            printf("\n");   //termino de mostrar o registro
            free(registro.nomeServidor);    //libero memoria anteriormente alocada
            return;
        }

        b = fgetc(file);   //leio o byte apontado pelo ponteiro de leitura
        if (b == '@') {    //o registro eh o ultimo de uma pagina de disco
            printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
            printf("valor nao declarado\n");
            printf("\n");   //termino de mostrar o registro
            fseek(file, tamanho-1, SEEK_CUR);   //reposiciono o ponteiro de leitura para ficar logo depois do final desse registro
            free(registro.nomeServidor);    //libero memoria anteriormente alocada
            return;
        }
        ungetc(b, file);    //"devolvo" o byte lido para o arquivo

        fread(&(registro.tamCampo5), 4, 1, file);   //leio os 4 bytes do indicador de tamanho do campo "cargoServidor"
        tamanho -= 4;

        fgetc(file);    //pulo a tag do campo
        tamanho -= 1;

        registro.cargoServidor = malloc(100*sizeof(char));   //aloco espaco dinamicamente para guardar o campo
        fread(registro.cargoServidor, 1, registro.tamCampo5-1, file);    //leio os bytes do campo
        tamanho -= (registro.tamCampo5-1);  //-1 pois se retirou a tag da contagem
        printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
        printf("%s\n", registro.cargoServidor);

        printf("\n");   //termino de mostrar o registro

        if (tamanho != 0) fseek(file, tamanho, SEEK_CUR);   //se o registro ainda nao terminou (provavelmente porque eh o ultimo registro de uma pagina de disco), reposiciono o ponteiro de leitura para ficar logo depois do final desse registro

        free(registro.nomeServidor);    //libero memoria anteriormente alocada
        free(registro.cargoServidor);   //libero memoria anteriormente alocada
    }
    else {      //o campo a ser lido sera "cargoServidor"
        printf("%s: ", cabec->desCampo[3]);   //mostro o metadado referente ao campo
        printf("valor nao declarado\n");   //o valor de "nomeServidor" nao foi declarado

        registro.cargoServidor = malloc(100*sizeof(char));   //aloco espaco dinamicamente para guardar o campo
        fread(registro.cargoServidor, 1, tamCampo-1, file);    //leio os bytes do campo
        tamanho -= (tamCampo-1);  //-1 pois se retirou a tag da contagem
        printf("%s: ", cabec->desCampo[4]);   //mostro o metadado referente ao campo
        printf("%s\n", registro.cargoServidor);

        printf("\n");   //termino de mostrar o registro
        if (tamanho != 0) fseek(file, tamanho, SEEK_CUR);   //se o registro ainda nao terminou (provavelmente porque eh o ultimo registro de uma pagina de disco), reposiciono o ponteiro de leitura para ficar logo depois do final desse registro
        free(registro.cargoServidor);
    }

    return;
}

/*
  Busca, em todo o arquivo binario, registros que
  satisfacam um criterio de busca determinado pelo
  usuario, mostrando-os na tela assim que sao encontrados.
  Um exemplo de busca seria "cargoServidor ENGENHEIRO",
  no qual a funcao ira mostrar na tela todos os registros
  em que o campo "cargoServidor" possui o valor "ENGENHEIRO".
  Tambem eh mostrado, ao final da execucao, quantas paginas
  de disco foram acessadas ao todo.
*/
void buscaReg() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    char nomeCampo[51];    //campo a ser considerado na busca
    byte valorCampo[50];    //valor a ser considerado na busca

    scanf("%50s %50s", fileName, nomeCampo);
    scanf(" %[^\r\n]", valorCampo);     //paro de ler antes da quebra de linha

    FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária

    if (readFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    int acessosPagina = 0; //vai contar a quantidade de acessos a paginas de disco no decorrer da execucao

    if (fgetc(readFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    acessosPagina++;

    fseek(readFile, 8, SEEK_CUR);   //pulo o campo "topoLista", pois ele nao sera considerado

    //comeco recuperando os metadados do registro de cabecalho
    regCabec cabecalho;
    for (int i = 0; i < 5; i++) {
        fgetc(readFile);    //pulo a tag (nao sera necessaria)
        fread(cabecalho.desCampo[i], 1, 40, readFile);  //recupero a descricao dos campos
    }

    fseek(readFile, TAMPAG-214, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b = fgetc(readFile);

    if (feof(readFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

    //aqui comeca a comparacao por campos

    int lidos = 0;    //guardara provisoriamente quantos bytes ja se leram de um registro
    int achou = 0;    //indicara se pelo menos um registro foi achado

    if (strcmp(nomeCampo, "idServidor") == 0) {     //se o campo a ser buscado eh "idServidor"...
        while (!feof(readFile)) {
            if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

            if (b == '-') {     //se ele nao esta removido...
                int indicTam;
                fread(&indicTam, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do registro
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "encadeamentoLista" (8 bytes)
                lidos += 8;

                int valor;
                fread(&valor, 4, 1, readFile);  //leio o valor do campo "idServidor"
                lidos += 4;

                fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                if (valor == atoi(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                    mostraRegistroMeta(readFile, &cabecalho);
                    achou = 1;
                    break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
                }

                fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                lidos = 0;
            }

            else if (b == '*') {    //se ele esta removido...
                int pulo = 0;
                fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
                fseek(readFile, pulo, SEEK_CUR);    //pula o registro
            }

            b = fgetc(readFile);
        }
    }
    else if (strcmp(nomeCampo, "salarioServidor") == 0) {     //se o campo a ser buscado eh "salarioServidor"...
        while (!feof(readFile)) {
            if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

            if (b == '-') {     //se ele nao esta removido...
                int indicTam;
                fread(&indicTam, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do registro
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "encadeamentoLista" (8 bytes)
                lidos += 8;

                fseek(readFile, 4, SEEK_CUR);  //pulo o campo "idServidor" (4 bytes)
                lidos += 4;

                double valor;
                fread(&valor, 8, 1, readFile);  //leio o valor do campo "salarioServidor"
                lidos += 8;

                fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                if (valor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                    mostraRegistroMeta(readFile, &cabecalho);
                    achou = 1;
                }
                else {
                    fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                }

                lidos = 0;
            }

            else if (b == '*') {    //se ele esta removido...
                int pulo = 0;
                fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
                fseek(readFile, pulo, SEEK_CUR);    //pula o registro
            }

            b = fgetc(readFile);
        }
    }
    else if (strcmp(nomeCampo, "telefoneServidor") == 0) {     //se o campo a ser buscado eh "telefoneServidor"...
        while (!feof(readFile)) {
            if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

            if (b == '-') {     //se ele nao esta removido...
                int indicTam;
                fread(&indicTam, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do registro
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "encadeamentoLista" (8 bytes)
                lidos += 8;

                fseek(readFile, 4, SEEK_CUR);  //pulo o campo "idServidor" (4 bytes)
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "salarioServidor" (8 bytes)
                lidos += 8;

                char valor[14];
                fread(valor, 1, 14, readFile);  //leio o valor do campo "telefoneServidor"
                lidos += 14;

                fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                if (strcmp(valor, valorCampo) == 0) {    //se o valor lido eh igual ao do dado como criterio de busca...
                    mostraRegistroMeta(readFile, &cabecalho);
                    achou = 1;
                }
                else {
                    fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                }

                lidos = 0;
            }

            else if (b == '*') {    //se ele esta removido...
                int pulo = 0;
                fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
                fseek(readFile, pulo, SEEK_CUR);    //pula o registro
            }

            b = fgetc(readFile);
        }
    }
    else if (strcmp(nomeCampo, "nomeServidor") == 0) {     //se o campo a ser buscado eh "nomeServidor"...
        while (!feof(readFile)) {
            if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

            if (b == '-') {     //se ele nao esta removido...
                int indicTam;
                fread(&indicTam, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do registro
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "encadeamentoLista" (8 bytes)
                lidos += 8;

                fseek(readFile, 4, SEEK_CUR);  //pulo o campo "idServidor" (4 bytes)
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "salarioServidor" (8 bytes)
                lidos += 8;

                fseek(readFile, 14, SEEK_CUR);  //pulo o campo "telefoneServidor" (14 bytes)
                lidos += 14;

                if (lidos != indicTam+4) {   //se o registro nao terminou...
                    b = fgetc(readFile);    //leio o proximo byte do registro
                    lidos += 1;

                    if (b == '@') {
                        //ele eh o ultimo de uma pagina de disco, e o lixo "acoplado" a ele deve ser pulado
                        fseek(readFile, indicTam+4 - lidos, SEEK_CUR);
                    }
                    else {
                        ungetc(b, readFile);    //"devolvo" o byte lido para o arquivo
                        lidos -= 1;

                        int tamCampo;
                        fread(&tamCampo, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do campo
                        lidos += 4;

                        char tag = fgetc(readFile);   //leio a tag do campo
                        lidos += 1;

                        if (tag == 'c') {   //o campo "nomeServidor" nao existe nesse registro de dados
                            fseek(readFile, tamCampo-1, SEEK_CUR);  //vou para o proximo registro
                        }
                        else {  //ele existe
                            char valor[100];
                            fread(valor, 1, tamCampo-1, readFile);  //leio o valor do campo "nomeServidor"
                            lidos += (tamCampo-1);

                            fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                            if (strcmp(valor, valorCampo) == 0) {    //se o valor lido eh igual ao do dado como criterio de busca...
                                mostraRegistroMeta(readFile, &cabecalho);
                                achou = 1;
                            }
                            else {
                                fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                        }
                    }
                }

                lidos = 0;
            }
            else if (b == '*') {    //se ele esta removido...
                int pulo = 0;
                fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
                fseek(readFile, pulo, SEEK_CUR);    //pula o registro
            }

            b = fgetc(readFile);
        }
    }
    else if (strcmp(nomeCampo, "cargoServidor") == 0) {     //se o campo a ser buscado eh "cargoServidor"...
        while (!feof(readFile)) {
            if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

            if (b == '-') {     //se ele nao esta removido...
                int indicTam;
                fread(&indicTam, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do registro
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "encadeamentoLista" (8 bytes)
                lidos += 8;

                fseek(readFile, 4, SEEK_CUR);  //pulo o campo "idServidor" (4 bytes)
                lidos += 4;

                fseek(readFile, 8, SEEK_CUR);  //pulo o campo "salarioServidor" (8 bytes)
                lidos += 8;

                fseek(readFile, 14, SEEK_CUR);  //pulo o campo "telefoneServidor" (14 bytes)
                lidos += 14;

                if (lidos != indicTam+4) {   //se o registro nao terminou...
                    b = fgetc(readFile);    //leio o proximo byte do registro
                    lidos += 1;

                    if (b == '@') {
                        //ele eh o ultimo de uma pagina de disco, e o lixo "acoplado" a ele deve ser pulado
                        fseek(readFile, indicTam+4 - lidos, SEEK_CUR);
                    }
                    else {
                        ungetc(b, readFile);    //"devolvo" o byte lido para o arquivo
                        lidos -= 1;

                        int tamCampo;
                        fread(&tamCampo, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do campo
                        lidos += 4;

                        char tag = fgetc(readFile);   //leio a tag do campo
                        lidos += 1;

                        if (tag == 'n') {   //o campo "cargoServidor" pode nao existir
                            fseek(readFile, tamCampo-1, SEEK_CUR);  //pulo o campo "nomeServidor"
                            lidos += (tamCampo-1);

                            if (lidos != indicTam+4) {   //se o registro ainda nao terminou...
                                b = fgetc(readFile);    //leio o proximo byte do registro
                                lidos += 1;

                                if (b == '@') {
                                    //ele eh o ultimo de uma pagina de disco, e o lixo "acoplado" a ele deve ser pulado
                                    fseek(readFile, indicTam+4 - lidos, SEEK_CUR);
                                }
                                else {
                                    ungetc(b, readFile);    //"devolvo" o byte lido para o arquivo
                                    lidos -= 1;

                                    fread(&tamCampo, 4, 1, readFile);   //leio e armazeno o valor do indicador de tamanho do campo
                                    lidos += 4;

                                    fgetc(readFile);   //pulo a tag do campo
                                    lidos += 1;

                                    char valor[100];
                                    fread(valor, 1, tamCampo-1, readFile);  //leio o valor do campo "nomeServidor"
                                    lidos += (tamCampo-1);

                                    fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                                    if (strcmp(valor, valorCampo) == 0) {    //se o valor lido eh igual ao do dado como criterio de busca...
                                        mostraRegistroMeta(readFile, &cabecalho);
                                        achou = 1;
                                    }
                                    else {
                                        fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                                    }
                                }
                            }
                        }
                        else {  //ele existe
                            char valor[100];
                            fread(valor, 1, tamCampo-1, readFile);  //leio o valor do campo "nomeServidor"
                            lidos += (tamCampo-1);

                            fseek(readFile, -lidos, SEEK_CUR);    //volto ao inicio do registro

                            if (strcmp(valor, valorCampo) == 0) {    //se o valor lido eh igual ao do dado como criterio de busca...
                                mostraRegistroMeta(readFile, &cabecalho);
                                achou = 1;
                            }
                            else {
                                fseek(readFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                        }
                    }
                }

                lidos = 0;
            }

            else if (b == '*') {    //se ele esta removido...
                int pulo = 0;
                fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
                fseek(readFile, pulo, SEEK_CUR);    //pula o registro
            }

            b = fgetc(readFile);
        }
    }
    else {  //o usuario digitou errado o nome do campo
        printf("Falha no processamento do arquivo.");
        return;
    }

    if (!achou) {
        printf("Registro inexistente.");
    }
    else {
        printf("Número de páginas de disco acessadas: %d", acessosPagina);
    }

    fclose(readFile);
    return;
}

/*
  Cuida da execucao do programa.
*/
int main() {
    int opt; //vai guardar a opcao digitada pelo usuario
    scanf("%d ", &opt);

    switch (opt) {
        case 1:
            leCSV();
            break;
        case 2:
            mostraBin();
            break;
        case 3:
            buscaReg();
            break;
        default:
            printf("Opcao invalida!\n");
    }

    return 0;
}
