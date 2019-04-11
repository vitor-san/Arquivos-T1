/*
* Programa que manipula arquivos de dados,
* permitindo leitura, escrita e recuperação
* de campos e registros escritos em disco.
* Autor: 10734345, Vitor Santana Cordeiro
* Turma: BCC B
* São Carlos, SP - Brasil
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

typedef unsigned char byte; //define um "byte"

typedef struct {  //define o registro de cabecalho
  byte status;
  long topoLista;
  char tagCampo[5];
  char desCampo[5][40];
} regCabec;

typedef struct {  //define o registro de dados
  char removido;
  int tamanhoRegistro;
  long encadeamentoLista; //inteiro de 8 bytes
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

int leCSV() {
  char fileName[51];   //vai guardar o nome do arquivo a ser aberto
  scanf("%50s", fileName);

  FILE *readFile = fopen(fileName, "r");  //abre o arquivo "fileName" para leitura
  FILE *writeFile = fopen("arquivoTrab1.bin", "wb");  //cria um novo arquivo binario para escrita
  if (readFile == NULL || writeFile == NULL) {   //erro na abertura dos arquivos
    printf("Falha no carregamento do arquivo.");
    return -1;
  }

  byte paginaDisco[TAMPAG];
  memset(paginaDisco, '@', TAMPAG);   //inicializo a pagina de disco com "lixo"

  fwrite(paginaDisco, 1, sizeof(paginaDisco), writeFile); //crio a primeira pagina de disco no arquivo
  fseek(writeFile, -TAMPAG, SEEK_CUR);  //retorna o "ponteiro" de escrita para o inicio da pagina de disco

  regCabec cabecalho;   //cria um registro de cabecalho, utilizando valores definidos na especificacao
  cabecalho.status = 0;
  cabecalho.topoLista = -1L;
  cabecalho.tagCampo[0] = 'i';
  cabecalho.tagCampo[1] = 's';
  cabecalho.tagCampo[2] = 't';
  cabecalho.tagCampo[3] = 'n';
  cabecalho.tagCampo[4] = 'c';
  //agora, so falta armazenar os metadados
  char c, buffer[40];
  int i = 0, cont = 0;
  while ((c = fgetc(readFile)) != EOF && c != '\n') {   //le a primeira linha do arquivo csv (que contem os metadados)
    if (c == ',') {
      strcpy(cabecalho.desCampo[cont++], buffer);   //guardo o metadado lido no campo correspondente
      i = 0;  //reinicio o indexador do buffer
    }
    buffer[i++] = c;  //guardo o char lido no buffer
  }

  //agora leio o resto do arquivo, criando os registros de dados
  










  printf("Leitura do CSV \"%s\" e escrita no BIN \"arquivoTrab1.bin\"!\n", fileName);

  fclose(readFile);
  fclose(writeFile);
  return 0;
}

int mostraBin() {
  char fileName[51];   //vai guardar o nome do arquivo a ser aberto
  scanf("50%s", fileName);

  FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária
  if (readFile == NULL) {   //erro na abertura do arquivo
    printf("Falha no processamento do arquivo.");
    return -1;
  }

  printf("Mostra na tela o BIN \"%s\" (formatado)!\n", fileName);

  fclose(readFile);
  return 0;
}

int buscaReg() {
  char fileName[100];   //vai guardar o nome do arquivo a ser aberto
  char fieldName[100];  //campo a ser buscado
  //int
  scanf("%s", fileName);


  FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária
  if (readFile == NULL) {   //erro na abertura do arquivo
    printf("Falha no processamento do arquivo.");
    return -1;
  }

  printf("Busca o registro %s no arquivo BIN %s!\n", fieldName, fileName);

  fclose(readFile);
  return 0;
}

// Cuida da execução do programa.
int main () {
  int opt; //vai guardar a opção digitada pelo usuário
  scanf("%d ", &opt);   //lê a opção

  switch (opt) {
    case 1:   //lê um arquivo CSV e gera um arquivo BIN
      return leCSV();
    case 2:   //mostra os registros de um arquivo BIN
      return mostraBin();
    case 3:   //mostra registros que satisfaçam um critério de busca
      return buscaReg();
    default:
      printf("Opcao invalida!\n");
      return -1;  //erro na execução
  }
}
