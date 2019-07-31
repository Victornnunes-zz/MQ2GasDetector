/* 
  Trabalho de Introdução aos Algoritmos 
  Detecção de incêndio com quicksort
  Copyright 2019 by Claudio Manoel dos Reis, Lucas Nunes Vital Souza e Victor Nunes do Carmo
  Arquivo de coleta e processamento de dados (principal)
*/

#include <Keypad.h>
#include <SPI.h>
#include <SD.h>

/*  
    Declarações de conexão e chamada dos pinos do teclado matricial 4x4
    LINHA E COLUNA definem um teclado 4x4
    hexaKeys define as teclas do teclado que serão passadas como valor nas funções
    colPins e rowPins definem os pinos de colunas e linhas no arduino
    Keypad teclaPressionada inicializa a instância de Keypad.h
*/
const byte LINHA = 4;  
const byte COLUNA = 4; 
char hexaKeys[LINHA][COLUNA] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte colPins[COLUNA] = {6, 8, 9};  
byte rowPins[LINHA] = {0, 2, 3, 5};
Keypad teclaPressionada = Keypad(makeKeymap(hexaKeys), rowPins, colPins, LINHA, COLUNA);

/*  
    Declarações de conexão e chamada do pino do leitor SD
    chipSelect faz coneção do pino 4 do arduino com o CS do leitor
*/
const int chipSelect = 4;

/*  
    Declarações de conexão e chamada do pino do sensor MQ-2
    pin_a0 faz coneção do pino A2 do arduino com o DOUT (saida digital) do sensor
    nivel_sensor é a variavel regulada no sensor para sensibilidade de leitura
*/
int pin_a0 = A2;
int nivel_sensor = 500;

/*  
    Declarações de conexão e chamada do pino do buzzer
    pin_buzzer faz coneção do pino 10 do arduino com o positivo do buzzer
*/
int pin_buzzer = 10;

/*  
    Declarações do registro que será armazenado no cartão sd e será parametro para as funções do projeto
    int intensidade é o valor medido pelo sensor
    bool exclusao é a flag booleana que fica responsável por definir um registro excluso
*/
struct dados
{
    int intensidade;
    bool exclusao;
};

/*  
    Declarações úteis para a leitura de novos valores pelo sensor
    int contador_geral é responsável por definir a quantidade de valores para serem alocados dinâmicamente
    bool leitura é responsável por impedir que infinitos valores sejam inclusos em uma leitura de presença de gás
*/
int contador_geral = 0;
bool leitura = true;

void setup()
{
    Serial.begin(9600);

    //Testa a conexão do sensor SD e do cartão SD
    while (!Serial)
    {
        ;
    }
    Serial.print("Initializing SD card...");
    if (!SD.begin(4))
    {
        Serial.println("initialization failed!");
        while (1)
            ;
    }
    Serial.println("initialization done.");

    // Define buzzer (pin_buzzer) como saída e sensor (pin_a0 MQ-2) como entrada
    pinMode(pin_buzzer, OUTPUT);
    pinMode(pin_a0, INPUT);

    // Definições do MENU
    Serial.println("PRESSIONE 1 PARA ORDENAR CRESCENTE, 2 PARA DECRESCENTE, 3 PARA BUSCAR PELO MAIOR VALOR, 4 PARA BUSCAR PELO MENOR VALOR, 5 PARA DELETAR O MENOR VALOR E 6 PARA EXIBIR LEITURA DO ARQUIVO");

    // Busca a existência registros no arquivo "medicoes.txt"
    recuperaDoArquivo(contador_geral);
}

void loop()
{
    dados recebe;
    int valor_analogico = analogRead(pin_a0);
     
    if (valor_analogico > nivel_sensor)
    {
        digitalWrite(pin_buzzer, HIGH);
        if (leitura)
        {
            Serial.print("Gás detectado com intensidade ");
            Serial.println(valor_analogico);
            leitura = false;
            recebe.intensidade = valor_analogico;
            contador_geral++;
            escritaArquivoTXTSD(recebe);
        }
    }
    else
    {
        leitura = true;
        digitalWrite(pin_buzzer, LOW);
    }

    /*
        Define chamada de funções de acordo com teclas pressionadas e definidas pelo MENU
        tecla == '1' Ordena de forma crescente os registros no arquivo "medicoes.txt"
        tecla == '2' Ordena de forma decrescente os registros no arquivo "medicoes.txt"
        tecla == '3' Busca o maior valor de intensidade presente nos registros do arquivo "medicoes.txt"
        tecla == '4' Busca o menor valor de intensidade presente nos registros do arquivo "medicoes.txt"
        tecla == '5' Deleta o menor valor de intensidade presente nos registros do arquivo "medicoes.txt"
        tecla == '6' Lê e imprime o arquivo "medicoes.txt" no Monitor Serial
    */
    char tecla = teclaPressionada.getKey();
    if (tecla == '1')
    {
        dados *vetor = new dados[contador_geral];
        leDoArquivo(vetor);
        quicksort(vetor, 0, contador_geral - 1, '1');
        escreveNoArquivo(vetor, contador_geral);
        delete[] vetor;
        Serial.println("Ordenação crescente concluída com sucesso!");
    }
    else if (tecla == '2')
    {
        dados *vetor = new dados[contador_geral];
        leDoArquivo(vetor);
        quicksort(vetor, 0, contador_geral - 1, '2');
        escreveNoArquivo(vetor, contador_geral);
        delete[] vetor;
        Serial.println("Ordenação decrescente concluída com sucesso!");
    }
    else if (tecla == '3')
    {
        int resultado_busca;
        dados *vetor = new dados[contador_geral];
        leDoArquivo(vetor);
        quicksort(vetor, 0, contador_geral - 1, '2');
        resultado_busca = vetor[0].intensidade;
        Serial.print("O maior elemento é: ");
        Serial.println(resultado_busca);
        delete[] vetor;
    }
    else if (tecla == '4')
    {
        int resultado_busca;
        dados *vetor = new dados[contador_geral];
        leDoArquivo(vetor);
        quicksort(vetor, 0, contador_geral - 1, '1');
        resultado_busca = vetor[0].intensidade;
        Serial.print("O menor elemento é: ");
        Serial.println(resultado_busca);
        delete[] vetor;
    }
    else if (tecla == '5')
    {
        Serial.println("Menor Posicao deletada!");
        dados *vetor = new dados[contador_geral];
        leDoArquivo(vetor);
        quicksort(vetor, 0, contador_geral - 1, '1');
        vetor[0].exclusao = true;
        escreveNoArquivo(vetor, contador_geral);
        delete[] vetor;
        contador_geral -= 1;
    }
    else if (tecla == '6')
    {
        Serial.println("Leitura arquivo medições.txt");
        imprimeNoSerial(tecla);
        Serial.print("\n");
        Serial.println("Fim leitura arquivo medições.txt");
    }
    else if (tecla != '1' and tecla != '2' and tecla != '3' and tecla != '4' and tecla != '5' and tecla != '5' and tecla != NULL)
    {
        Serial.println("OPÇÃO INVÁLIDA");
    }
    delay(10);
}

/*  
    Função recuperaDoArquivo(int &contador_geral)
    Responsável por ler o arquivo "medicoes.txt" e informar a quantidade de registros presentes no mesmo
    Recebe como parâmetro (int &contador_geral)
    @param{int &contador_geral} quantidade de registros responsável por ser o tamanho do vetor que será alocado dinâmicamente
*/
void recuperaDoArquivo(int &contador_geral)
{
    File arquivo;
    arquivo = SD.open("medicoes.txt", FILE_READ);
    if (arquivo)
    {
        int iRecupera = 0;
        int valor;
        while (arquivo.available())
        {
            valor = arquivo.read();
            iRecupera++;
        }
        arquivo.close();
        if (iRecupera == 0)
        {
            contador_geral = 0;
            Serial.println("Arquivo não possuí registros!");
        }
        else
        {
            contador_geral = iRecupera / 4;
        }
    }
    else
    {
        Serial.println("Erro ao abrir medicoes.txt");
    }
}

/*  
    Função escritaArquivoTXTSD(dados recebe)
    Responsável por escrever os dados de intensidade coletados pelo sensor MQ-2 no arquivo "medicoes.txt"
    Recebe como parâmetro (dados recebe)
    @param{dados recebe} registro definido para valores de intensidade e exclusao
*/
void escritaArquivoTXTSD(dados recebe)
{
    File arquivo;
    arquivo = SD.open("medicoes.txt", FILE_WRITE);
    if (arquivo)
    {
        arquivo.println(recebe.intensidade);
        arquivo.close();
    }
    else
    {
        Serial.println("Erro ao abrir medicoes.txt");
    }
}

/*  
    Função leDoArquivo(dados *vetor)
    Responsável por ler os valores de intensidade presentes no arquivo "medicoes.txt" e enviar para o vetor de registros
    Recebe como parâmetro (dados *vetor)
    @param{dados vetor[]} vetor alocado dinâmicamente que recebe os valores de intensidade
*/
void leDoArquivo(dados *vetor)
{
    File arquivo;
    int primeiroNumero = 0;
    int numero = 0;
    arquivo = SD.open("medicoes.txt", FILE_READ);
    if (arquivo)
    {
        int iMandaVetor = 0;
        while (arquivo.available())
        {
            char lido = arquivo.read();
            if (isdigit(lido))
            {
                numero *= 10;
                numero += (lido - '0');
            }
            else
            {
                primeiroNumero = numero;
                if (numero > 0)
                {
                    vetor[iMandaVetor].intensidade = numero;
                    iMandaVetor++;
                }
                numero = 0;
            }
        }
        arquivo.close();
    }
    else
    {
        Serial.println("Erro ao abrir medicoes.txt");
    }
}

/*  
    Função escreveNoArquivo(dados *vetor, int contador_geral)
    Responsável por escrever os valores de intensidade ordenados no arquivo "medicoes.txt"
    Recebe como parâmetro (dados *vetor, int contador_geral)
    @param{dados vetor[]} vetor alocado dinâmicamente que recebe os valores de intensidade
    @param{int contador_geral} tamanho do vetor alocado dinâmicamente e preenchido com valores de intensidade pela função void leDoArquivo
*/
void escreveNoArquivo(dados *vetor, int contador_geral)
{
    File arquivo;
    arquivo = SD.open("medicoes.txt", O_TRUNC | O_WRITE);
    if (arquivo)
    {
        int iEscreveArquivo = 0;
        while (iEscreveArquivo < contador_geral)
        {
            if (vetor[iEscreveArquivo].exclusao != true)
            {
                arquivo.println(vetor[iEscreveArquivo].intensidade);
            }
            iEscreveArquivo++;
        }
        arquivo.close();
    }
    else
    {
        Serial.println("Erro ao abrir medicoes.txt");
    }
}

/*  
    Função particiona(dados vetor[], int inicio, int fim, char tecla)
    Responsável por particionar o vetor e retornar a posicão do pivo a função void quicksort
    Recebe como parâmetro (dados vetor[], int inicio, int fim, char tecla)
    @param{dados vetor[]} vetor alocado dinâmicamente e preenchido com valores de intensidade pela função void leDoArquivo
    @param{int inicio} posição inicial do vetor
    @param{int fim} posição final do vetor
    @param{char tecla} char passado pelo Keypad que define a ordenação em crescente ou decrescente
*/
int particiona(dados vetor[], int inicio, int fim, char tecla)
{   
    // (tecla == '1') define uma ordenação crescente
    if (tecla == '1')
    {
        dados pivo = vetor[inicio];
        int iCrescente = inicio + 1;
        int j = fim;
        dados aux;

        while (iCrescente <= j)
        {
            if (vetor[iCrescente].intensidade <= pivo.intensidade)
                iCrescente++;
            else if (pivo.intensidade < vetor[j].intensidade)
                j--;
            else
            {
                aux = vetor[iCrescente];
                vetor[iCrescente] = vetor[j];
                vetor[j] = aux;
                iCrescente++;
                j--;
            }
        }

        vetor[inicio] = vetor[j];
        vetor[j] = pivo;

        return j;
    }
    // (tecla == '2') define uma ordenação decrescente
    else if (tecla == '2')
    {

        dados pivo = vetor[inicio];
        int iDecrescente = inicio + 1;
        int j = fim;
        dados aux;

        while (iDecrescente <= j)
        {
            if (vetor[iDecrescente].intensidade >= pivo.intensidade)
                iDecrescente++;
            else if (pivo.intensidade > vetor[j].intensidade)
                j--;
            else
            {
                aux = vetor[iDecrescente];
                vetor[iDecrescente] = vetor[j];
                vetor[j] = aux;
                iDecrescente++;
                j--;
            }
        }

        vetor[inicio] = vetor[j];
        vetor[j] = pivo;

        return j;
    }
}

/*  
    Função quicksort(dados vetor[], int pos_pivo, int fim, char tecla)
    Responsável por ordenar os registros presentes no arquivo "medicoes.txt"
    Recebe como parâmetro (dados vetor[], int pos_pivo, int fim, char tecla)
    @param{dados vetor[]} vetor alocado dinâmicamente e preenchido com valores de intensidade pela função void leDoArquivo
    @param{int pos_pivo} posição do pivo retornada pela função int particiona
    @param{int fim} posição final do vetor
    @param{char tecla} char passado pelo Keypad que define a ordenação em crescente ou decrescente
*/
void quicksort(dados vetor[], int pos_pivo, int fim, char tecla)
{
    int pos_novo_pivo;
    if (pos_pivo < fim)
    {
        pos_novo_pivo = particiona(vetor, pos_pivo, fim, tecla);
        quicksort(vetor, pos_pivo, pos_novo_pivo - 1, tecla);
        quicksort(vetor, pos_novo_pivo + 1, fim, tecla);
    }
}

/*  
    Função imprimeNoSerial (char tecla)
    Responsável por exibir a leitura do arquivo "medicoes.txt" caso seja solicitado
    Recebe como parâmetro (char tecla)
    @param{tecla} char passado pelo Keypad que define a opção de impressão
*/
void imprimeNoSerial(char tecla)
{
    File arquivo;
    int primeiroNumero = 0;
    int numero = 0;
    arquivo = SD.open("medicoes.txt", FILE_READ);
    if (arquivo)
    {
        while (arquivo.available())
        {
            char lido = arquivo.read();
            if (isdigit(lido))
            {
                numero *= 10;
                numero += (lido - '0');
            }
            else
            {
                primeiroNumero = numero;
                if (numero > 0)
                {
                    Serial.print(numero);
                    Serial.print(" ");
                }
                numero = 0;
            }
        }
        arquivo.close();
    }
    else
    {
        Serial.println("Erro ao abrir medições.txt");
    }
}
